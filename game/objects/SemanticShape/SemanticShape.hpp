#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <cstdint>

namespace godot {

class MeshGenerator;

class SemanticShape : public Resource {
    GDCLASS(SemanticShape, Resource)

private:
    uint64_t _id = 0;

    godot::PackedInt64Array _owned_shape_ids;
    godot::PackedInt64Array _needed_shape_ids;

protected:
    AABB _aabb;
    bool _aabb_dirty = true;

    static void _bind_methods();

public:
    SemanticShape();
    virtual ~SemanticShape(); 

    void set_id(uint64_t id) { _id = id; }
    uint64_t get_id() const { return _id; }

    virtual AABB get_aabb() const;
    virtual void recompute_aabb() { _aabb_dirty = true; }
    
    void notify_shape_changed();

    virtual String get_shape_type() const { return "shape"; }
    virtual float evaluate_sdf(const Vector3& world_pos) const;

    // --- OWN (Владелец) ---
    void add_owned_shape(uint64_t id);
    void remove_owned_shape(uint64_t id);
    bool has_owned_shape(uint64_t id) const;
    godot::PackedInt64Array get_owned_shapes() const;
    void set_owned_shapes(const godot::PackedInt64Array& ids);

    // --- NEED (Зависимость) ---
    void add_needed_shape(uint64_t id);
    void remove_needed_shape(uint64_t id);
    bool has_needed_shape(uint64_t id) const;
    godot::PackedInt64Array get_needed_shapes() const;
    void set_needed_shapes(const godot::PackedInt64Array& ids);

    virtual Ref<MeshGenerator> get_mesh_generator() const;
};

} // namespace godot