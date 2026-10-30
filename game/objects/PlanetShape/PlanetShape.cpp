#include "PlanetShape.hpp"
#include <PlanetTerrainShape/PlanetTerrainShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
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

    ClassDB::bind_method(D_METHOD("set_textures", "textures"), &PlanetShape::set_textures);
    ClassDB::bind_method(D_METHOD("get_textures"), &PlanetShape::get_textures);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "textures", PROPERTY_HINT_ARRAY_TYPE, "string"), "set_textures", "get_textures");
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

    _parts.clear();

    // Одна пространственная сетка на всю планету: дерево создаёт сама планета,
    // все её фигуры регистрируются в нём.
    const float r = _radius + _amplitude;
    const float root_size = r * 2.0f;

    // Дистанции, на которых происходит переход на более глубокий уровень октодерева
    // (по убыванию). Их количество задаёт максимальную глубину детализации.
    const std::vector<float> split_distances = {
        root_size * 4.0f,       // переход с уровня 0 на уровень 1
        root_size * 4.0f * 0.5f // переход с уровня 1 на уровень 2
    };

    mgr->create_octree(get_id(), _center, root_size, split_distances);

    for (int i = 0; i < _textures.size(); ++i) {
        Ref<PlanetTerrainShape> part;
        part.instantiate();
        part->set_center(_center);
        part->set_radius(_radius);
        part->set_amplitude(_amplitude);
        part->set_texture_path(_textures[i]);
        part->set_octree_id(get_id()); // использует дерево планеты, своё не создаёт

        uint64_t part_id = sw->register_shape(part);
        _parts.push_back(part);
        add_owned_shape(part_id);

        mgr->bind_spatially(part_id, get_id());
    }
}
