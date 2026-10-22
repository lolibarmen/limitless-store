#include "SpatialStreaming.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>

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

    // Обновляем LOD только для фигур, владеющих собственным октодеревом
    mgr->for_each_owned_octree([this, world](uint64_t shape_id, ChunkOctree* octree) {
        Ref<SemanticShape> shape = world->get_shape(shape_id);
        if (!shape.is_valid()) return;

        // Используем центр AABB фигуры как точку отсчета для LOD
        Vector3 shape_center = shape->get_aabb().get_center();
        
        // Делегируем логику сплита/коллапса самому дереву
        octree->evaluate_and_update_lod(player_pos, max_depth);

        // Собираем текущие листья для сравнения
        std::unordered_set<uint64_t> current_zones;
        octree->for_each_leaf([&](Chunk* chunk) {
            current_zones.insert(chunk->id);
        });

        auto& prev_zones = _prev_active_zones[shape_id];
        if (current_zones != prev_zones) {
            std::vector<uint64_t> added_zones;
            for (uint64_t id : current_zones) {
                if (prev_zones.find(id) == prev_zones.end()) {
                    added_zones.push_back(id);
                }
            }
            
            if (!added_zones.empty()) {
                world->on_zone_changed(shape_id, added_zones);
            }
            prev_zones = std::move(current_zones);
        }
    });
}