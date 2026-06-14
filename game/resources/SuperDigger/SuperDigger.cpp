#include "SuperDigger.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <BlockSource/BlockSource.hpp>
#include <NeochunkNode/NeochunkNode.hpp>
#include <NeochunkManager/NeochunkManager.hpp>
using namespace godot;

void SuperDigger::dig(const Dictionary &raycast_result, float radius, float delta) {
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
        float d = block_source->get_block_density(voxel);

        float surface_threshold = 0.3f;
        if (d > surface_threshold && delta > 0) continue;
        if (d < -surface_threshold && delta < 0) continue;

        float t = dist / radius;
        float edge_falloff = (t < 0.7f) ? 1.0f : (1.0f - t) / 0.3f;

        float new_d = d + delta * edge_falloff;
        if (delta > 0) new_d = Math::min(new_d,  surface_threshold + delta);
        else           new_d = Math::max(new_d, -surface_threshold + delta);

        block_source->set_block_density(voxel, CLAMP(new_d, -1.0f, 1.0f));
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
    dig(raycast_result, 1.5f, 0.08f);
}

void SuperDigger::use_alt(const Dictionary &raycast_result) {
    dig(raycast_result, 3.0f, -0.08f);
}