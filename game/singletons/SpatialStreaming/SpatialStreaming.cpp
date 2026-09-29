#include "SpatialStreaming.hpp"
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Инклуды для дебажной визуализации
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

using namespace godot;

void SpatialStreaming::_bind_methods() {}

SpatialStreaming* SpatialStreaming::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;

    Object* obj = engine->get_singleton("SpatialStreaming");
    return Object::cast_to<SpatialStreaming>(obj);
}

void SpatialStreaming::_ready() {}

void SpatialStreaming::_sync_debug_mesh(Chunk* chunk) {
    if (!chunk) return;

    if (chunk->is_leaf()) {
        if (!chunk->debug_mesh) {
            chunk->debug_mesh = memnew(MeshInstance3D);
            
            Ref<ArrayMesh> wireframe_mesh;
            wireframe_mesh.instantiate();

            AABB aabb = chunk->get_aabb();
            
            Vector3 half_size = aabb.size * 0.5;
            Vector3 min = -half_size;
            Vector3 max = half_size;

            PackedVector3Array vertices;
            
            // Нижняя грань
            vertices.push_back(Vector3(min.x, min.y, min.z)); vertices.push_back(Vector3(max.x, min.y, min.z));
            vertices.push_back(Vector3(max.x, min.y, min.z)); vertices.push_back(Vector3(max.x, min.y, max.z));
            vertices.push_back(Vector3(max.x, min.y, max.z)); vertices.push_back(Vector3(min.x, min.y, max.z));
            vertices.push_back(Vector3(min.x, min.y, max.z)); vertices.push_back(Vector3(min.x, min.y, min.z));
            
            // Верхняя грань
            vertices.push_back(Vector3(min.x, max.y, min.z)); vertices.push_back(Vector3(max.x, max.y, min.z));
            vertices.push_back(Vector3(max.x, max.y, min.z)); vertices.push_back(Vector3(max.x, max.y, max.z));
            vertices.push_back(Vector3(max.x, max.y, max.z)); vertices.push_back(Vector3(min.x, max.y, max.z));
            vertices.push_back(Vector3(min.x, max.y, max.z)); vertices.push_back(Vector3(min.x, max.y, min.z));
            
            // Вертикальные ребра
            vertices.push_back(Vector3(min.x, min.y, min.z)); vertices.push_back(Vector3(min.x, max.y, min.z));
            vertices.push_back(Vector3(max.x, min.y, min.z)); vertices.push_back(Vector3(max.x, max.y, min.z));
            vertices.push_back(Vector3(max.x, min.y, max.z)); vertices.push_back(Vector3(max.x, max.y, max.z));
            vertices.push_back(Vector3(min.x, min.y, max.z)); vertices.push_back(Vector3(min.x, max.y, max.z));

            Array arrays;
            arrays.resize(Mesh::ARRAY_MAX);
            arrays[Mesh::ARRAY_VERTEX] = vertices;

            wireframe_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, arrays);
            
            // --- ВЫЧИСЛЕНИЕ ЦВЕТА В ЗАВИСИМОСТИ ОТ РАЗМЕРА ---
            
            // Находим максимальное измерение (длину, ширину или высоту)
            float max_dim = MAX(aabb.size.x, MAX(aabb.size.y, aabb.size.z));
            max_dim = chunk->depth;
            
            Color chunk_color;
            
            // ВАРИАНТ 1: Линейный градиент (от синего к красному)
            // // Замените 128.0 на ваш максимально возможный размер чанка в сцене
            float max_possible_size = 3.0; 
            float t = CLAMP(1.0 - max_dim / max_possible_size, 0.0, 1.0);
            chunk_color = Color(t, 1.0, 1.0); // t=0 -> Синий, t=1 -> Красный


            // ВАРИАНТ 2: Логарифмический градиент (Идеально для Октодере)
            // Если размеры чанков всегда степени двойки (1, 2, 4, 8, 16, 32...)
            // float log_size = Math::log(MAX(max_dim, 0.001)) / Math::log(2.0);
            // float hue = fmod(log_size / 3.0, 1.0); // 6.0 - это кол-во уровней (например, от 1 до 64)
            // chunk_color = Color::from_hsv(hue, 1.0, 1.0);
            
            // -------------------------------------------------

            Ref<StandardMaterial3D> mat;
            mat.instantiate();
            mat->set_albedo(chunk_color); // Используем вычисленный цвет
            mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
            mat->set_cull_mode(StandardMaterial3D::CULL_DISABLED);

            chunk->debug_mesh->set_mesh(wireframe_mesh);
            chunk->debug_mesh->set_material_override(mat);

            add_child(chunk->debug_mesh);

            chunk->debug_mesh->set_global_position(chunk->center);
        }
    } else {
        if (chunk->debug_mesh) {
            chunk->debug_mesh->queue_free();
            chunk->debug_mesh = nullptr;
        }
    }
}

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
        //_sync_debug_mesh(raw);
    }
}

void SpatialStreaming::delete_children(Chunk* n) {
    if (!n || n->is_leaf()) return;
    
    for (auto& child_ptr : n->children) {
        if (child_ptr) {
            Chunk* child = child_ptr.get();
            
            if (child->is_leaf()) {
                Vector3i child_key((int)child->center.x, (int)child->center.y, (int)child->center.z);
                leaf_chunks.erase(child_key);
            }
            
            // Очищаем дебажный меш ребенка перед уничтожением
            child->clear_debug_mesh();
            
            delete_children(child);
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
        Vector3i parent_key((int)n->center.x, (int)n->center.y, (int)n->center.z);
        leaf_chunks.erase(parent_key);
        
        n->clear_debug_mesh();

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
                    
                    //_sync_debug_mesh(new_child);
                    
                    i++;
                }
            }
        }
    }
    else if (!n->is_leaf() && should_collapse) {
        // delete_children рекурсивно вызовет clear_debug_mesh() для всех детей
        delete_children(n);
        
        Vector3i key((int)n->center.x, (int)n->center.y, (int)n->center.z);
        leaf_chunks[key] = n;
        
        //_sync_debug_mesh(n);
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

    // 1. Удаляем дальние корни
    for (auto it = roots.begin(); it != roots.end(); ) {
        Vector3i d = it->first - pc;
        if (abs(d.x) > root_radius || abs(d.y) > root_radius || abs(d.z) > root_radius) {
            delete_children(it->second.get());
            
            // Очищаем дебажный меш корня перед удалением
            it->second->clear_debug_mesh();
            
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
            int lod = MAX_DEPTH - leaf_chunks.at(key)->depth; 
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