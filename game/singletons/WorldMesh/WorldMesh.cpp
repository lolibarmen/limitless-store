#include "WorldMesh.hpp"
#include "RenderRequest.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <memory>
#include <utility>
#include <vector>

using namespace godot;

void WorldMesh::_bind_methods() {}

WorldMesh* WorldMesh::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("WorldMesh");
    return Object::cast_to<WorldMesh>(obj);
}

void WorldMesh::_ready() {
    set_as_top_level(true);
    set_process(true);
}

void WorldMesh::_process(double delta) {
    if (_dirty_shapes.empty()) {
        _dirty_timer = 0.0;
        return;
    }

    _dirty_timer += delta;
    if (_dirty_timer < 0.15) {
        return;
    }

    _dirty_timer = 0.0;
    _flush_invalidations();
}

void WorldMesh::_remove_mesh_from_chunk(Chunk* chunk, uint64_t shape_id) {
    if (!chunk) return;
    chunk->remove_shape(shape_id);
}

void WorldMesh::_clear_shape_in_subtree(Chunk* node, uint64_t shape_id) {
    if (!node) return;

    _remove_mesh_from_chunk(node, shape_id);

    for (auto& child_ptr : node->children) {
        if (child_ptr) {
            _clear_shape_in_subtree(child_ptr.get(), shape_id);
        }
    }
}

void WorldMesh::_flush_invalidations() {
    std::vector<uint64_t> ids(_dirty_shapes.begin(), _dirty_shapes.end());
    _dirty_shapes.clear();

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    for (uint64_t shape_id : ids) {
        {
            std::lock_guard<std::mutex> lock(_tokens_mutex);
            _tokens.erase(shape_id);
        }

        ChunkOctree* octree = mgr->get_octree(shape_id);
        if (!octree) {
            continue;
        }

        octree->for_each_chunk([shape_id](Chunk* chunk) {
            if (chunk) {
                chunk->remove_shape(shape_id);
            }
        });
        octree->mark_user_pending(shape_id);
    }
}

void WorldMesh::request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> mesh_generator, Ref<MaterialGenerator> material_generator) {
    if (shape_id == 0 || mesh_generator.is_null() || material_generator.is_null()) {
        UtilityFunctions::printerr("WorldMesh::request_render: Неверные входные данные.");
        return;
    }

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) { UtilityFunctions::printerr("WorldMesh::request_render: ShapeSpatialManager не найден."); return; }
    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) { UtilityFunctions::printerr("WorldMesh::request_render: Октодерево для shape_id ", shape_id, " не найдено! Фигура не зарегистрирована в ShapeSpatialManager (см. on_register/create_octree)."); return; }
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) { UtilityFunctions::printerr("WorldMesh::request_render: Чанк ", chunk_id, " не найден в октодереве фигуры ", shape_id, "."); return; }

    uint64_t token = 0;
    {
        std::lock_guard<std::mutex> lock(_tokens_mutex);
        token = _next_token++;
        _tokens[shape_id][chunk_id] = token;
    }

    _clear_shape_in_subtree(chunk, shape_id);
    Chunk* current_parent = chunk->parent;
    while (current_parent) {
        if (current_parent->shape_requests.count(shape_id) > 0) {
            _remove_mesh_from_chunk(current_parent, shape_id);
            break;
        }
        current_parent = current_parent->parent;
    }

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);
    mesh_instance->set_global_position(chunk->center);

    auto request = std::make_unique<RenderRequest>();
    request->token = token;
    request->mesh_instance = mesh_instance;
    request->material_generator = material_generator;

    Engine* engine = Engine::get_singleton();
    const bool editor = engine && engine->is_editor_hint();
    if (!editor) {
        StaticBody3D* collision_body = memnew(StaticBody3D);
        add_child(collision_body);
        collision_body->set_global_position(chunk->center);

        CollisionShape3D* collision_shape = memnew(CollisionShape3D);
        collision_body->add_child(collision_shape);

        request->collision_body = collision_body;
        request->collision_shape = collision_shape;
    }

    chunk->shape_requests[shape_id] = std::move(request);

    mesh_generator->generate(shape_id, chunk_id);
}

void WorldMesh::cancel_render(uint64_t shape_id, uint64_t chunk_id) {
    if (shape_id == 0) return;
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (octree) {
        Chunk* chunk = octree->find(chunk_id);
        if (chunk) chunk->remove_shape(shape_id);
    }
}

uint64_t WorldMesh::current_token(uint64_t shape_id, uint64_t chunk_id) const {
    std::lock_guard<std::mutex> lock(_tokens_mutex);
    auto it = _tokens.find(shape_id);
    if (it == _tokens.end()) return 0;
    auto chunk_it = it->second.find(chunk_id);
    return chunk_it == it->second.end() ? 0 : chunk_it->second;
}

bool WorldMesh::is_current(uint64_t shape_id, uint64_t chunk_id, uint64_t token) const {
    if (token == 0) return false;
    return current_token(shape_id, chunk_id) == token;
}

void WorldMesh::invalidate_shape(uint64_t shape_id) {
    if (shape_id == 0) return;
    _dirty_shapes.insert(shape_id);
}

void WorldMesh::drop_shape(uint64_t shape_id) {
    if (shape_id == 0) return;

    _dirty_shapes.erase(shape_id);
    {
        std::lock_guard<std::mutex> lock(_tokens_mutex);
        _tokens.erase(shape_id);
    }

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return;

    octree->for_each_chunk([shape_id](Chunk* chunk) {
        if (chunk) {
            chunk->remove_shape(shape_id);
        }
    });
}

void WorldMesh::complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh, Ref<ConcavePolygonShape3D> collision_shape) {
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return;

    auto it = chunk->shape_requests.find(shape_id);
    if (it == chunk->shape_requests.end()) return;

    complete_mesh(shape_id, chunk_id, it->second->token, mesh, collision_shape);
}

void WorldMesh::complete_mesh(uint64_t shape_id, uint64_t chunk_id, uint64_t token, Ref<ArrayMesh> mesh, Ref<ConcavePolygonShape3D> collision_shape) {
    if (token == 0) return;

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return;

    auto it = chunk->shape_requests.find(shape_id);
    if (it == chunk->shape_requests.end()) return;

    RenderRequest* request = it->second.get();
    if (!request) return;

    request->apply(shape_id, chunk_id, token, mesh, collision_shape);
}
