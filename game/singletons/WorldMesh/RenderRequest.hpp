#pragma once

#include <MeshGenerator/MeshGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <cstdint>

namespace godot {

enum MeshTarget : uint32_t {
    MESH_TARGET_VISUAL = 1u << 0,
    MESH_TARGET_COLLISION = 1u << 1,
    MESH_TARGET_AREA = 1u << 2,
};

class RenderRequest {
public:
    uint64_t token = 0;
    Vector3 shape_origin;
    MeshInstance3D* mesh_instance = nullptr;
    StaticBody3D* collision_body = nullptr;
    CollisionShape3D* collision_shape = nullptr;
    Area3D* area = nullptr;
    CollisionShape3D* area_shape = nullptr;
    Ref<MaterialGenerator> material_generator;
    bool active = true;
    uint32_t target_mask = 0;
    uint32_t applied_targets = 0;

    RenderRequest() = default;
    ~RenderRequest();

    void drop();

    bool apply(uint64_t shape_id, uint64_t chunk_id, uint64_t p_token,
               const Ref<ArrayMesh>& mesh, uint32_t targets);
};

} // namespace godot
