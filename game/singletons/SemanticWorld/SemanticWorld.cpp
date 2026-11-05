#include "SemanticWorld.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <unordered_set>

using namespace godot;

SemanticWorld::SemanticWorld() {
    _shapes_mutex.instantiate();
}

void SemanticWorld::_bind_methods() {
    // Binding'и для Godot, если нужны
}

SemanticWorld* SemanticWorld::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("SemanticWorld");
    return Object::cast_to<SemanticWorld>(obj);
}

uint64_t SemanticWorld::register_shape(Ref<SemanticShape> shape) {
    if (!shape.is_valid()) return 0;
    uint64_t id = _next_id++;
    shape->set_id(id);
    _shapes_mutex->lock();
    _shapes[id] = shape;
    _shapes_mutex->unlock();

    shape->on_register();

    return id;
}

void SemanticWorld::unregister_shape(uint64_t id) {
    _shapes_mutex->lock();
    _shapes.erase(id);
    _shapes_mutex->unlock();
    
    auto* spatial_mgr = ShapeSpatialManager::get_singleton();
    if (spatial_mgr) {
        spatial_mgr->unregister_shape(id);
    }
}

Ref<SemanticShape> SemanticWorld::get_shape(uint64_t id) const {
    _shapes_mutex->lock();
    auto it = _shapes.find(id);
    Ref<SemanticShape> result = (it != _shapes.end()) ? it->second : nullptr;
    _shapes_mutex->unlock();
    return result;
}

void SemanticWorld::on_zone_changed(uint64_t shape_id, const std::vector<uint64_t>& chunk_ids) {
    if (chunk_ids.empty()) return;

    _shapes_mutex->lock();
    auto it = _shapes.find(shape_id);
    Ref<SemanticShape> shape = (it != _shapes.end()) ? it->second : nullptr;
    _shapes_mutex->unlock();

    if (shape.is_valid()) {
        for (uint64_t chunk_id : chunk_ids) {
            shape->on_zone_changed(chunk_id);
        }
    }
}

void SemanticWorld::_commit_changes(const std::unordered_map<uint64_t, Ref<SemanticShape>>& accepted_changes) {
    std::unordered_map<uint64_t, uint64_t> id_mapping; // old_id -> new_id

    for (const auto& pair : accepted_changes) {
        uint64_t old = pair.first;
        Ref<SemanticShape> new_shape = pair.second;

        // А. Удаляем старую фигуру через официальный метод
        // (Внутри он сам захватит и отпустит мьютекс)
        unregister_shape(old);

        // Б. Регистрируем новую фигуру через официальный метод
        // (Внутри он сам сгенерирует ID, вызовет set_id, захватит мьютекс и добавит в _shapes)
        uint64_t new_id = register_shape(new_shape);
        id_mapping[old] = new_id;
    }

    // В. Обновляем граф зависимостей (_owned_shape_ids) для ВСЕХ фигур
    // Поскольку register_shape/unregister_shape уже отработали, нам нужно
    // снова безопасно захватить мьютекс для пакетного обновления ссылок.
    _shapes_mutex->lock();
    for (auto& pair : _shapes) {
        for (const auto& mapping : id_mapping) {
            pair.second->replace_owned_id(mapping.first, mapping.second);
        }
    }
    _shapes_mutex->unlock();

    // 4. Уведомление внешних систем
    // notify_mesh_generator_of_changes(id_mapping);
}

void SemanticWorld::_propose_parameter_change(uint64_t old_id, Ref<SemanticShape> proposed_shape) {
    if (_parameter_change_in_progress) {
        return;
    }

    _parameter_change_in_progress = true;

    struct ChangeNotice {
        uint64_t trigger_id;
        Ref<SemanticShape> trigger_shape;
        uint64_t target_id;
    };

    std::vector<ChangeNotice> evaluation_queue;
    std::unordered_map<uint64_t, Ref<SemanticShape>> accepted_changes;
    std::unordered_set<uint64_t> visited;
    std::unordered_set<uint64_t> scheduled;

    // 1. Инициализация: исходная фигура уже помечена изменённой в notify_shape_changed().
    evaluation_queue.push_back({old_id, proposed_shape, old_id});
    scheduled.insert(old_id);

    // 2. Фаза уведомления и оценки реакций.
    int max_iterations = 100;
    int iteration = 0;

    while (!evaluation_queue.empty() && iteration < max_iterations) {
        iteration++;
        ChangeNotice current = evaluation_queue.back();
        evaluation_queue.pop_back();

        if (!visited.insert(current.target_id).second) {
            continue;
        }

        Ref<SemanticShape> target = get_shape(current.target_id);
        if (!target.is_valid()) {
            continue;
        }

        // Граф владения считается двусторонним: фигура зависит и от тех, кем она
        // владеет, и от тех, кто владеет ею.
        _shapes_mutex->lock();
        std::vector<uint64_t> connected;
        for (const auto& pair : _shapes) {
            if (pair.first == current.target_id) {
                continue;
            }

            if (pair.second->has_owned_shape(current.target_id) ||
                target->has_owned_shape(pair.first)) {
                connected.push_back(pair.first);
            }
        }
        _shapes_mutex->unlock();

        for (uint64_t connected_id : connected) {
            if (visited.find(connected_id) != visited.end() ||
                scheduled.find(connected_id) != scheduled.end()) {
                continue;
            }
            scheduled.insert(connected_id);

            Ref<SemanticShape> connected_shape = get_shape(connected_id);
            if (!connected_shape.is_valid()) {
                continue;
            }

            // Уведомляем зависимую фигуру. Вложенный propose_shape_change()
            // будет подавлен guard-ом, но AABB и кэши фигуры обновятся.
            connected_shape->notify_shape_changed();

            Ref<SemanticShape> reaction = connected_shape->evaluate_reaction(
                current.trigger_id,
                current.trigger_shape
            );

            const bool is_replacement =
                reaction.is_valid() &&
                reaction.ptr() != connected_shape.ptr() &&
                reaction->get_shape_type() != StringName("shape");

            if (is_replacement && accepted_changes.find(connected_id) == accepted_changes.end()) {
                accepted_changes[connected_id] = reaction;
                evaluation_queue.push_back({connected_id, reaction, connected_id});
            } else {
                // Реакции нет, но обход должен продолжиться, чтобы уведомить
                // всю связную компоненту графа владения.
                evaluation_queue.push_back({current.trigger_id, current.trigger_shape, connected_id});
            }
        }
    }

    if (iteration >= max_iterations) {
        print_error("propose_shape_change: Max iterations reached. Possible infinite loop in shape reactions.");
    }

    // 3. Финализация. Исходная фигура не попадает в accepted_changes и сохраняет свой ID.
    _commit_changes(accepted_changes);

    _parameter_change_in_progress = false;
}

void SemanticWorld::propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape) {
    if (!proposed_shape.is_valid()) {
        ERR_PRINT("propose_shape_change: proposed_shape is invalid.");
        return;
    }

    Ref<SemanticShape> registered_shape = get_shape(old_id);
    if (registered_shape.is_valid() && registered_shape.ptr() == proposed_shape.ptr()) {
        _propose_parameter_change(old_id, proposed_shape);
        return;
    }

    struct ChangeProposal {
        uint64_t target_old_id;
        Ref<SemanticShape> new_shape;
    };

    std::vector<ChangeProposal> evaluation_queue;
    std::unordered_map<uint64_t, Ref<SemanticShape>> accepted_changes;

    // 1. Инициализация
    evaluation_queue.push_back({old_id, proposed_shape});
    accepted_changes[old_id] = proposed_shape;

    // 2. Фаза оценки (Evaluation Phase)
    int max_iterations = 100;
    int iteration = 0;

    while (!evaluation_queue.empty() && iteration < max_iterations) {
        iteration++;
        ChangeProposal current = evaluation_queue.back();
        evaluation_queue.pop_back();

        // Находим всех "владельцев" изменяемой фигуры
        _shapes_mutex->lock();
        std::vector<uint64_t> owners;
        for (const auto& pair : _shapes) {
            if (pair.second->has_owned_shape(current.target_old_id)) {
                owners.push_back(pair.first);
            }
        }
        _shapes_mutex->unlock();

        // Спрашиваем каждого владельца, как он хочет отреагировать
        for (uint64_t owner_id : owners) {
            Ref<SemanticShape> owner = get_shape(owner_id);
            if (!owner.is_valid()) continue;

            Ref<SemanticShape> owner_reaction = owner->evaluate_reaction(current.target_old_id, current.new_shape);

            if (owner_reaction.is_valid() && owner_reaction->get_shape_type() != StringName("shape")) {
                if (accepted_changes.find(owner_id) == accepted_changes.end()) {
                    accepted_changes[owner_id] = owner_reaction;
                    evaluation_queue.push_back({owner_id, owner_reaction});
                }
            }
        }
    }

    if (iteration >= max_iterations) {
        print_error("propose_shape_change: Max iterations reached. Possible infinite loop in shape reactions.");
    }

    // 3. Фаза финализации (Commit Phase)
    _commit_changes(accepted_changes);
}

void SemanticWorld::propose_shape_change(uint64_t old_id) {
    Ref<SemanticShape> shape = get_shape(old_id);
    if (shape.is_valid()) {
        propose_shape_change(old_id, shape);
    }
}