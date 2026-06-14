#include "NeochunkManager.hpp"
#include <functional>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <NeochunkNode/ChunkMeshQueue.hpp>

using namespace godot;

void NeochunkManager::_ready() {

}

void NeochunkManager::_process(double delta) {
    auto* vp = get_viewport();
    if (!vp) return;
    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    player_pos = cam->get_global_position();

    update_roots();
    for (auto& [cell, root] : roots)
        update_recurs(root);

    ChunkMeshQueue::get_singleton().tick(9);
}

void NeochunkManager::spawn_mesh(Neochunk* n) {
    // print_line(" - spawn");
    auto* node = memnew(NeochunkNode);
    node->set_position(n->center);
    node->set_block_source(block_source);
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
    if (!n->is_leaf())
        for (auto& c : n->children) {
            delete_childrens(c);
            despawn_mesh(c);
            delete c;
            c = nullptr;
        }
}

float cube_distance(Vector3 a, Vector3 b) {
    float r_x = Math::abs(a.x - b.x);
    float r_y = Math::abs(a.y - b.y);
    float r_z = Math::abs(a.z - b.z);

    return Math::max(Math::max(r_x, r_y), r_z);
}

void NeochunkManager::update_recurs(Neochunk* n) {
    // float dist = n->center.distance_to(player_pos);
    // bool should_split    = dist < n->size * 3.0f;
    // bool should_collapse = dist > n->size * 3.1f;

    float dist = cube_distance(n->center, player_pos);
    bool should_split    = dist < n->size * 4.0f;
    bool should_collapse = dist > n->size * 4.1f;

    if (n->is_leaf() && should_split && n->depth < MAX_DEPTH) {
        despawn_mesh(n);

        float h = n->size / 2.0f, q = h / 2.0f;
        int i = 0;
        for (int x : {-1,1}) for (int y : {-1,1}) for (int z : {-1,1}) {
            n->children[i] = new Neochunk(n->center + Vector3(x,y,z) * q, h, n->depth + 1, n);
            spawn_mesh(n->children[i]);
            i++;
        }
    }
    else if (!n->is_leaf() && should_collapse) {
        for (auto& c : n->children) {
            delete_childrens(c);
            despawn_mesh(c);
            delete c;
            c = nullptr;
        }
        spawn_mesh(n);
    }
    else if (!n->is_leaf()) {
        for (auto& c : n->children) update_recurs(c);
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
            delete_childrens(it->second);
            despawn_mesh(it->second);
            delete it->second;
            it = roots.erase(it);
        } else ++it;
    }

    for (int dx = -root_radius; dx <= root_radius; dx++)
    for (int dy = -root_radius; dy <= root_radius; dy++)
    for (int dz = -root_radius; dz <= root_radius; dz++) {
        Vector3i cell = pc + Vector3i(dx, dy, dz);
        if (!roots.count(cell)) {
            Vector3 center = (Vector3(cell) + Vector3(0.5f, 0.5f, 0.5f)) * ROOT_SIZE;
            auto root = new Neochunk(center, ROOT_SIZE, 0);
            spawn_mesh(root);
            roots[cell] = root;
        }
    }
}

void NeochunkManager::refresh_mesh(const std::vector<Vector3i>& changed_voxels) {
    std::function<void(Neochunk*)> check_and_refresh = [&](Neochunk* n) {
        if (n->is_leaf()) {
            if (!n->node) return;
            // AABB чанка
            float half = n->size / 2.0f;
            Vector3 min = n->center - Vector3(half, half, half);
            Vector3 max = n->center + Vector3(half, half, half);

            for (const auto& v : changed_voxels) {
                // +1 вокрел отступ — чтобы захватить граничащие чанки
                if (v.x >= (int)min.x - 1 && v.x <= (int)max.x + 1 &&
                    v.y >= (int)min.y - 1 && v.y <= (int)max.y + 1 &&
                    v.z >= (int)min.z - 1 && v.z <= (int)max.z + 1) {
                    n->node->generate_mesh();
                    return; // достаточно одного попадания
                }
            }
        } else {
            for (auto* c : n->children)
                if (c) check_and_refresh(c);
        }
    };

    for (auto& [_, root] : roots)
        check_and_refresh(root);
}