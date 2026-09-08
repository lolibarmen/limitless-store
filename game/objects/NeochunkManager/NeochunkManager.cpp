#include "NeochunkManager.hpp"
#include <ChunkMeshNode/ChunkMeshQueue.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <functional>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void NeochunkManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_on_shape_changed", "shape_id", "bounds"), &NeochunkManager::_on_shape_changed);
}

void NeochunkManager::_ready() {
    print_line(">>> NeochunkManager _ready called");
    SemanticWorld* sw = (SemanticWorld*)Engine::get_singleton()->get_singleton("SemanticWorld");
    if (sw) {
        sw->set_on_shape_changed([this](uint64_t id, const AABB& bounds) {
            call_deferred("_on_shape_changed", id, bounds);
        });
    }

    player_pos = get_global_position();
    update_roots();
}

void NeochunkManager::_process(double delta) {
    auto* vp = get_viewport();
    if (!vp) return;
    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    player_pos = cam->get_global_position();

    update_roots();
    
    int active_nodes = 0;
    for (auto& [cell, root] : roots) {
        update_recurs(root);
    }

    ChunkMeshQueue::get_singleton().tick(9);
}

void NeochunkManager::spawn_mesh(Neochunk* n) {
    auto* node = memnew(NeochunkNode);
    node->set_position(n->center);
    node->set_lod_level(MAX_DEPTH - n->depth);
    node->set_chunk_size(n->size);
    
    add_child(node);
    n->node = node;
}

void NeochunkManager::despawn_mesh(Neochunk* n) {
    if (n->node) {
        n->node->queue_free();
        n->node = nullptr;
    }
}

void NeochunkManager::delete_childrens(Neochunk* n) {
    if (!n->is_leaf()) {
        for (auto& c : n->children) {
            delete_childrens(c);
            despawn_mesh(c);
            delete c;
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

void NeochunkManager::update_recurs(Neochunk* n) {
    if (!n) return;
    
    float dist = cube_distance(n->center, player_pos);
    bool should_split    = dist < n->size * 4.0f;
    bool should_collapse = dist > n->size * 4.1f;

    if (n->is_leaf() && should_split && n->depth < MAX_DEPTH) {
        print_line("--- UPDATE_RECURS: SPLITTING chunk depth ", n->depth, " at dist ", dist, " (limit ", n->size * 4.0f, ")");
        despawn_mesh(n);

        float h = n->size / 2.0f, q = h / 2.0f;
        int i = 0;
        for (int x : {-1, 1}) {
            for (int y : {-1, 1}) {
                for (int z : {-1, 1}) {
                    n->children[i] = new Neochunk(n->center + Vector3(x, y, z) * q, h, n->depth + 1, n);
                    spawn_mesh(n->children[i]);
                    i++;
                }
            }
        }
    }
    else if (!n->is_leaf() && should_collapse) {
        print_line("--- UPDATE_RECURS: COLLAPSING chunk depth ", n->depth, " at dist ", dist, " (limit ", n->size * 4.1f, ")");
        for (auto& c : n->children) {
            if (c) {
                delete_childrens(c);
                despawn_mesh(c);
                delete c;
                c = nullptr;
            }
        }
        spawn_mesh(n);
    }
    else if (!n->is_leaf()) {
        for (auto* c : n->children) {
            if (c) update_recurs(c);
        }
    }
}

void NeochunkManager::update_roots() {
    Vector3i pc = Vector3i(
        (int)Math::floor(player_pos.x / ROOT_SIZE),
        (int)Math::floor(player_pos.y / ROOT_SIZE),
        (int)Math::floor(player_pos.z / ROOT_SIZE)
    );

    for (auto it = roots.begin(); it != roots.end(); ) {
        Vector3i d = it->first - pc;
        if (abs(d.x) > root_radius || abs(d.y) > root_radius || abs(d.z) > root_radius) {
            print_line("!!! UPDATE_ROOTS: REMOVING distant root at cell ", it->first);
            delete_childrens(it->second);
            despawn_mesh(it->second);
            delete it->second;
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
                    print_line(">>> UPDATE_ROOTS: CREATING new root at cell ", cell);
                    Vector3 center = (Vector3(cell) + Vector3(0.5f, 0.5f, 0.5f)) * ROOT_SIZE;
                    auto root = new Neochunk(center, ROOT_SIZE, 0);
                    spawn_mesh(root);
                    roots[cell] = root;
                }
            }
        }
    }
}

void NeochunkManager::_on_shape_changed(uint64_t shape_id, const AABB& bounds) {
    print_line(">>> SHAPE CHANGED: Refreshing chunks in AABB");
    refresh_chunks_in_aabb(bounds.grow(2.0f));
}

void NeochunkManager::refresh_chunks_in_aabb(const AABB& bounds) {
    std::function<void(Neochunk*)> check_and_refresh = [&](Neochunk* n) {
        if (!n) return;
        if (n->is_leaf()) {
            if (!n->node) return;
            
            float half = n->size / 2.0f;
            AABB chunk_aabb(
                n->center - Vector3(half, half, half), 
                Vector3(half * 2.0f, half * 2.0f, half * 2.0f)
            );
            
            if (chunk_aabb.intersects(bounds)) {
                print_line("    Refreshing leaf chunk at ", n->center);
                n->node->generate(); // Убедитесь, что этот метод есть и он корректен
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