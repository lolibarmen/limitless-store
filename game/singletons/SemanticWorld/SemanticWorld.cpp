#include "SemanticWorld.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <unordered_set>
#include <utility>
#include <vector>

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
    if (id == 0) return;

    Ref<SemanticShape> shape;
    _shapes_mutex->lock();
    auto it = _shapes.find(id);
    if (it != _shapes.end()) {
        shape = it->second;
        _shapes.erase(it);
    }
    _shapes_mutex->unlock();

    // Сначала убираем представление, пока октодерево ещё существует.
    if (WorldMesh* wm = WorldMesh::get_singleton()) {
        wm->drop_shape(id);
    }

    auto* spatial_mgr = ShapeSpatialManager::get_singleton();
    if (spatial_mgr) {
        spatial_mgr->unregister_shape(id);
    }

    if (shape.is_valid()) {
        shape->set_id(0);
        shape->on_unregister();
    }
}

void SemanticWorld::unregister_shape_with_owned(uint64_t id) {
    if (id == 0) return;

    std::vector<std::pair<uint64_t, bool>> stack;
    std::unordered_set<uint64_t> visited;
    stack.push_back({id, false});

    while (!stack.empty()) {
        const uint64_t current_id = stack.back().first;
        const bool expanded = stack.back().second;
        stack.pop_back();

        if (current_id == 0) continue;

        if (expanded) {
            unregister_shape(current_id);
            continue;
        }

        if (!visited.insert(current_id).second) {
            continue;
        }

        stack.push_back({current_id, true});

        Ref<SemanticShape> shape = get_shape(current_id);
        if (shape.is_valid()) {
            const PackedInt64Array owned = shape->get_owned_shapes();
            for (int i = owned.size() - 1; i >= 0; --i) {
                stack.push_back({(uint64_t)owned[i], false});
            }
        }
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

    Ref<SemanticShape> shape = get_shape(shape_id);
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
        if (WorldMesh* wm = WorldMesh::get_singleton()) {
            wm->invalidate_shape(new_id);
        }
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

std::vector<uint64_t> SemanticWorld::_find_related_shapes(uint64_t shape_id, bool two_way) const {
    std::vector<uint64_t> related;

    _shapes_mutex->lock();
    auto target_it = _shapes.find(shape_id);
    const Ref<SemanticShape>* target = (target_it != _shapes.end()) ? &target_it->second : nullptr;
    for (const auto& [id, shape] : _shapes) {
        if (id != shape_id &&
            (shape->has_owned_shape(shape_id) ||
             (two_way && target && (*target)->has_owned_shape(id)))) {
            related.push_back(id);
        }
    }
    _shapes_mutex->unlock();

    return related;
}

void SemanticWorld::_run_change_cascade(uint64_t old_id, Ref<SemanticShape> proposed_shape, bool parameter_change) {
    if (parameter_change) {
        if (_parameter_change_in_progress) {
            return;
        }
        _parameter_change_in_progress = true;
    }

    struct ChangeNotice {
        uint64_t trigger_id;
        Ref<SemanticShape> trigger_shape;
        uint64_t target_id;
    };

    std::vector<ChangeNotice> evaluation_queue;
    std::unordered_map<uint64_t, Ref<SemanticShape>> accepted_changes;
    std::unordered_set<uint64_t> visited{old_id};

    evaluation_queue.push_back({old_id, proposed_shape, old_id});

    if (!parameter_change) {
        accepted_changes[old_id] = proposed_shape;
    }

    constexpr int kMaxIterations = 100;
    int iteration = 0;

    while (!evaluation_queue.empty() && iteration < kMaxIterations) {
        iteration++;
        ChangeNotice current = evaluation_queue.back();
        evaluation_queue.pop_back();

        // В режиме изменения параметров граф владения обходится в обе стороны,
        // в режиме замены — только по владельцам изменяемой фигуры.
        const std::vector<uint64_t> related = _find_related_shapes(current.target_id, parameter_change);

        for (uint64_t related_id : related) {
            if (visited.find(related_id) != visited.end()) {
                continue;
            }

            // В режиме параметров фигура посещается один раз. В режиме замены
            // фиксируем её только после принятой реакции, чтобы её можно было
            // перепроверить при изменении другого владельца.
            if (parameter_change) {
                visited.insert(related_id);
            }

            Ref<SemanticShape> related_shape = get_shape(related_id);
            if (!related_shape.is_valid()) {
                continue;
            }

            if (parameter_change) {
                // Вложенный propose_shape_change() подавляется guard-ом,
                // но AABB и кэши фигуры обновляются.
                related_shape->notify_shape_changed();
            }

            Ref<SemanticShape> reaction = related_shape->evaluate_reaction(
                current.trigger_id,
                current.trigger_shape
            );

            const bool is_replacement =
                reaction.is_valid() &&
                reaction->get_shape_type() != StringName("shape") &&
                (!parameter_change || reaction.ptr() != related_shape.ptr());

            if (is_replacement && accepted_changes.find(related_id) == accepted_changes.end()) {
                accepted_changes[related_id] = reaction;
                visited.insert(related_id);
                evaluation_queue.push_back({related_id, reaction, related_id});
            } else if (parameter_change) {
                // Реакции нет, но обход продолжается, чтобы уведомить всю компоненту.
                evaluation_queue.push_back({current.trigger_id, current.trigger_shape, related_id});
            }
        }
    }

    if (iteration >= kMaxIterations) {
        print_error("propose_shape_change: Max iterations reached. Possible infinite loop in shape reactions.");
    }

    _commit_changes(accepted_changes);

    if (parameter_change) {
        _parameter_change_in_progress = false;
    }
}

void SemanticWorld::propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape) {
    if (!proposed_shape.is_valid()) {
        ERR_PRINT("propose_shape_change: proposed_shape is invalid.");
        return;
    }

    Ref<SemanticShape> registered_shape = get_shape(old_id);
    const bool parameter_change =
        registered_shape.is_valid() && registered_shape.ptr() == proposed_shape.ptr();

    _run_change_cascade(old_id, proposed_shape, parameter_change);

    if (parameter_change) {
        if (WorldMesh* wm = WorldMesh::get_singleton()) {
            wm->invalidate_shape(old_id);
        }
    }
}

void SemanticWorld::propose_shape_change(uint64_t old_id) {
    Ref<SemanticShape> shape = get_shape(old_id);
    if (shape.is_valid()) {
        propose_shape_change(old_id, shape);
    }
}