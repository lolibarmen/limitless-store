#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <cstdint>

namespace godot {

class SemanticWorld; 

class SemanticShape : public Resource {
    GDCLASS(SemanticShape, Resource)

private:
    uint64_t _id = 0;
    SemanticWorld* _world = nullptr;

protected:
    AABB _aabb;
    bool _aabb_dirty = true;

    static void _bind_methods();

public:
    SemanticShape();
    virtual ~SemanticShape(); 

    void set_id(uint64_t id) { _id = id; }
    uint64_t get_id() const { return _id; }

    void set_world(SemanticWorld* world) { _world = world; }
    SemanticWorld* get_world() const { return _world; }

    virtual AABB get_aabb() const;
    virtual void recompute_aabb() { _aabb_dirty = true; }
    
    void notify_shape_changed();

    virtual String get_shape_type() const { return "shape"; }
    virtual float evaluate_sdf(const Vector3& world_pos) const;
};

} // namespace godot