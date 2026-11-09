#include "RenderRequest.hpp"

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/shape3d.hpp>

using namespace godot;

RenderRequest::~RenderRequest() {
    drop();
}

void RenderRequest::drop() {
    active = false;

    if (collision_body) {
        collision_body->queue_free();
        collision_body = nullptr;
        collision_shape = nullptr;
    } else if (collision_shape) {
        collision_shape->queue_free();
        collision_shape = nullptr;
    }

    if (mesh_instance) {
        mesh_instance->queue_free();
        mesh_instance = nullptr;
    }

    mesh_generator = Ref<MeshGenerator>();
    material_generator = Ref<MaterialGenerator>();
}

bool RenderRequest::apply(uint64_t shape_id, uint64_t chunk_id, uint64_t p_token,
                          const Ref<ArrayMesh>& mesh,
                          const Ref<ConcavePolygonShape3D>& collision_faces) {
    if (!active || p_token == 0 || p_token != token) {
        return false;
    }

    active = false;

    const bool has_surface = mesh.is_valid() && mesh->get_surface_count() > 0;

    if (mesh_instance) {
        mesh_instance->set_mesh(mesh);
        if (has_surface && material_generator.is_valid()) {
            Ref<Material> material = material_generator->generate(shape_id, chunk_id);
            if (material.is_valid()) {
                mesh_instance->set_surface_override_material(0, material);
            }
        }
    }

    if (collision_shape) {
        if (has_surface) {
            Ref<ConcavePolygonShape3D> shape = collision_faces;
            if (shape.is_null()) {
                shape = mesh->create_trimesh_shape();
            }
            collision_shape->set_shape(shape);
        } else {
            collision_shape->set_shape(Ref<Shape3D>());
        }
    }

    mesh_generator = Ref<MeshGenerator>();
    material_generator = Ref<MaterialGenerator>();
    return true;
}
