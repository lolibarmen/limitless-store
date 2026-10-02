#include "SurfaceGenerator.hpp"
#include <Utils/VoxelBaker.hpp>
#include <ChunkOctree/ChunkOctree.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>

using namespace godot;

void SurfaceGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_lod_level", "level"), &SurfaceGenerator::set_lod_level);
    ClassDB::bind_method(D_METHOD("get_lod_level"), &SurfaceGenerator::get_lod_level);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "lod_level"), "set_lod_level", "get_lod_level");

    ClassDB::bind_method(D_METHOD("set_voxel_count", "count"), &SurfaceGenerator::set_voxel_count);
    ClassDB::bind_method(D_METHOD("get_voxel_count"), &SurfaceGenerator::get_voxel_count);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "voxel_count"), "set_voxel_count", "get_voxel_count");

    // ОБНОВЛЕНО: Добавлен аргумент chunk_key в привязку метода
    ClassDB::bind_method(D_METHOD("_on_mesh_generated", "shape_id", "chunk_key", "mesh"), &SurfaceGenerator::_on_mesh_generated);
}

Ref<ArrayMesh> SurfaceGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    uint64_t generator_id = get_instance_id();

    WorkerThreadPool::get_singleton()->add_task(
        callable_mp_static(&SurfaceGenerator::_build_mesh_task).bind(generator_id, shape_id, chunk_id),
        false,
        "SurfaceGenerator::generate"
    );

    return Ref<ArrayMesh>();
}

void SurfaceGenerator::_build_mesh_task(uint64_t generator_id, uint64_t shape_id, uint64_t chunk_id) {
    Object* obj = ObjectDB::get_instance(generator_id);
    SurfaceGenerator* gen = Object::cast_to<SurfaceGenerator>(obj);
    
    if (!gen) return;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    Ref<SemanticShape> shape = sw->get_shape(shape_id);
    if (shape.is_null()) return;

    Chunk* chunk = ChunkOctree::get_singleton()->find(chunk_id);
    if(!chunk) return;

    Vector3 chunk_pos = chunk->center;
    float chunk_size = chunk->size; 
    
    auto inp = std::make_shared<ChunkBuildInput>();
    inp->lod_level   = gen->_lod_level;
    inp->voxel_count = gen->_voxel_count;
    inp->chunk_size  = chunk_size;
    inp->stride      = (int)gen->_voxel_count + 4;
    inp->step        = 1 << gen->_lod_level;
    
    inp->chunk_coord = Vector3i(
        (int)Math::floor(chunk_pos.x - chunk_size / 2.0f),
        (int)Math::floor(chunk_pos.y - chunk_size / 2.0f),
        (int)Math::floor(chunk_pos.z - chunk_size / 2.0f)
    );

    inp->cache = std::make_shared<VoxelCache>(inp->stride, inp->step, inp->chunk_coord);

    std::vector<Ref<SemanticShape>> shapes = sw->get_shapes_snapshot();
    VoxelBaker::bake(*(inp->cache), shapes);

    const MeshData data = build_neochunk_mesh(*inp);

    if (data.vertices.is_empty()) {
        gen->call_deferred("_on_mesh_generated", shape_id, chunk_id, Ref<ArrayMesh>());
        return;
    }

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = data.vertices;
    arrays[Mesh::ARRAY_NORMAL] = data.normals;

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    gen->call_deferred("_on_mesh_generated", shape_id, chunk_id, mesh);
}

void SurfaceGenerator::_on_mesh_generated(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->complete_mesh(shape_id, chunk_id, mesh); 
    }
}