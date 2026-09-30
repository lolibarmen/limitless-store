#include "SpatialStreaming.hpp"
#include <ChunkOctree/ChunkOctree.hpp>
#include <ChunkOctree/Chunk.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Вспомогательная функция вне класса, так как это чистая математика
static float cube_distance(const Vector3& a, const Vector3& b) {
    return Math::max(Math::max(Math::abs(a.x - b.x), Math::abs(a.y - b.y)), Math::abs(a.z - b.z));
}

void SpatialStreaming::_bind_methods() {}

SpatialStreaming* SpatialStreaming::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("SpatialStreaming");
    return Object::cast_to<SpatialStreaming>(obj);
}

void SpatialStreaming::_ready() {}

void SpatialStreaming::_process(double delta) {
    auto* vp = get_viewport();
    if (!vp) return;
    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    player_pos = cam->get_global_position();

    update_root_zones();

    ChunkOctree::get_singleton()->for_each_root([this](Chunk* root) {
        evaluate_and_update_tree(root);
    });

    // 3. Уведомление SemanticWorld об изменениях в листьях
    _notify_changes();
}

void SpatialStreaming::update_root_zones() {
    Vector3i pc = Vector3i(
        (int)Math::floor(player_pos.x / ROOT_SIZE),
        (int)Math::floor(player_pos.y / ROOT_SIZE),
        (int)Math::floor(player_pos.z / ROOT_SIZE)
    );

    std::vector<Vector3i> roots_to_remove;

    for (const auto& key : roots_to_remove) {
        ChunkOctree::get_singleton()->remove_root(key);
    }

    // Добавляем новые корни в радиусе
    for (int dx = -root_radius; dx <= root_radius; dx++) {
        for (int dy = -root_radius; dy <= root_radius; dy++) {
            for (int dz = -root_radius; dz <= root_radius; dz++) {
                Vector3i cell = pc + Vector3i(dx, dy, dz);
                // Нужен способ проверить, существует ли уже корень. Добавим has_root(key) в ChunkOctree
                if (!ChunkOctree::get_singleton()->has_root(cell)) {
                    Vector3 center = (Vector3(cell) + Vector3(0.5f, 0.5f, 0.5f)) * ROOT_SIZE;
                    ChunkOctree::get_singleton()->add_root(cell, center, ROOT_SIZE);
                }
            }
        }
    }
}

void SpatialStreaming::evaluate_and_update_tree(Chunk* node) {
    if (!node) return;

    float dist = cube_distance(node->center, player_pos);
    bool should_split = dist < node->size * 4.0f;
    bool should_collapse = dist > node->size * 4.1f;

    if (node->is_leaf() && should_split && node->depth < MAX_DEPTH) {
        ChunkOctree::get_singleton()->split_node(node);
    }
    else if (!node->is_leaf() && should_collapse) {
        ChunkOctree::get_singleton()->collapse_node(node);
    }
    else if (!node->is_leaf()) {
        // Рекурсивный обход детей
        for (auto& child_ptr : node->children) {
            if (child_ptr) {
                evaluate_and_update_tree(child_ptr.get());
            }
        }
    }
}

void SpatialStreaming::_notify_changes() {
    const auto& current_leaves = ChunkOctree::get_singleton()->get_leaf_chunks();
    std::unordered_map<Vector3i, AABB, Vector3iHash> current_zones;
    current_zones.reserve(current_leaves.size());
    
    for (const auto& [key, chunk] : current_leaves) {
        current_zones[key] = chunk->get_aabb();
    }

    // Оптимизация: если зоны идентичны, выходим
    if (current_zones.size() == _prev_active_zones.size()) {
        bool identical = true;
        for (const auto& [key, _] : current_zones) {
            if (_prev_active_zones.find(key) == _prev_active_zones.end()) {
                identical = false;
                break;
            }
        }
        if (identical) return;
    }

    std::vector<ZoneLODUpdate> updates;

    for (const auto& [key, aabb] : current_zones) {
        if (_prev_active_zones.find(key) == _prev_active_zones.end()) {
            int lod = MAX_DEPTH - current_leaves.at(key)->depth; 
            updates.push_back({aabb, lod}); 
        }
    }

    for (const auto& [key, aabb] : _prev_active_zones) {
        if (current_zones.find(key) == current_zones.end()) {
            updates.push_back({aabb, LOD_UNLOADED}); 
        }
    }

    if (!updates.empty()) {
        SemanticWorld* world = SemanticWorld::get_singleton();
        if (world) {
            world->on_zone_changed(updates);
        }
    }

    _prev_active_zones = std::move(current_zones);
}