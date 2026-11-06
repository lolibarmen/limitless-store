#include "BranchMeshGenerator.hpp"
#include <BranchShape/BranchShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void BranchMeshGenerator::_bind_methods() {}

void BranchMeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    auto* sw = SemanticWorld::get_singleton();
    if (!sw) return;
    Ref<SemanticShape> base = sw->get_shape(shape_id);
    BranchShape* shape = Object::cast_to<BranchShape>(base.ptr());
    if (!shape) return;

    Vector3 origin;
    if (!get_chunk_origin(shape_id, chunk_id, origin)) return;

    // Маленькая фигура: фильтр по боксу чанка выключен, меш эмитится целиком.
    MeshBuilder b = builder(shape_id, chunk_id, false);

    const Vector3 pos = shape->get_position();
    const Vector3 dir = shape->get_direction();
    const Vector3 side = shape->side();
    const Vector3 s = side * (shape->get_width() * 0.5f);
    const Vector3 tip = pos + dir * shape->get_length();
    const Vector3 normal = side.cross(dir);

    // UV считаются по позиции вершины: U — проекция на боковую ось (0..1 поперёк),
    // V — проекция на направление ветки (0..1 вдоль). Масштаб V под квадратный
    // тексель задаёт материал (uv1_scale.y).
    const float width = shape->get_width();
    const float length = shape->get_length();
    auto uv_at = [&](const Vector3& world_pos) {
        const Vector3 d = world_pos - pos;
        return Vector2(0.5f + d.dot(side) / width, d.dot(dir) / length);
    };

    // Четырёхугольник карточки ветки: V=0 основание, V=1 кончик; U поперёк ветки.
    Vertex bl; bl.pos = (pos - s) - origin;         bl.normal = normal; bl.uv = uv_at(pos - s);
    Vertex br; br.pos = (pos + s) - origin;         br.normal = normal; br.uv = uv_at(pos + s);
    Vertex tr; tr.pos = (tip + s) - origin;         tr.normal = normal; tr.uv = uv_at(tip + s);
    Vertex tl; tl.pos = (tip - s) - origin;         tl.normal = normal; tl.uv = uv_at(tip - s);

    // Godot: лицевая грань — по часовой стрелке, порядок вершин такой,
    // чтобы (v1-v0)x(v2-v0) был против normal (как в TrunkMeshGenerator).
    b.emit(bl, tr, br);
    b.emit(bl, tl, tr);

    submit(shape_id, chunk_id, b);
}
