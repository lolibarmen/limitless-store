#include "PlanetShape.hpp"
#include <PlanetTerrainShape/PlanetTerrainShape.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void PlanetShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_center", "center"), &PlanetShape::set_center);
    ClassDB::bind_method(D_METHOD("get_center"), &PlanetShape::get_center);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");

    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &PlanetShape::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &PlanetShape::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");

    ClassDB::bind_method(D_METHOD("set_amplitude", "amplitude"), &PlanetShape::set_amplitude);
    ClassDB::bind_method(D_METHOD("get_amplitude"), &PlanetShape::get_amplitude);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");
}

PlanetShape::PlanetShape() {
    _grass_shape.instantiate();
    _grass_shape->set_albedo_texture("res://assets/Chunk/grass.png");

    _stone_shape.instantiate();
    _stone_shape->set_albedo_texture("res://assets/Chunk/stone.png");
}

void PlanetShape::recompute_aabb() {
    const float r = _radius + _amplitude;
    _aabb = AABB(_center - Vector3(r, r, r), Vector3(r * 2.0f, r * 2.0f, r * 2.0f));
    _aabb_dirty = false;
}

void PlanetShape::on_register() {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (!sw || !mgr) {
        return;
    }

    // Собственное октодерево планеты.
    const float r = _radius + _amplitude;
    constexpr int kMaxDepth = 3;
    mgr->create_octree(get_id(), _center, r * 2.0f, kMaxDepth);

    // Дочерние фигуры: регистрируются после родителя и привязываются к его дереву.
    Ref<PlanetTerrainShape> shapes[2] = { _grass_shape, _stone_shape };
    for (const Ref<PlanetTerrainShape>& shape : shapes) {
        shape->set_center(_center);
        shape->set_radius(_radius);
        shape->set_amplitude(_amplitude);
        sw->register_shape(shape);
        add_owned_shape(shape->get_id());
        mgr->bind_spatially(shape->get_id(), get_id());
    }
}
