#include "RenderRequest.hpp"
#include <AlchemyArea3D/AlchemyArea3D.hpp>

#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/convex_polygon_shape3d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh.hpp>

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

    if (area) {
        area->set_monitoring(false);
        area->queue_free();
        area = nullptr;
        area_shape = nullptr;
    } else if (area_shape) {
        area_shape->queue_free();
        area_shape = nullptr;
    }

    if (mesh_instance) {
        mesh_instance->queue_free();
        mesh_instance = nullptr;
    }

    material_generator = Ref<MaterialGenerator>();
    target_mask = 0;
    applied_targets = 0;
}

bool RenderRequest::apply(uint64_t shape_id, uint64_t chunk_id, uint64_t p_token,
                          const Ref<ArrayMesh>& mesh, uint32_t targets) {
    if (!active || p_token == 0 || p_token != token) {
        return false;
    }

    targets &= ~applied_targets;
    if (targets == 0) {
        return false;
    }
    applied_targets |= targets;

    const bool has_surface = mesh.is_valid() && mesh->get_surface_count() > 0;

    if (mesh_instance && (targets & MESH_TARGET_VISUAL)) {
        mesh_instance->set_position(shape_origin);
        mesh_instance->set_mesh(mesh);
        if (has_surface && material_generator.is_valid()) {
            Ref<Material> material = material_generator->generate(shape_id, chunk_id);
            if (material.is_valid()) {
                mesh_instance->set_surface_override_material(0, material);
            }
        }
        mesh_instance->set_visible(has_surface);
        material_generator = Ref<MaterialGenerator>();
    }

    if (collision_shape && (targets & MESH_TARGET_COLLISION)) {
        if (collision_body) {
            collision_body->set_position(shape_origin);
        }
        if (has_surface) {
            Ref<ConcavePolygonShape3D> shape = mesh->create_trimesh_shape();
            collision_shape->set_shape(shape);
            collision_shape->set_disabled(shape.is_null());
        } else {
            collision_shape->set_shape(Ref<Shape3D>());
            collision_shape->set_disabled(true);
        }
    }

    if (area_shape && (targets & MESH_TARGET_AREA)) {
        if (area) {
            area->set_position(shape_origin);
        }
        if (has_surface) {
            Ref<ConvexPolygonShape3D> shape = mesh->create_convex_shape(true, false);
            area_shape->set_shape(shape);
            area_shape->set_disabled(shape.is_null());
        } else {
            area_shape->set_shape(Ref<Shape3D>());
            area_shape->set_disabled(true);
        }
    }

    return true;
}
