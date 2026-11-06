#include "BranchShape.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void BranchShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_position", "position"), &BranchShape::set_position);
    ClassDB::bind_method(D_METHOD("get_position"), &BranchShape::get_position);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

    ClassDB::bind_method(D_METHOD("set_direction", "direction"), &BranchShape::set_direction);
    ClassDB::bind_method(D_METHOD("get_direction"), &BranchShape::get_direction);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "direction"), "set_direction", "get_direction");

    ClassDB::bind_method(D_METHOD("set_length", "length"), &BranchShape::set_length);
    ClassDB::bind_method(D_METHOD("get_length"), &BranchShape::get_length);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length", PROPERTY_HINT_RANGE, "0.01,50.0,0.01"), "set_length", "get_length");

    ClassDB::bind_method(D_METHOD("set_width", "width"), &BranchShape::set_width);
    ClassDB::bind_method(D_METHOD("get_width"), &BranchShape::get_width);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width", PROPERTY_HINT_RANGE, "0.01,10.0,0.01"), "set_width", "get_width");

    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &BranchShape::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &BranchShape::get_seed);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");

    ClassDB::bind_method(D_METHOD("bind_planet", "planet"), &BranchShape::bind_planet);
    ClassDB::bind_method(D_METHOD("side"), &BranchShape::side);
}

BranchShape::BranchShape() {
    _mesh_generator.instantiate();
    _material_generator.instantiate();
}

void BranchShape::bind_planet(const Ref<PlanetShape>& planet) {
    _planet = planet;
    notify_shape_changed();
}

void BranchShape::set_direction(const Vector3& v) {
    const float len = v.length();
    _direction = len > 0.0001f ? v / len : Vector3(0.0f, 1.0f, 0.0f);
    notify_shape_changed();
}

Vector3 BranchShape::side() const {
    // Ось ширины карточки: компонента мирового up, перпендикулярная direction.
    // Тогда quad лежит в вертикальной плоскости (содержит Y), его нормаль
    // перпендикулярна Y. Вектор остаётся перпендикулярным direction — quad прямоугольный.
    const Vector3 w = Vector3(0.0f, 1.0f, 0.0f) - _direction * _direction.y;
    const float len = w.length();
    if (len > 0.0001f) {
        return w / len;
    }
    // direction параллелен up — любая горизонтальная ось образует вертикальную плоскость.
    return Vector3(1.0f, 0.0f, 0.0f);
}

void BranchShape::recompute_aabb() {
    const Vector3 s = side() * (_width * 0.5f);
    const Vector3 tip = _position + _direction * _length;

    const Vector3 corners[4] = {
        _position - s, _position + s, tip - s, tip + s
    };

    Vector3 mn = corners[0], mx = corners[0];
    for (int i = 1; i < 4; ++i) {
        mn = Vector3(Math::min(mn.x, corners[i].x), Math::min(mn.y, corners[i].y), Math::min(mn.z, corners[i].z));
        mx = Vector3(Math::max(mx.x, corners[i].x), Math::max(mx.y, corners[i].y), Math::max(mx.z, corners[i].z));
    }

    const Vector3 margin(0.01f, 0.01f, 0.01f);
    _aabb = AABB(mn - margin, (mx - mn) + margin * 2.0f);
    _aabb_dirty = false;
}

void BranchShape::on_register() {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (mgr && _planet.is_valid()) {
        // Своего октодерева нет — ветка живёт в пространственном дереве планеты.
        mgr->bind_spatially(get_id(), _planet->get_id());
    }
}

void BranchShape::on_zone_changed(uint64_t chunk_id) {
    // Подавление дублей (см. SemanticShape::on_zone_changed): рендерим только в чанке,
    // содержащем центр AABB фигуры. Меш не режется — ветка целиком попадает в чанк.
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    Chunk* chunk = mgr ? mgr->get_chunk(get_id(), chunk_id) : nullptr;
    if (!chunk) return;
    if (!chunk->get_aabb().has_point(get_aabb().get_center())) return;

    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
    }
}
