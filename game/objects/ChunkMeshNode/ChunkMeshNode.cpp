#include "SurfaceNets.hpp"
#include "ChunkMeshNode.hpp"
#include <Utils/VoxelBaker.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <ChunkMeshNode/ChunkMeshQueue.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <cstdio>

using namespace godot;

void ChunkMeshNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &ChunkMeshNode::set_mesh);
    ClassDB::bind_method(D_METHOD("generate_mesh"), &ChunkMeshNode::generate_mesh);
}

ChunkMeshNode::ChunkMeshNode() {}
ChunkMeshNode::~ChunkMeshNode() {
    // !!! НЕ ВЫЗЫВАЙ get_instance_id() здесь !!!
    // Это приводит к крашу, потому что объект уже в процессе удаления
    
    // Если нужна диагностика, используй только безопасные проверки:
    if (_mesh_instance) {
        UtilityFunctions::print("!!! ChunkMeshNode DESTRUCTOR: _mesh_instance exists, cleaning up...");
        // НЕ вызывай _mesh_instance->get_instance_id() тоже!
        _mesh_instance = nullptr; // Просто обнуляем указатель
    }
    
    if (_collision_shape) {
        _collision_shape = nullptr;
    }
    
    UtilityFunctions::print("!!! ChunkMeshNode DESTRUCTOR: Cleanup complete");
}

void ChunkMeshNode::_ready() {
    _mesh_instance = nullptr;
    _collision_shape = nullptr;

    generate_mesh();
}

void ChunkMeshNode::generate_mesh() {
    Vector3 chunk_pos = get_global_position();
    float half_chunk_size = _chunk_size / 2.0f;

    auto inp = std::make_shared<ChunkBuildInput>();
    inp->lod_level    = _lod_level;
    inp->voxel_count  = _voxel_count;
    inp->chunk_size   = _chunk_size;
    inp->stride       = (int)_voxel_count + 4;
    inp->step         = 1 << _lod_level;
    inp->chunk_coord  = Vector3i(
        (int)(chunk_pos.x - half_chunk_size),
        (int)(chunk_pos.y - half_chunk_size),
        (int)(chunk_pos.z - half_chunk_size)
    );

    _task.input   = inp;
    _task.task_id = WorkerThreadPool::get_singleton()->add_task(
        callable_mp_static(&ChunkMeshNode::_build_mesh_task).bind(get_instance_id()),
        false,
        "ChunkMeshNode::generate_mesh"
    );
}

void ChunkMeshNode::_build_mesh_task(uint64_t node_id) {
    Object* obj = ObjectDB::get_instance(node_id);
    ChunkMeshNode* node = Object::cast_to<ChunkMeshNode>(obj);
    if (!node) return;

    auto task = node->_task;
    auto inp = task.input;

    if (!inp) {
        node->call_deferred("set_mesh", Ref<Mesh>());
        return;
    }

    inp->cache = std::make_shared<VoxelCache>(inp->stride, inp->step, inp->chunk_coord);
    
    SemanticWorld* sw = (SemanticWorld*)Engine::get_singleton()->get_singleton("SemanticWorld");
    if (sw) {
        std::vector<Ref<SemanticShape>> shapes = sw->get_shapes_snapshot();
        VoxelBaker::bake(*(inp->cache), shapes);
    }

    const MeshData data = build_neochunk_mesh(*inp);

    if (data.vertices.is_empty()) {
        node->call_deferred("set_mesh", Ref<Mesh>());
        return;
    }

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = data.vertices;
    arrays[Mesh::ARRAY_NORMAL] = data.normals;
    arrays[Mesh::ARRAY_COLOR]  = data.colors;

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    ChunkMeshQueue::get_singleton().push(node_id, mesh);
}

#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>

void ChunkMeshNode::set_mesh(Ref<Mesh> mesh) {
    print_line(">>> set_mesh call! Mesh is valid: ", mesh.is_valid() ? "True" : "False");
    
    if (is_queued_for_deletion()) {
        UtilityFunctions::print(">>> set_mesh IGNORED: Node is already queued for deletion (ID: ", get_instance_id(), ")");
        return;
    }

    if (!mesh.is_valid()) {
        print_line("Mesh is invalid, skipping");
        return;
    }
    
    if (!_mesh_instance) {
        print_line("Creating TestMeshInstance3D...");
        _mesh_instance = memnew(MeshInstance3D);
        print_line("TestMeshInstance3D created, ID: ", _mesh_instance->get_instance_id());
        
        add_child(_mesh_instance);
        print_line("Added to tree, children count: ", get_child_count());
    }
    
    print_line("Calling set_mesh on TestMeshInstance3D...");
    _mesh_instance->set_mesh(mesh);
    
    print_line("After set_mesh: _mesh_instance->get_mesh() is valid: " + String(_mesh_instance->get_mesh().is_valid() ? "True" : "False"));
    print_line("After set_mesh: mesh->get_surface_count(): " + itos(mesh->get_surface_count()));
    
    // Создаем гигантский маяк для теста
    Ref<BoxMesh> box;
    box.instantiate();
    box->set_size(Vector3(100.0, 100.0, 100.0));
    
    _mesh_instance->set_mesh(box);
    
    print_line("After setting box: _mesh_instance->get_mesh() is valid: " + String(_mesh_instance->get_mesh().is_valid() ? "True" : "False"));
    print_line("After setting box: box->get_surface_count(): " + itos(box->get_surface_count()));
    
    _mesh_instance->set_global_transform(Transform3D());
    
    Ref<StandardMaterial3D> beacon_mat;
    beacon_mat.instantiate();
    beacon_mat->set_albedo(Color(1, 0, 0));
    beacon_mat->set_emission(Color(10, 0, 0));
    beacon_mat->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
    beacon_mat->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
    _mesh_instance->set_surface_override_material(0, beacon_mat);
    
    print_line("!!! LIGHTHOUSE MADE COMPLETE !!!");
    print_line("Final check: _mesh_instance->get_mesh() is valid: " + String(_mesh_instance->get_mesh().is_valid() ? "True" : "False"));
}

    // ============================================================
    // ЭКСПОРТ В OBJ ДЛЯ BLENDER (Теперь сохраняет реальный mesh)
    // Если этот дебаг больше не нужен, весь этот блок можно удалить.
    // ============================================================
    // if (mesh->get_surface_count() > 0) {
    //     FILE* f = fopen("/tmp/chunk_debug.obj", "w");
    //     if (f) {
    //         fprintf(f, "# Chunk Mesh Export\n");
    //         fprintf(f, "# Chunk pos: %f %f %f\n", gp.x, gp.y, gp.z);
            
    //         // Берем данные первой поверхности (обычно чанки состоят из одной поверхности)
    //         godot::Array arrays = mesh->surface_get_arrays(0);
    //         godot::PackedVector3Array vertices = arrays[godot::Mesh::ARRAY_VERTEX];
    //         godot::PackedInt32Array indices = arrays[godot::Mesh::ARRAY_INDEX];

    //         // 1. Записываем вершины (v x y z)
    //         for (int i = 0; i < vertices.size(); i++) {
    //             godot::Vector3 v = vertices[i];
    //             fprintf(f, "v %f %f %f\n", v.x, v.y, v.z);
    //         }

    //         // 2. Записываем грани (f v1 v2 v3)
    //         if (indices.size() > 0) {
    //             for (int i = 0; i < indices.size(); i += 3) {
    //                 fprintf(f, "f %d %d %d\n", 
    //                     indices[i] + 1, 
    //                     indices[i + 1] + 1, 
    //                     indices[i + 2] + 1);
    //             }
    //         } else {
    //             for (int i = 0; i < vertices.size(); i += 3) {
    //                 fprintf(f, "f %d %d %d\n", i + 1, i + 2, i + 3);
    //             }
    //         }
            
    //         fclose(f);
    //         printf("[DEBUG] Real mesh successfully saved to /tmp/chunk_debug.obj\n");
    //     } else {
    //         printf("[ERROR] Failed to open /tmp/chunk_debug.obj for writing\n");
    //     }
    // }
    // ============================================================
// }