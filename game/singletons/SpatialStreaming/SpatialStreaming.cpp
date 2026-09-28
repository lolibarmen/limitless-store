#include "SpatialStreaming.hpp"
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

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

    // 1. Обновляем структуру дерева (сплит/мёрдж)
    update_roots();
    for (auto& [cell, root_ptr] : roots) {
        update_recurs(root_ptr.get());
    }

    // 2. Считаем дельту и уведомляем SemanticWorld об изменениях
    _notify_changes();
}

void SpatialStreaming::spawn_chunk(std::unique_ptr<Chunk> chunk_ptr, const Vector3i& key) {
    Chunk* raw = chunk_ptr.get();
    roots[key] = std::move(chunk_ptr);
    if (raw->is_leaf()) {
        leaf_chunks[key] = raw;
    }
}

void SpatialStreaming::delete_children(Chunk* n) {
    if (!n || n->is_leaf()) return;
    
    for (auto& child_ptr : n->children) {
        if (child_ptr) {
            Chunk* child = child_ptr.get();
            
            // Если ребенок был листом, убираем его из быстрого доступа
            if (child->is_leaf()) {
                Vector3i child_key((int)child->center.x, (int)child->center.y, (int)child->center.z);
                leaf_chunks.erase(child_key);
            }
            
            // Рекурсивно удаляем внуков
            delete_children(child);
            
            // unique_ptr автоматически уничтожит объект и вызовет деструктор Chunk,
            // который сам очистит свои mesh_instances через clear_meshes()
            child_ptr.reset(); 
        }
    }
}

float cube_distance(Vector3 a, Vector3 b) {
    return Math::max(Math::max(Math::abs(a.x - b.x), Math::abs(a.y - b.y)), Math::abs(a.z - b.z));
}

void SpatialStreaming::update_recurs(Chunk* n) {
    if (!n) return;

    float dist = cube_distance(n->center, player_pos);
    bool should_split    = dist < n->size * 4.0f;
    bool should_collapse = dist > n->size * 4.1f;

    if (n->is_leaf() && should_split && n->depth < MAX_DEPTH) {
        // Чанк перестает быть листовым, убираем из leaf_chunks
        Vector3i parent_key((int)n->center.x, (int)n->center.y, (int)n->center.z);
        leaf_chunks.erase(parent_key);
        n->clear_meshes();

        float h = n->size / 2.0f, q = h / 2.0f;
        int i = 0;
        for (int x : {-1, 1}) {
            for (int y : {-1, 1}) {
                for (int z : {-1, 1}) {
                    auto child = std::make_unique<Chunk>(n->center + Vector3(x, y, z) * q, h, n->depth + 1, n);
                    Chunk* new_child = child.get();
                    
                    n->children[i] = std::move(child);
                    
                    Vector3i child_key((int)new_child->center.x, (int)new_child->center.y, (int)new_child->center.z);
                    leaf_chunks[child_key] = new_child;
                    i++;
                }
            }
        }
    }
    else if (!n->is_leaf() && should_collapse) {
        // Удаляем детей (они сами уберутся из leaf_chunks внутри delete_children)
        delete_children(n);
        
        // Родитель снова становится листовым
        Vector3i key((int)n->center.x, (int)n->center.y, (int)n->center.z);
        leaf_chunks[key] = n;
    }
    else if (!n->is_leaf()) {
        for (auto& child_ptr : n->children) {
            if (child_ptr) {
                update_recurs(child_ptr.get());
            }
        }
    }
}

void SpatialStreaming::update_roots() {
    Vector3i pc = Vector3i(
        (int)Math::floor(player_pos.x / ROOT_SIZE),
        (int)Math::floor(player_pos.y / ROOT_SIZE),
        (int)Math::floor(player_pos.z / ROOT_SIZE)
    );

    // 1. Удаляем дальние корни (БЕЗОПАСНАЯ ИТЕРАЦИЯ)
    for (auto it = roots.begin(); it != roots.end(); ) {
        Vector3i d = it->first - pc;
        if (abs(d.x) > root_radius || abs(d.y) > root_radius || abs(d.z) > root_radius) {
            delete_children(it->second.get());
            
            leaf_chunks.erase(it->first);
            it = roots.erase(it); 
        } else {
            ++it;
        }
    }

    // 2. Добавляем ближние корни
    for (int dx = -root_radius; dx <= root_radius; dx++) {
        for (int dy = -root_radius; dy <= root_radius; dy++) {
            for (int dz = -root_radius; dz <= root_radius; dz++) {
                Vector3i cell = pc + Vector3i(dx, dy, dz);
                if (!roots.count(cell)) {
                    Vector3 center = (Vector3(cell) + Vector3(0.5f, 0.5f, 0.5f)) * ROOT_SIZE;
                    auto root = std::make_unique<Chunk>(center, ROOT_SIZE, 0, nullptr);
                    spawn_chunk(std::move(root), cell);
                }
            }
        }
    }
}

void SpatialStreaming::_notify_changes() {
    std::unordered_map<Vector3i, AABB, Vector3iHash> current_zones;
    current_zones.reserve(leaf_chunks.size());
    
    for (const auto& [key, chunk] : leaf_chunks) {
        current_zones[key] = chunk->get_aabb();
    }

    // Оптимизация: если размеры совпадают и все ключи на месте, изменений нет
    if (current_zones.size() == _prev_active_zones.size()) {
        bool identical = true;
        for (const auto& [key, _] : current_zones) {
            if (_prev_active_zones.find(key) == _prev_active_zones.end()) {
                identical = false;
                break;
            }
        }
        if (identical) return; // Выходим без лишних вычислений
    }

    std::vector<ZoneLODUpdate> updates;

    // Находим добавленные зоны (LOD = 0, базовый уровень)
    for (const auto& [key, aabb] : current_zones) {
        if (_prev_active_zones.find(key) == _prev_active_zones.end()) {
            updates.push_back({aabb, 0}); 
        }
    }

    // Находим удаленные зоны (LOD = LOD_UNLOADED)
    for (const auto& [key, aabb] : _prev_active_zones) {
        if (current_zones.find(key) == current_zones.end()) {
            updates.push_back({aabb, LOD_UNLOADED}); 
        }
    }

    // Уведомляем SemanticWorld только если есть реальные изменения
    if (!updates.empty()) {
        SemanticWorld* world = SemanticWorld::get_singleton();
        if (world) {
            world->on_zone_changed(updates);
        }
    }

    // Сохраняем текущее состояние для сравнения в следующем кадре
    _prev_active_zones = std::move(current_zones);
}