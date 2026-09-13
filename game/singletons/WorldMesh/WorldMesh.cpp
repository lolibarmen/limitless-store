#include "WorldMesh.hpp"
#include <ChunkMeshNode/ChunkMeshQueue.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <functional>

using namespace godot;

void WorldMesh::_bind_methods() {}

WorldMesh* WorldMesh::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) {
        return nullptr;
    }

    Object* obj = engine->get_singleton("WorldMesh");
    return Object::cast_to<WorldMesh>(obj);
}

void WorldMesh::_ready() {}

void WorldMesh::_process(double delta) {
    auto* vp = get_viewport();
    if (!vp) return;
    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    player_pos = cam->get_global_position();

    update_roots();
    for (auto& [cell, root] : roots) {
        update_recurs(root);
    }

    // ChunkMeshQueue::get_singleton().tick(9); // Раскомментируйте при необходимости
}

void WorldMesh::spawn_chunk(ChunkNode* n) {
    if (!n) return;
    n->set_position(n->center);
    n->set_lod_level(MAX_DEPTH - n->depth);
    n->set_chunk_size(n->size);
    
    add_child(n);
}

void WorldMesh::despawn_chunk(ChunkNode* n) {
    if (n) {
        n->queue_free();
    }
}

void WorldMesh::delete_children(ChunkNode* n) {
    if (!n || n->is_leaf()) return;
    
    for (auto& c : n->children) {
        if (c) {
            delete_children(c);
            despawn_chunk(c);
            c = nullptr;
        }
    }
}

float cube_distance(Vector3 a, Vector3 b) {
    float r_x = Math::abs(a.x - b.x);
    float r_y = Math::abs(a.y - b.y);
    float r_z = Math::abs(a.z - b.z);
    return Math::max(Math::max(r_x, r_y), r_z);
}

void WorldMesh::update_recurs(ChunkNode* n) {
    if (!n) return;

    float dist = cube_distance(n->center, player_pos);
    bool should_split    = dist < n->size * 4.0f;
    bool should_collapse = dist > n->size * 4.1f;

    if (n->is_leaf() && should_split && n->depth < MAX_DEPTH) {
        n->clear_meshes();

        float h = n->size / 2.0f, q = h / 2.0f;
        int i = 0;
        for (int x : {-1, 1}) {
            for (int y : {-1, 1}) {
                for (int z : {-1, 1}) {
                    n->children[i] = memnew(ChunkNode(n->center + Vector3(x, y, z) * q, h, n->depth + 1, n));
                    spawn_chunk(n->children[i]);
                    i++;
                }
            }
        }
    }
    else if (!n->is_leaf() && should_collapse) {
        delete_children(n);
        n->generate(); 
    }
    else if (!n->is_leaf()) {
        for (auto* c : n->children) {
            if (c) update_recurs(c);
        }
    }
}

void WorldMesh::update_roots() {
    Vector3i pc = Vector3i(
        (int)Math::floor(player_pos.x / ROOT_SIZE),
        (int)Math::floor(player_pos.y / ROOT_SIZE),
        (int)Math::floor(player_pos.z / ROOT_SIZE)
    );

    for (auto it = roots.begin(); it != roots.end(); ) {
        Vector3i d = it->first - pc;
        if (abs(d.x) > root_radius || abs(d.y) > root_radius || abs(d.z) > root_radius) {
            delete_children(it->second);
            despawn_chunk(it->second);
            it = roots.erase(it);
        } else {
            ++it;
        }
    }

    for (int dx = -root_radius; dx <= root_radius; dx++) {
        for (int dy = -root_radius; dy <= root_radius; dy++) {
            for (int dz = -root_radius; dz <= root_radius; dz++) {
                Vector3i cell = pc + Vector3i(dx, dy, dz);
                if (!roots.count(cell)) {
                    Vector3 center = (Vector3(cell) + Vector3(0.5f, 0.5f, 0.5f)) * ROOT_SIZE;
                    auto root = memnew(ChunkNode(center, ROOT_SIZE, 0, nullptr));
                    spawn_chunk(root);
                    roots[cell] = root;
                }
            }
        }
    }
}

void WorldMesh::refresh_chunks_in_aabb(const AABB& bounds) {
    std::function<void(ChunkNode*)> check_and_refresh = [&](ChunkNode* n) {
        if (!n) return;
        
        if (n->is_leaf()) {
            float half = n->size / 2.0f;
            AABB chunk_aabb(
                n->center - Vector3(half, half, half), 
                Vector3(half * 2.0f, half * 2.0f, half * 2.0f)
            );
            
            if (chunk_aabb.intersects(bounds)) {
                n->generate();
            }
        } else {
            for (auto* c : n->children) {
                if (c) check_and_refresh(c);
            }
        }
    };

    for (auto& [cell, root] : roots) {
        check_and_refresh(root);
    }
}

void WorldMesh::register_shape(uint64_t id) {
    if (_registered_shape_ids.find(id) == _registered_shape_ids.end()) {
        _registered_shape_ids.insert(id);
        update_mesh(id);
    }
}
void WorldMesh::unregister_shape(uint64_t id) {
    _registered_shape_ids.erase(id);
}

void WorldMesh::update_mesh(uint64_t id) {
    Ref<SemanticShape> shape = SemanticWorld::get_singleton()->get_shape(id);
    if (shape.is_null()) return;

    Ref<MeshGenerator> generator = shape->get_mesh_generator();
    if (generator.is_null()) return;

    SemanticWorld* world_ptr = nullptr;
    Object* sw_obj = Engine::get_singleton()->get_singleton("SemanticWorld");
    if (sw_obj) {
        world_ptr = Object::cast_to<SemanticWorld>(sw_obj);
    }

    Ref<ArrayMesh> mesh = generator->generate(shape, world_ptr);
    if (mesh.is_null()) return;

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    mesh_instance->set_mesh(mesh);

    AABB aabb = shape->get_aabb();
    Vector3 center = aabb.get_center();

    ChunkNode* target_chunk = find_leaf_chunk(center);
    if (target_chunk) {
        target_chunk->add_mesh_instance(mesh_instance);
    } else {
        mesh_instance->queue_free();
    }
}

ChunkNode* WorldMesh::find_leaf_chunk(const Vector3& pos) const {
    Vector3i pc = Vector3i(
        (int)Math::floor(pos.x / ROOT_SIZE),
        (int)Math::floor(pos.y / ROOT_SIZE),
        (int)Math::floor(pos.z / ROOT_SIZE)
    );
    
    auto it = roots.find(pc);
    if (it == roots.end()) return nullptr;

    ChunkNode* current = it->second;
    
    // Спускаемся по дереву до листового узла, содержащего точку
    while (!current->is_leaf()) {
        ChunkNode* next_child = nullptr;
        for (int i = 0; i < 8; i++) {
            if (current->children[i]) {
                float half = current->children[i]->size / 2.0f;
                Vector3 min = current->children[i]->center - Vector3(half, half, half);
                Vector3 max = current->children[i]->center + Vector3(half, half, half);
                
                if (pos.x >= min.x && pos.x <= max.x &&
                    pos.y >= min.y && pos.y <= max.y &&
                    pos.z >= min.z && pos.z <= max.z) {
                    next_child = current->children[i];
                    break;
                }
            }
        }
        if (!next_child) break;
        current = next_child;
    }
    
    return current;
}

void WorldMesh::complete_mesh(uint64_t id, Ref<ArrayMesh> mesh) {
    if (mesh.is_null()) return;

    if (_registered_shape_ids.find(id) == _registered_shape_ids.end()) {
        return; 
    }

    Ref<SemanticShape> shape = SemanticWorld::get_singleton()->get_shape(id);
    if (shape.is_null()) return;

    Vector3 center = shape->get_aabb().get_center();
    ChunkNode* target_chunk = find_leaf_chunk(center);

    if (target_chunk) {
        MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
        mesh_instance->set_mesh(mesh);
        target_chunk->add_mesh_instance(mesh_instance);
    }
}