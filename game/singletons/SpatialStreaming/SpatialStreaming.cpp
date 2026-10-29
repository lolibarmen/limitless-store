#include "SpatialStreaming.hpp"

#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>

#include <vector>

using namespace godot;

SpatialStreaming* SpatialStreaming::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;

    Object* obj = engine->get_singleton("SpatialStreaming");
    return Object::cast_to<SpatialStreaming>(obj);
}

void SpatialStreaming::_bind_methods() {}
void SpatialStreaming::_ready() {}

void SpatialStreaming::_process(double delta) {
    auto* vp = get_viewport();
    if (!vp) return;

    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    player_pos = cam->get_global_position();

    auto* mgr = ShapeSpatialManager::get_singleton();
    auto* world = SemanticWorld::get_singleton();

    if (!mgr || !world) return;

    mgr->for_each_octree([this, world](uint64_t octree_id, ChunkOctree* octree) {
        (void)octree_id;

        if (!octree) {
            return;
        }

        octree->update_lod_and_notify(
            player_pos,
            [world](uint64_t shape_id, const std::vector<uint64_t>& chunk_ids) {
                world->on_zone_changed(shape_id, chunk_ids);
            }
        );
    });
}