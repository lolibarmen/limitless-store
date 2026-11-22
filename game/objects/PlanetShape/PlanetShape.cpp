#include "PlanetShape.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <GravityShape/GravityShape.hpp>
#include <SurfaceShape/SurfaceShape.hpp>
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

void PlanetShape::set_position(const Vector3& v) {
    if (ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton()) {
        mgr->set_octree_position(get_id(), v);
    }
}

Vector3 PlanetShape::get_position() const {
    if (ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton()) {
        if (ChunkOctree* octree = mgr->get_octree(get_id())) {
            return octree->get_position();
        }
    }
    return Vector3();
}

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
    }

    _surface_id = 0;
    _gravity_id = 0;
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

    if (_surface_id != 0) {
        return;
    }

    // Поверхность — дочерняя фигура планеты. Параметры она читает у планеты,
    // а рендерится в общем октодереве через bind_spatially() в своём on_register().
    Ref<SurfaceShape> surface;
    surface.instantiate();
    surface->bind_planet(Ref<PlanetShape>(this));
    sw->register_shape(surface);

    _surface_id = surface->get_id();
    add_owned_shape(_surface_id);
}
