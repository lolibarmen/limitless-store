#include "ShapeSpatialManager.hpp"
#include <godot_cpp/classes/engine.hpp>

namespace godot {

ShapeSpatialManager* ShapeSpatialManager::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("ShapeSpatialManager");
    return Object::cast_to<ShapeSpatialManager>(obj);
}

void ShapeSpatialManager::register_shape(uint64_t shape_id, Ref<SemanticShape> shape) {
    if (shape->is_relative()) {
        // Фигура независима. Создаем ей личное дерево.
        auto octree = std::make_unique<ChunkOctree>();
        ChunkOctree* raw_ptr = octree.get();
        _owned_octrees[shape_id] = std::move(octree);
        _shape_to_octree[shape_id] = raw_ptr;
        
        // Инициализируем корень дерева размером из AABB фигуры
        raw_ptr->add_root(shape->get_aabb().get_center(), shape->get_aabb().get_size().x);
    } else {
        // Фигура статична. Дерева у неё пока нет, ждем bind_spatially или она будет использовать глобальную сетку.
        _shape_to_octree[shape_id] = nullptr; 
    }
}

void ShapeSpatialManager::bind_spatially(uint64_t child_shape_id, uint64_t parent_shape_id) {
    auto it = _shape_to_octree.find(parent_shape_id);
    if (it != _shape_to_octree.end() && it->second != nullptr) {
        _shape_to_octree[child_shape_id] = it->second; // Прокси-ссылка на дерево родителя
    }
}

void ShapeSpatialManager::unregister_shape(uint64_t shape_id) {
    auto owned_it = _owned_octrees.find(shape_id);
    if (owned_it != _owned_octrees.end()) {
        ChunkOctree* dying_tree = owned_it->second.get();
        // Обнуляем всех, кто использовал это дерево, чтобы избежать висячих ссылок
        for (auto& pair : _shape_to_octree) {
            if (pair.second == dying_tree) {
                pair.second = nullptr;
            }
        }
        _owned_octrees.erase(owned_it);
    }
    _shape_to_octree.erase(shape_id);
}

ChunkOctree* ShapeSpatialManager::get_octree(uint64_t shape_id) const {
    auto it = _shape_to_octree.find(shape_id);
    return (it != _shape_to_octree.end()) ? it->second : nullptr;
}

void ShapeSpatialManager::for_each_owned_octree(std::function<void(uint64_t, ChunkOctree*)> func) {
    for (auto& pair : _owned_octrees) {
        if (pair.second) {
            func(pair.first, pair.second.get());
        }
    }
}

} // namespace godot