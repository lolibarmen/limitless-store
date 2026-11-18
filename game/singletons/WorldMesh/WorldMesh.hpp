#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <cstdint>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace godot {

class Area3D;
class Chunk;

class WorldMesh : public Node3D {
    GDCLASS(WorldMesh, Node3D)

private:
    mutable std::mutex _tokens_mutex;
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> _tokens;
    uint64_t _next_token = 1;

    std::unordered_set<uint64_t> _dirty_shapes;
    double _dirty_timer = 0.0;

    struct PendingRender {
        uint64_t shape_id = 0;
        uint64_t chunk_id = 0;
        uint64_t token = 0;
        Ref<MeshGenerator> visual_generator;
        Ref<MeshGenerator> collision_generator;
        Ref<MeshGenerator> area_generator;
        Ref<MaterialGenerator> material_generator;
    };

    struct ActiveRender {
        uint64_t shape_id = 0;
        uint64_t chunk_id = 0;
        int pending = 0;
    };

    struct PendingAlchemy {
        uint64_t self_id = 0;
        Ref<SemanticShape> proposed;
    };

    std::deque<PendingRender> _pending_renders;
    std::unordered_map<uint64_t, ActiveRender> _active_renders;
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> _pending_tokens;
    std::vector<PendingAlchemy> _pending_alchemy;

    int _max_concurrent_renders = 0;
    bool _pumping_renders = false;

    void _pump_renders();
    bool _start_render(const PendingRender& job);
    void _cancel_pending(uint64_t shape_id, uint64_t chunk_id);
    void _release_active(uint64_t token);
    void _on_render_task_finished(uint64_t shape_id, uint64_t chunk_id, uint64_t token);
    void _remove_mesh_from_chunk(Chunk* chunk, uint64_t shape_id);
    void _clear_shape_in_subtree(Chunk* node, uint64_t shape_id);
    void _flush_invalidations();
    void _flush_alchemy();
    void _on_alchemy_area_entered(Area3D* other, int64_t self_id);

protected:
    static void _bind_methods();

public:
    static WorldMesh* get_singleton();

    void _ready() override;
    void _process(double delta) override;

    int get_max_concurrent_renders() const { return _max_concurrent_renders; }
    void set_max_concurrent_renders(int count);

    void request_render(uint64_t shape_id, uint64_t chunk_id,
                        Ref<MeshGenerator> visual_mesh_generator,
                        Ref<MeshGenerator> collision_mesh_generator,
                        Ref<MeshGenerator> area_mesh_generator,
                        Ref<MaterialGenerator> material_generator);
    void cancel_render(uint64_t shape_id, uint64_t chunk_id);
    void cancel_request(uint64_t shape_id, uint64_t chunk_id);
    void complete_mesh(uint64_t shape_id, uint64_t chunk_id, uint64_t token, Ref<ArrayMesh> mesh);
    uint64_t current_token(uint64_t shape_id, uint64_t chunk_id) const;
    bool is_current(uint64_t shape_id, uint64_t chunk_id, uint64_t token) const;
    void invalidate_shape(uint64_t shape_id);
    void drop_shape(uint64_t shape_id);
};

} // namespace godot
