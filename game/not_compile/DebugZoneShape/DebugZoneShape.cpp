#include "DebugZoneShape.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void DebugZoneShape::_bind_methods() {}

DebugZoneShape::DebugZoneShape() {
    // Создаем генератор
    _generator.instantiate();

    // Задаем гигантский AABB, чтобы эта фигура пересекалась с ЛЮБОЙ зоной стриминга.
    // Благодаря этому SemanticWorld всегда будет вызывать у неё on_zone_changed.
    _aabb = AABB(Vector3(-100000, -100000, -100000), Vector3(200000, 200000, 200000));
    _aabb_dirty = false; // Говорим системе, что AABB уже посчитан и не требует пересчета
}

Ref<MeshGenerator> DebugZoneShape::get_mesh_generator() const {
    return _generator;
}

void DebugZoneShape::on_zone_changed(const AABB& zone, int lod_level) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) return;

    // Если зона активна (LOD валиден), запрашиваем отрисовку бокса в её границах
    if (lod_level >= 0) {
        wm->request_render(get_id(), zone);
    } 
    // Если зона выгружена, убираем бокс
    else {
        wm->cancel_render(get_id(), zone);
    }
}