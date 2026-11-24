#include "PlanetShape.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <ForestShape/ForestShape.hpp>
#include <GravityShape/GravityShape.hpp>
#include <SurfaceShape/SurfaceShape.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace {
constexpr int kMinChunkSize = 16;

int compute_max_depth(float size) {
    int root_size = kMinChunkSize;
    int depth = 0;
    while (static_cast<float>(root_size) < size) {
        root_size *= 2;
        ++depth;
    }
    return depth;
}
} // namespace

void PlanetShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &PlanetShape::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &PlanetShape::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");

    ClassDB::bind_method(D_METHOD("set_amplitude", "amplitude"), &PlanetShape::set_amplitude);
    ClassDB::bind_method(D_METHOD("get_amplitude"), &PlanetShape::get_amplitude);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");
}

PlanetShape::PlanetShape() = default;

void PlanetShape::recompute_aabb() {
    const float r = _radius + _amplitude;
    _aabb = AABB(-Vector3(r, r, r), Vector3(r * 2.0f, r * 2.0f, r * 2.0f));
    _aabb_dirty = false;
}

Ref<GravityShape> PlanetShape::get_gravity() const {
    if (_gravity_id == 0) {
        return nullptr;
    }

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) {
        return nullptr;
    }

    Ref<SemanticShape> base = sw->get_shape(_gravity_id);
    return Object::cast_to<GravityShape>(base.ptr());
}

void PlanetShape::notify_shape_changed() {
    SemanticShape::notify_shape_changed();

    if (get_id() == 0) {
        return;
    }

    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) {
        return;
    }

    const float r = _radius + _amplitude;
    mgr->rebuild_octree(get_id(), r * 2.0f, compute_max_depth(r * 2.0f));
}

void PlanetShape::on_unregister() {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (sw) {
        if (_gravity_id != 0 && sw->get_shape(_gravity_id).is_valid()) {
            sw->unregister_shape_with_owned(_gravity_id);
        }
        if (_surface_id != 0 && sw->get_shape(_surface_id).is_valid()) {
            sw->unregister_shape_with_owned(_surface_id);
        }
        if (_forest_id != 0 && sw->get_shape(_forest_id).is_valid()) {
            sw->unregister_shape_with_owned(_forest_id);
        }
    }

    _surface_id = 0;
    _gravity_id = 0;
    _forest_id = 0;
    set_owned_shapes(PackedInt64Array());
}

void PlanetShape::on_register() {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (!sw || !mgr) {
        return;
    }

    // Собственное октодерево планеты: к нему привязываются все фигуры внутри неё.
    const float r = _radius + _amplitude;
    mgr->create_octree(get_id(), get_position(), r * 2.0f, compute_max_depth(r * 2.0f));

    if (_gravity_id == 0) {
        Ref<GravityShape> gravity;
        gravity.instantiate();
        gravity->bind_planet(Ref<PlanetShape>(this));
        sw->register_shape(gravity);
        _gravity_id = gravity->get_id();
        add_owned_shape(_gravity_id);
    }

    if (_surface_id == 0) {
        Ref<SurfaceShape> surface;
        surface.instantiate();
        surface->bind_planet(Ref<PlanetShape>(this));
        sw->register_shape(surface);

        _surface_id = surface->get_id();
        add_owned_shape(_surface_id);
    }

    if (_forest_id != 0) {
        return;
    }

    Ref<Resource> preset = ResourceLoader::get_singleton()->load("res://objects/world/tree_test.tres");
    Ref<SemanticShape> tree_shape(Object::cast_to<SemanticShape>(preset.ptr()));

    Ref<ForestShape> forest;
    forest.instantiate();
    forest->bind_planet(Ref<PlanetShape>(this));
    forest->set_tree_shape(tree_shape);
    sw->register_shape(forest);

    if (forest->get_id() != 0) {
        mgr->bind_spatially(forest->get_id(), get_id());
    }

    _forest_id = forest->get_id();
    add_owned_shape(_forest_id);
}
