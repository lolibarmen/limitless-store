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
    ClassDB::bind_method(D_METHOD("up"), &BranchShape::up);
    ClassDB::bind_method(D_METHOD("get_zenith"), &BranchShape::get_zenith);
    ClassDB::bind_method(D_METHOD("side"), &BranchShape::side);
    ClassDB::bind_method(D_METHOD("card_height"), &BranchShape::card_height);
    ClassDB::bind_method(D_METHOD("lean"), &BranchShape::lean);
    ClassDB::bind_method(D_METHOD("card_width"), &BranchShape::card_width);
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

Vector3 BranchShape::up() const {
    // Локальный верх планеты: направление от центра планеты к точке крепления.
    // Длинная ось карточки всегда следует этому вектору — зенитный угол карточки нулевой.
    if (_planet.is_valid()) {
        const Vector3 nose = _position - _planet->get_center();
        const float len = nose.length();
        if (len > 0.0001f) {
            return nose / len;
        }
    }
    return Vector3(0.0f, 1.0f, 0.0f);
}

float BranchShape::get_zenith() const {
    const float d = CLAMP(_direction.dot(up()), -1.0f, 1.0f);
    return Math::acos(d);
}

Vector3 BranchShape::side() const {
    // Ось ширины карточки: горизонтальная проекция direction (перпендикулярна up()).
    // Карточка остаётся прямоугольной и вертикальной, а наклон запекается в текстуру.
    const Vector3 u = up();
    const Vector3 w = _direction - u * _direction.dot(u);
    const float len = w.length();
    if (len > 0.0001f) {
        return w / len;
    }
    // direction параллелен up() — любая ось перпендикулярна up().
    const Vector3 helper = Math::abs(u.y) < 0.99f ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
    return u.cross(helper).normalized();
}

float BranchShape::card_height() const {
    // Проекция ветки на локальный верх: высота вертикальной карточки.
    // Минимум — чтобы карточка не вырождалась у почти горизонтальных веток.
    return Math::max(0.0001f, _length * _direction.dot(up()));
}

float BranchShape::lean() const {
    // Проекция ветки на боковую ось: боковой сдвиг кончика, запекаемый в текстуру.
    return _length * _direction.dot(side());
}

float BranchShape::card_width() const {
    return _width + Math::abs(lean());
}

void BranchShape::recompute_aabb() {
    const Vector3 s = side() * (card_width() * 0.5f);
    const Vector3 tip = _position + up() * card_height();

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
