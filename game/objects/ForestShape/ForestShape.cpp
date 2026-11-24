#include "ForestShape.hpp"
#include <SemanticWorld/SemanticWorld.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SurfaceShape/SurfaceShape.hpp>
#include <TreeShape/TreeShape.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <Utils/RandomUtils.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

#include <cmath>

using namespace godot;

void ForestShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("bind_planet", "planet"), &ForestShape::bind_planet);
    ClassDB::bind_method(D_METHOD("get_planet"), &ForestShape::get_planet);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "planet", PROPERTY_HINT_RESOURCE_TYPE, "PlanetShape"), "bind_planet", "get_planet");

    ClassDB::bind_method(D_METHOD("set_tree_shape", "shape"), &ForestShape::set_tree_shape);
    ClassDB::bind_method(D_METHOD("get_tree_shape"), &ForestShape::get_tree_shape);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tree_shape", PROPERTY_HINT_RESOURCE_TYPE, "SemanticShape"), "set_tree_shape", "get_tree_shape");

    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &ForestShape::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &ForestShape::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.01,1000.0,0.01"), "set_radius", "get_radius");

    ClassDB::bind_method(D_METHOD("set_tree_count", "count"), &ForestShape::set_tree_count);
    ClassDB::bind_method(D_METHOD("get_tree_count"), &ForestShape::get_tree_count);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "tree_count", PROPERTY_HINT_RANGE, "0,10000,1"), "set_tree_count", "get_tree_count");
}

ForestShape::ForestShape() {
    _area_generator.instantiate();
}

void ForestShape::bind_planet(const Ref<PlanetShape>& planet) {
    _planet = planet;

    if (get_id() != 0) {
        ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
        if (mgr && _planet.is_valid() && _planet->get_id() != 0) {
            mgr->bind_spatially(get_id(), _planet->get_id());
        }
    }

    notify_shape_changed();
    _replant();
}

void ForestShape::set_tree_shape(const Ref<SemanticShape>& shape) {
    _tree_template = shape;
    _replant();
}

void ForestShape::set_radius(float r) {
    _radius = Math::max(r, 0.001f);
    notify_shape_changed();
    _replant();
}

void ForestShape::set_tree_count(int n) {
    _tree_count = Math::max(n, 0);
    notify_shape_changed();
    _replant();
}

void ForestShape::recompute_aabb() {
    const Vector3 extent(_radius, _radius, _radius);
    _aabb = AABB(get_position() - extent, extent * 2.0f);
    _aabb_dirty = false;
}

void ForestShape::on_register() {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (mgr && _planet.is_valid() && _planet->get_id() != 0) {
        mgr->bind_spatially(get_id(), _planet->get_id());
    }
}

void ForestShape::on_unregister() {
    _clear_trees();
    _pending_surface_id = 0;
    _planted_surface_id = 0;
    _planted_surfaces.clear();
}

void ForestShape::on_zone_changed(uint64_t chunk_id) {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    Chunk* chunk = mgr ? mgr->get_chunk(get_id(), chunk_id) : nullptr;
    if (!chunk || !chunk->get_aabb().has_point(get_aabb().get_center())) return;

    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->request_render(get_id(), chunk_id, Ref<MeshGenerator>(), Ref<MeshGenerator>(),
                           _area_generator, Ref<MaterialGenerator>());
    }
}

Ref<SemanticShape> ForestShape::evaluate_alchemy(uint64_t other_id) const {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return nullptr;

    Ref<SemanticShape> other = sw->get_shape(other_id);
    if (Object::cast_to<SurfaceShape>(other.ptr())) {
        _pending_surface_id = other_id;
    }
    return nullptr;
}

void ForestShape::on_alchemy_separation(uint64_t other_id) {
    if (_pending_surface_id == other_id) {
        _pending_surface_id = 0;
    }
}

void ForestShape::consume_pending_surface() {
    if (_pending_surface_id == 0 || get_id() == 0) {
        _pending_surface_id = 0;
        return;
    }

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return;

    const uint64_t surface_id = _pending_surface_id;
    _pending_surface_id = 0;

    Ref<SemanticShape> other = sw->get_shape(surface_id);
    SurfaceShape* surface = Object::cast_to<SurfaceShape>(other.ptr());
    if (!surface || _tree_template.is_null()) return;
    if (_planted_surfaces.find(surface_id) != _planted_surfaces.end()) return;

    _plant_trees(surface);
    if (_planted_surface_id != 0) {
        _planted_surfaces.insert(surface_id);
    }
}

void ForestShape::_plant_trees(SurfaceShape* surface) {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw || _tree_template.is_null() || get_id() == 0) return;

    const Vector3 origin = surface->get_position();
    const Vector3 center_offset = get_position() - origin;
    const float center_len = center_offset.length();
    const Vector3 radial = center_len > 0.0001f ? center_offset / center_len : Vector3(0.0f, 1.0f, 0.0f);

    const Vector3 helper = Math::abs(radial.y) < 0.99f ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
    const Vector3 right = radial.cross(helper).normalized();
    const Vector3 forward = right.cross(radial).normalized();

    for (int i = 0; i < _tree_count; ++i) {
        const float angle = 2.0f * (float)Math_PI * (float)Random::randf01(_id, 2 * i);
        const float distance = _radius * Math::sqrt((float)Random::randf01(_id, 2 * i + 1));

        Vector3 p = get_position() + right * (Math::cos(angle) * distance)
                                  + forward * (Math::sin(angle) * distance);

        for (int k = 0; k < 4; ++k) {
            const float sdf = surface->evaluate_sdf(p);
            if (Math::abs(sdf) < 0.0001f) {
                break;
            }
            const Vector3 d = p - origin;
            const float len = d.length();
            const Vector3 dir = len > 0.0001f ? d / len : radial;
            p -= dir * sdf;
        }

        if (Math::abs(surface->evaluate_sdf(p)) > 0.001f) {
            continue;
        }

        Ref<Resource> copy = _tree_template->duplicate();
        Ref<SemanticShape> tree(Object::cast_to<SemanticShape>(copy.ptr()));
        if (tree.is_null()) continue;

        if (TreeShape* typed_tree = Object::cast_to<TreeShape>(tree.ptr())) {
            typed_tree->bind_planet(_planet);
        }

        tree->set_position(p);
        const uint64_t tree_id = sw->register_shape(tree);
        if (tree_id == 0) continue;

        ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
        if (mgr && _planet.is_valid() && _planet->get_id() != 0 && !mgr->get_octree(tree_id)) {
            mgr->bind_spatially(tree_id, _planet->get_id());
        }

        add_owned_shape(tree_id);
    }

    _planted_surface_id = surface->get_id();
}

void ForestShape::_clear_trees() {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    const PackedInt64Array owned = get_owned_shapes();
    if (sw) {
        for (int i = 0; i < owned.size(); ++i) {
            sw->unregister_shape_with_owned((uint64_t)owned[i]);
        }
    }
    set_owned_shapes(PackedInt64Array());
}

void ForestShape::_replant() {
    if (get_id() == 0 || _planted_surface_id == 0) return;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    Ref<SemanticShape> base = sw ? sw->get_shape(_planted_surface_id) : Ref<SemanticShape>();
    SurfaceShape* surface = Object::cast_to<SurfaceShape>(base.ptr());
    if (!surface) {
        _planted_surface_id = 0;
        _clear_trees();
        return;
    }

    _clear_trees();
    _plant_trees(surface);
    notify_shape_changed();
}
