#include "SuperDigger.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <BlockSource/BlockSource.hpp>
#include <NeochunkNode/NeochunkNode.hpp>
#include <NeochunkManager/NeochunkManager.hpp>
using namespace godot;

void SuperDigger::dig(const Dictionary &raycast_result, float radius, float delta, BlockMaterial mat = BlockMaterial::VOID) {
    Object* obj = raycast_result["collider"];
    if (!obj) return;
    NeochunkNode* ncnode = Object::cast_to<NeochunkNode>(obj);
    Ref<BlockSource> block_source = ncnode->get_block_source();
    Vector3i coord = raycast_result["position"];

    int r = (int)Math::ceil(radius);
    std::vector<Vector3i> changed_voxels;

    for (int dx = -r; dx <= r; dx++)
    for (int dy = -r; dy <= r; dy++)
    for (int dz = -r; dz <= r; dz++) {
        float dist = Vector3(dx, dy, dz).length();
        if (dist > radius) continue;

        Vector3i voxel = coord + Vector3i(dx, dy, dz);

        // Считаем градиент density (конечные разности)
        float gx = block_source->get_block_density(voxel + Vector3i(1,0,0))
                 - block_source->get_block_density(voxel - Vector3i(1,0,0));
        float gy = block_source->get_block_density(voxel + Vector3i(0,1,0))
                 - block_source->get_block_density(voxel - Vector3i(0,1,0));
        float gz = block_source->get_block_density(voxel + Vector3i(0,0,1))
                 - block_source->get_block_density(voxel - Vector3i(0,0,1));

        float grad_len = Vector3(gx, gy, gz).length();

        // Градиент близок к нулю — воксель далеко от поверхности,
        // визуального эффекта не будет, пропускаем
        if (grad_len < 0.01f) continue;

        // Чтобы сдвинуть поверхность на delta единиц в мировом пространстве,
        // нужно изменить density на delta * |grad| (обратная теорема о неявной функции)
        float influence = CLAMP(1.0f - dist / radius, 0.0f, 1.0f);
        float d = block_source->get_block_density(voxel);
        float new_d = CLAMP(d + delta * grad_len * influence, -1.0f, 1.0f);

        if (new_d == d) continue;

        block_source->set_block_density(voxel, new_d);
        // if(delta > 0) block_source->set_block_material(voxel, mat);
        changed_voxels.push_back(voxel);
    }

    if (changed_voxels.empty()) return;
    NeochunkManager* ncmanager = Object::cast_to<NeochunkManager>(ncnode->get_parent());
    ncmanager->refresh_mesh(changed_voxels);
}

bool SuperDigger::can_use_on(const Dictionary &raycast_result) const {
    if (raycast_result.is_empty()) return false;
    Node *collider = Object::cast_to<Node>(raycast_result["collider"]);
    if (!collider) return false;
    return collider->is_in_group("mineable");
}

bool SuperDigger::can_use_alt_on(const Dictionary &raycast_result) const {
    if (raycast_result.is_empty()) return false;
    Node *collider = Object::cast_to<Node>(raycast_result["collider"]);
    if (!collider) return false;
    return collider->is_in_group("mineable");
}

void SuperDigger::use(const Dictionary &raycast_result) {
    dig(raycast_result, 3.0f, 0.5f, BlockMaterial::DIRT);
}

void SuperDigger::use_alt(const Dictionary &raycast_result) {
    dig(raycast_result, 3.0f, -0.5f);
}