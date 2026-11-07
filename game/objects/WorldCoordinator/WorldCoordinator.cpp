#include "WorldCoordinator.hpp"
#include <cstdint>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <SpatialStreaming/SpatialStreaming.hpp>

using namespace godot;

void WorldCoordinator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_register_initial_shapes"), &WorldCoordinator::_register_initial_shapes);
}

void WorldCoordinator::_ready() {
    SpatialStreaming* ss = SpatialStreaming::get_singleton();
    add_child(ss);

    WorldMesh* wm = WorldMesh::get_singleton();
    add_child(wm);
    
    set_process(true);
}

void WorldCoordinator::_process(double delta) {
    frame_counter++;
    
    if (frame_counter >= 10) {
        set_process(false);
        _register_initial_shapes();
    }
}

#include <PlanetShape/PlanetShape.hpp>
#include <TreeShape/TreeShape.hpp>
void WorldCoordinator::_register_initial_shapes() {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return;
    
    Ref<PlanetShape> planet;
    planet.instantiate();
    float r = 1000.0f;
    planet->set_radius(r);
    planet->set_center(Vector3(0.0, -r, 0.0));
    sw->register_shape(planet);

    const PackedInt64Array owned = planet->get_owned_shapes();
    if (owned.is_empty()) return;
    Ref<SemanticShape> surface = sw->get_shape(owned[0]);
    if (surface.is_null()) return;

    const Vector3 center = planet->get_center();
    const Vector3 dir = (Vector3(5.0f, 0.0f, 5.0f) - center).normalized();

    float lo = r - planet->get_amplitude() - 1.0f;
    float hi = r + planet->get_amplitude() + 1.0f;
    for (int i = 0; i < 32; ++i) {
        const float mid = (lo + hi) * 0.5f;
        if (surface->evaluate_sdf(center + dir * mid) < 0.0f) lo = mid;
        else hi = mid;
    }
    const Vector3 base = center + dir * ((lo + hi) * 0.5f);

    Ref<TreeShape> tree;
    tree.instantiate();
    tree->bind_planet(planet);
    tree->set_origin(base);
    tree->set_up(dir);
    tree->set_height(3.0f);
    tree->set_base_radius(0.2f);
    tree->set_top_radius(0.0f);
    tree->set_bend(Vector3(0.6f, 0.0f, 0.3f));
    tree->set_branch_count(12);
    tree->set_branch_length(2.0f);
    tree->set_branch_width(0.1f);
    tree->set_ring_variation(0.1f);
    sw->register_shape(tree);
}