#include "SemanticWorld.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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
    
    // Уведомляем пространственный менеджер
    auto* spatial_mgr = ShapeSpatialManager::get_singleton();
    if (spatial_mgr) {
        spatial_mgr->register_shape(id, shape);
    }
    
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

void SemanticWorld::propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape) {
    if (!proposed_shape.is_valid()) {
        ERR_PRINT("propose_shape_change: proposed_shape is invalid.");
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

    // 3. Фаза финализации (Commit Phase) - ИСПРАВЛЕННАЯ ВЕРСИЯ
    std::unordered_map<uint64_t, uint64_t> id_mapping; // old_id -> new_id

    for (auto& pair : accepted_changes) {
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
            // Вызываем наш новый метод замены ID внутри фигуры
            pair.second->replace_owned_id(mapping.first, mapping.second);
        }
    }
    _shapes_mutex->unlock();

    // 4. Уведомление внешних систем
    // notify_mesh_generator_of_changes(id_mapping);
}