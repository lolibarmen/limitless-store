#include "SuperDigger.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <BlockSource/BlockSource.hpp>
#include <NeochunkNode/NeochunkNode.hpp>
#include <NeochunkManager/NeochunkManager.hpp>

using namespace godot;


bool SuperDigger::can_use_on(const Dictionary &raycast_result) const {
    if (raycast_result.is_empty()) return false;
    Node *collider = Object::cast_to<Node>(raycast_result["collider"]);
    if (!collider) return false;
    // Проверяем, есть ли у объекта группа "mineable" или компонент
    return collider->is_in_group("mineable");
}

bool SuperDigger::can_use_alt_on(const Dictionary &raycast_result) const {
    if (raycast_result.is_empty()) return false;
    Node *collider = Object::cast_to<Node>(raycast_result["collider"]);
    if (!collider) return false;
    // Проверяем, есть ли у объекта группа "mineable" или компонент
    return collider->is_in_group("mineable");
}

void SuperDigger::use(const Dictionary &raycast_result) {
    float radius = 3.0f;
    float delta = 0.1f;

    Object* obj = raycast_result["collider"];
    if (!obj) return;

    NeochunkNode* ncnode = Object::cast_to<NeochunkNode>(obj);
    Ref<BlockSource> block_source = ncnode->get_block_source();
    Vector3i coord = raycast_result["position"];

    int r = (int)Math::ceil(radius);

    bool changed = false;

    std::vector<Vector3i> changed_voxels;
    for (int dx = -r; dx <= r; dx++)
    for (int dy = -r; dy <= r; dy++)
    for (int dz = -r; dz <= r; dz++) {
        float dist = Vector3(dx, dy, dz).length();
        if (dist > radius) continue;

        float t       = dist / radius;
        float falloff = 1.0f - (t * t * (3.0f - 2.0f * t));

        Vector3i voxel = coord + Vector3i(dx, dy, dz);
        float d = block_source->get_block_density(voxel);
        block_source->set_block_density(voxel, d + delta * falloff);
        changed_voxels.push_back(voxel);
        changed = true;
    }

    if (!changed) return;

    NeochunkManager* ncmanager = Object::cast_to<NeochunkManager>(ncnode->get_parent());
    ncmanager->refresh_mesh(changed_voxels);

    // UtilityFunctions::print("Super digger used on: ", ncnode->get_name());
}

void SuperDigger::use_alt(const Dictionary &raycast_result) {
    float radius = 3.0f;
    float delta = -0.1f;

    Object* obj = raycast_result["collider"];
    if (!obj) return;

    NeochunkNode* ncnode = Object::cast_to<NeochunkNode>(obj);
    Ref<BlockSource> block_source = ncnode->get_block_source();
    Vector3i coord = raycast_result["position"];

    int r = (int)Math::ceil(radius);

    bool changed = false;

    std::vector<Vector3i> changed_voxels;
    for (int dx = -r; dx <= r; dx++)
    for (int dy = -r; dy <= r; dy++)
    for (int dz = -r; dz <= r; dz++) {
        float dist = Vector3(dx, dy, dz).length();
        if (dist > radius) continue;

        float t       = dist / radius;
        float falloff = 1.0f - (t * t * (3.0f - 2.0f * t));

        Vector3i voxel = coord + Vector3i(dx, dy, dz);
        float d = block_source->get_block_density(voxel);
        block_source->set_block_density(voxel, d + delta * falloff);
        changed_voxels.push_back(voxel);
        changed = true;
    }

    if (!changed) return;

    NeochunkManager* ncmanager = Object::cast_to<NeochunkManager>(ncnode->get_parent());
    ncmanager->refresh_mesh(changed_voxels);

    // UtilityFunctions::print("Super digger used on: ", ncnode->get_name());
}