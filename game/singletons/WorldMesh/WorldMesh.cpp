#include "WorldMesh.hpp"
#include "RenderRequest.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <memory>
#include <utility>
#include <vector>

using namespace godot;

void WorldMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_max_concurrent_renders", "count"), &WorldMesh::set_max_concurrent_renders);
    ClassDB::bind_method(D_METHOD("get_max_concurrent_renders"), &WorldMesh::get_max_concurrent_renders);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_concurrent_renders"), "set_max_concurrent_renders", "get_max_concurrent_renders");

    ClassDB::bind_method(D_METHOD("_on_render_task_finished", "shape_id", "chunk_id", "token"), &WorldMesh::_on_render_task_finished);
}

WorldMesh* WorldMesh::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("WorldMesh");
    return Object::cast_to<WorldMesh>(obj);
}

void WorldMesh::_ready() {
    set_as_top_level(true);
    set_process(true);

    if (_max_concurrent_renders <= 0) {
        OS* os = OS::get_singleton();
        const int cpus = os ? os->get_processor_count() : 1;
        _max_concurrent_renders = cpus > 1 ? cpus - 1 : 1;
    }
}

void WorldMesh::_process(double delta) {
    if (!_dirty_shapes.empty()) {
        _dirty_timer += delta;
        if (_dirty_timer >= 0.15) {
            _dirty_timer = 0.0;
            _flush_invalidations();
        }
    } else {
        _dirty_timer = 0.0;
    }

    _pump_renders();
}

void WorldMesh::set_max_concurrent_renders(int count) {
    _max_concurrent_renders = count > 0 ? count : 1;
    _pump_renders();
}

void WorldMesh::_release_active(uint64_t token) {
    _active_renders.erase(token);
}

void WorldMesh::_cancel_pending(uint64_t shape_id, uint64_t chunk_id) {
    {
        std::lock_guard<std::mutex> lock(_tokens_mutex);
        auto shape_it = _tokens.find(shape_id);
        if (shape_it != _tokens.end()) {
            shape_it->second.erase(chunk_id);
            if (shape_it->second.empty()) {
                _tokens.erase(shape_it);
            }
        }
    }

    auto shape_it = _pending_tokens.find(shape_id);
    if (shape_it != _pending_tokens.end()) {
        shape_it->second.erase(chunk_id);
        if (shape_it->second.empty()) {
            _pending_tokens.erase(shape_it);
        }
    }
}

void WorldMesh::cancel_request(uint64_t shape_id, uint64_t chunk_id) {
    if (shape_id == 0) return;
    _cancel_pending(shape_id, chunk_id);
}

bool WorldMesh::_start_render(const PendingRender& job) {
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return false;

    ChunkOctree* octree = mgr->get_octree(job.shape_id);
    if (!octree) return false;

    Chunk* chunk = octree->find(job.chunk_id);
    if (!chunk) return false;

    auto it = chunk->shape_requests.find(job.shape_id);
    if (it == chunk->shape_requests.end()) return false;

    RenderRequest* request = it->second.get();
    if (!request || !request->active || request->token != job.token || request->mesh_generator.is_null()) {
        return false;
    }

    if (!request->mesh_instance) {
        MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
        add_child(mesh_instance);
        mesh_instance->set_global_position(chunk->center);
        request->mesh_instance = mesh_instance;

        Engine* engine = Engine::get_singleton();
        const bool editor = engine && engine->is_editor_hint();
        if (!editor && request->mesh_generator->has_collision()) {
            StaticBody3D* collision_body = memnew(StaticBody3D);
            add_child(collision_body);
            collision_body->set_global_position(chunk->center);

            CollisionShape3D* collision_shape = memnew(CollisionShape3D);
            collision_body->add_child(collision_shape);

            request->collision_body = collision_body;
            request->collision_shape = collision_shape;
        }
    }

    _active_renders[job.token] = ActiveRender{job.shape_id, job.chunk_id};

    Ref<MeshGenerator> generator = request->mesh_generator;
    generator->generate(job.shape_id, job.chunk_id);

    if (!generator->is_async()) {
        _release_active(job.token);
    }

    return true;
}

void WorldMesh::_pump_renders() {
    if (_pumping_renders) return;
    _pumping_renders = true;

    while (static_cast<int>(_active_renders.size()) < _max_concurrent_renders && !_pending_renders.empty()) {
        PendingRender job = _pending_renders.front();
        _pending_renders.pop_front();

        auto shape_it = _pending_tokens.find(job.shape_id);
        if (shape_it == _pending_tokens.end()) continue;

        auto chunk_it = shape_it->second.find(job.chunk_id);
        if (chunk_it == shape_it->second.end() || chunk_it->second != job.token) continue;

        shape_it->second.erase(chunk_it);
        if (shape_it->second.empty()) {
            _pending_tokens.erase(shape_it);
        }

        _start_render(job);
    }

    _pumping_renders = false;
}

void WorldMesh::_on_render_task_finished(uint64_t shape_id, uint64_t chunk_id, uint64_t token) {
    (void)shape_id;
    (void)chunk_id;

    _release_active(token);
    _pump_renders();
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
        _pending_tokens.erase(shape_id);

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

    _clear_shape_in_subtree(chunk, shape_id);
    Chunk* current_parent = chunk->parent;
    while (current_parent) {
        if (current_parent->shape_requests.count(shape_id) > 0) {
            _remove_mesh_from_chunk(current_parent, shape_id);
            break;
        }
        current_parent = current_parent->parent;
    }

    uint64_t token = 0;
    {
        std::lock_guard<std::mutex> lock(_tokens_mutex);
        token = _next_token++;
        _tokens[shape_id][chunk_id] = token;
    }

    auto request = std::make_unique<RenderRequest>();
    request->token = token;
    request->mesh_generator = mesh_generator;
    request->material_generator = material_generator;

    chunk->shape_requests[shape_id] = std::move(request);

    _pending_tokens[shape_id][chunk_id] = token;
    _pending_renders.push_back(PendingRender{shape_id, chunk_id, token});

    _pump_renders();
}

void WorldMesh::cancel_render(uint64_t shape_id, uint64_t chunk_id) {
    if (shape_id == 0) return;
    cancel_request(shape_id, chunk_id);

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
    _pending_tokens.erase(shape_id);

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

    _release_active(token);

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (mgr) {
        ChunkOctree* octree = mgr->get_octree(shape_id);
        if (octree) {
            Chunk* chunk = octree->find(chunk_id);
            if (chunk) {
                auto it = chunk->shape_requests.find(shape_id);
                if (it != chunk->shape_requests.end()) {
                    RenderRequest* request = it->second.get();
                    if (request) {
                        request->apply(shape_id, chunk_id, token, mesh, collision_shape);
                    }
                }
            }
        }
    }

    _pump_renders();
}
