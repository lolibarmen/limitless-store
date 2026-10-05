#include "SpatialStreaming.hpp"
#include <ChunkOctree/ChunkOctree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/core/math.hpp>
#include <vector>

using namespace godot;

// Расстояние Чебышёва (максимальная разница по осям)
static float cube_distance(const Vector3& a, const Vector3& b) {
    return Math::max(Math::max(Math::abs(a.x - b.x), Math::abs(a.y - b.y)), Math::abs(a.z - b.z));
}

SpatialStreaming* SpatialStreaming::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("SpatialStreaming");
    return Object::cast_to<SpatialStreaming>(obj);
}

void SpatialStreaming::_bind_methods() {
    // ClassDB::bind_method(D_METHOD("set_player_pos", "pos"), &SpatialStreaming::set_player_pos);
}

void SpatialStreaming::_ready() {
    // Инициализация
}

void SpatialStreaming::_process(double delta) {
    auto* vp = get_viewport();
    if (!vp) return;
    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    player_pos = cam->get_global_position();

    update_root_zones();

    auto* octree = ChunkOctree::get_singleton();
    if (octree) {
        octree->for_each_root([this](Chunk* root) {
            evaluate_and_update_tree(root);
        });
    }

    _notify_changes();
}

Vector3i SpatialStreaming::get_cell_from_pos(const Vector3& pos) const {
    return Vector3i(
        static_cast<int32_t>(Math::floor(pos.x / ROOT_SIZE)),
        static_cast<int32_t>(Math::floor(pos.y / ROOT_SIZE)),
        static_cast<int32_t>(Math::floor(pos.z / ROOT_SIZE))
    );
}

void SpatialStreaming::update_root_zones() {
    Vector3i pc = get_cell_from_pos(player_pos);
    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;

    // 1. Находим и удаляем корни, вышедшие за пределы радиуса
    std::vector<Vector3i> cells_to_remove;
    for (const auto& pair : _active_roots) {
        if (Math::abs(pair.first.x - pc.x) > root_radius ||
            Math::abs(pair.first.y - pc.y) > root_radius ||
            Math::abs(pair.first.z - pc.z) > root_radius) {
            cells_to_remove.push_back(pair.first);
        }
    }

    for (const Vector3i& cell : cells_to_remove) {
        uint64_t root_id = _active_roots[cell];
        octree->remove_root(root_id);
        _active_roots.erase(cell);
    }

    // 2. Добавляем новые корни в радиусе
    for (int dx = -root_radius; dx <= root_radius; dx++) {
        for (int dy = -root_radius; dy <= root_radius; dy++) {
            for (int dz = -root_radius; dz <= root_radius; dz++) {
                Vector3i cell = pc + Vector3i(dx, dy, dz);
                
                // Проверяем по координатам ячейки, а не по ID, так как ID генерирует сам ChunkOctree
                if (_active_roots.find(cell) == _active_roots.end()) {
                    Vector3 center(
                        (static_cast<float>(cell.x) + 0.5f) * ROOT_SIZE,
                        (static_cast<float>(cell.y) + 0.5f) * ROOT_SIZE,
                        (static_cast<float>(cell.z) + 0.5f) * ROOT_SIZE
                    );
                    uint64_t new_id = octree->add_root(center, ROOT_SIZE);
                    _active_roots[cell] = new_id;
                }
            }
        }
    }
}

void SpatialStreaming::evaluate_and_update_tree(Chunk* node) {
    if (!node) return;

    // Использование расстояния Чебышёва и гистерезиса
    float dist = cube_distance(node->center, player_pos);
    bool should_split = dist < node->size * 4.0f;
    bool should_collapse = dist > node->size * 4.1f;

    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;

    if (node->is_leaf() && should_split && node->depth < MAX_DEPTH) {
        octree->split(node);
        // Рекурсивно оцениваем только что созданных детей
        for (auto& child_ptr : node->children) {
            if (child_ptr) {
                evaluate_and_update_tree(child_ptr.get());
            }
        }
    }
    else if (!node->is_leaf() && should_collapse) {
        octree->collapse(node);
    }
    else if (!node->is_leaf()) {
        // Рекурсивно оцениваем существующих детей
        for (auto& child_ptr : node->children) {
            if (child_ptr) {
                evaluate_and_update_tree(child_ptr.get());
            }
        }
    }
}

void SpatialStreaming::_notify_changes() {
    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;

    std::unordered_set<uint64_t> current_zones;
    
    octree->for_each_leaf([&](Chunk* chunk) {
        current_zones.insert(chunk->id);
    });

    if (current_zones == _prev_active_zones) {
        return;
    }

    std::vector<uint64_t> updates;

    for (uint64_t id : current_zones) {
        if (_prev_active_zones.find(id) == _prev_active_zones.end()) {
            updates.push_back(id);
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