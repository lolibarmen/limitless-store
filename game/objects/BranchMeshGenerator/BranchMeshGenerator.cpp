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

    MeshBuilder b = builder(shape_id, chunk_id, false);

    const Vector3 pos = shape->get_position();
    const Vector3 up = shape->up();
    const Vector3 side = shape->side();
    const float width = shape->card_width();
    const float height = shape->card_height();

    const Vector3 offset = side * width;
    const Vector3 tip = pos + up * height;
    const Vector3 normal = side.cross(up);

    auto uv_at = [&](const Vector3& world_pos) {
        const Vector3 d = world_pos - pos;
        return Vector2(d.dot(side) / width, d.dot(up) / height);
    };

    Vertex bl; bl.pos = pos - origin;             bl.normal = normal; bl.uv = uv_at(pos);
    Vertex br; br.pos = (pos + offset) - origin;  br.normal = normal; br.uv = uv_at(pos + offset);
    Vertex tr; tr.pos = (tip + offset) - origin;  tr.normal = normal; tr.uv = uv_at(tip + offset);
    Vertex tl; tl.pos = tip - origin;             tl.normal = normal; tl.uv = uv_at(tip);

    b.emit(bl, tr, br);
    b.emit(bl, tl, tr);

    submit(shape_id, chunk_id, b);
}