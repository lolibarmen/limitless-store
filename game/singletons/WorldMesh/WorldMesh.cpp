#include "WorldMesh.hpp"
#include "RenderRequest.hpp"
#include <AlchemyArea3D/AlchemyArea3D.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <algorithm>
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
    auto it = _active_renders.find(token);
    if (it == _active_renders.end()) {
        return;
    }
    if (it->second.pending > 0) {
        --it->second.pending;
    }
    if (it->second.pending == 0) {
        _active_renders.erase(it);
    }
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
    if (!request || !request->active || request->token != job.token) {
        return false;
    }

    SemanticWorld* world = SemanticWorld::get_singleton();
    if (!world) return false;

    const Ref<SemanticShape> shape = world->get_shape(job.shape_id);
    if (shape.is_null()) return false;

    const Vector3 shape_origin = shape->get_position();
    request->shape_origin = shape_origin;

    if (job.visual_generator.is_valid() && !request->mesh_instance) {
        MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
        octree->add_child(mesh_instance);
        mesh_instance->set_position(shape_origin);
        mesh_instance->set_visible(false);
        request->mesh_instance = mesh_instance;
    }

    Engine* engine = Engine::get_singleton();
    const bool editor = engine && engine->is_editor_hint();

    if (!editor && job.collision_generator.is_valid() && !request->collision_body) {
        StaticBody3D* collision_body = memnew(StaticBody3D);
        octree->add_child(collision_body);
        collision_body->set_position(shape_origin);

        CollisionShape3D* collision_shape = memnew(CollisionShape3D);
        collision_shape->set_disabled(true);
        collision_body->add_child(collision_shape);

        request->collision_body = collision_body;
        request->collision_shape = collision_shape;
    }

    if (job.area_generator.is_valid() && !request->area) {
        AlchemyArea3D* area = memnew(AlchemyArea3D);
        octree->add_child(area);
        area->set_position(shape_origin);
        area->set_shape_id(job.shape_id);

        CollisionShape3D* area_shape = memnew(CollisionShape3D);
        area_shape->set_disabled(true);
        area->add_child(area_shape);

        request->area = area;
        request->area_shape = area_shape;
    }

    request->material_generator = job.material_generator;

    struct GeneratorSlot {
        Ref<MeshGenerator> generator;
        uint32_t mask = 0;
    };

    std::vector<GeneratorSlot> slots;
    auto add_slot = [&slots](const Ref<MeshGenerator>& generator, uint32_t mask) {
        if (generator.is_null()) return;
        for (GeneratorSlot& slot : slots) {
            if (slot.generator.ptr() == generator.ptr()) {
                slot.mask |= mask;
                return;
            }
        }
        slots.push_back({generator, mask});
    };

    add_slot(job.visual_generator, MESH_TARGET_VISUAL);
    add_slot(job.collision_generator, MESH_TARGET_COLLISION);
    add_slot(job.area_generator, MESH_TARGET_AREA);

    std::stable_sort(slots.begin(), slots.end(), [](const GeneratorSlot& a, const GeneratorSlot& b) {
        return a.generator->is_async() < b.generator->is_async();
    });

    _active_renders[job.token] = ActiveRender{job.shape_id, job.chunk_id, (int)slots.size()};

    for (const GeneratorSlot& slot : slots) {
        request->target_mask = slot.mask;
        slot.generator->generate(job.shape_id, job.chunk_id);
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

void WorldMesh::request_render(uint64_t shape_id, uint64_t chunk_id,
                               Ref<MeshGenerator> visual_mesh_generator,
                               Ref<MeshGenerator> collision_mesh_generator,
                               Ref<MeshGenerator> area_mesh_generator,
                               Ref<MaterialGenerator> material_generator) {
    if (shape_id == 0) return;
    if (visual_mesh_generator.is_null() && collision_mesh_generator.is_null() && area_mesh_generator.is_null()) {
        return;
    }

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) { UtilityFunctions::printerr("WorldMesh::request_render: ShapeSpatialManager не найден."); return; }
    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) { UtilityFunctions::printerr("WorldMesh::request_render: Октодерево для shape_id ", shape_id, " не найдено!"); return; }
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
    request->material_generator = material_generator;

    chunk->shape_requests[shape_id] = std::move(request);

    _pending_tokens[shape_id][chunk_id] = token;
    _pending_renders.push_back(PendingRender{
        shape_id, chunk_id, token,
        visual_mesh_generator, collision_mesh_generator, area_mesh_generator, material_generator
    });

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

void WorldMesh::complete_mesh(uint64_t shape_id, uint64_t chunk_id, uint64_t token, Ref<ArrayMesh> mesh) {
    if (token == 0) return;

    auto active_it = _active_renders.find(token);
    if (active_it == _active_renders.end()) return;

    shape_id = active_it->second.shape_id;
    chunk_id = active_it->second.chunk_id;

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (mgr) {
        ChunkOctree* octree = mgr->get_octree(shape_id);
        if (octree) {
            Chunk* chunk = octree->find(chunk_id);
            if (chunk) {
                auto it = chunk->shape_requests.find(shape_id);
                if (it != chunk->shape_requests.end() && it->second) {
                    RenderRequest* request = it->second.get();
                    if (request) {
                        request->apply(shape_id, chunk_id, token, mesh, request->target_mask);
                    }
                }
            }
        }
    }

    _release_active(token);
    _pump_renders();
}
