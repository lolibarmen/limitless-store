#include "SurfaceGenerator.hpp"
#include <Utils/VoxelBaker.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

#include <godot_cpp/variant/utility_functions.hpp>

void SurfaceGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_lod_level", "level"), &SurfaceGenerator::set_lod_level);
    ClassDB::bind_method(D_METHOD("get_lod_level"), &SurfaceGenerator::get_lod_level);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "lod_level"), "set_lod_level", "get_lod_level");

    ClassDB::bind_method(D_METHOD("set_voxel_count", "count"), &SurfaceGenerator::set_voxel_count);
    ClassDB::bind_method(D_METHOD("get_voxel_count"), &SurfaceGenerator::get_voxel_count);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "voxel_count"), "set_voxel_count", "get_voxel_count");

    // ОБНОВЛЕНО: Добавлены аргументы chunk_key и collision_faces в привязку метода
    ClassDB::bind_method(D_METHOD("_on_mesh_generated", "shape_id", "chunk_key", "token", "mesh", "collision_faces"), &SurfaceGenerator::_on_mesh_generated);
}

void SurfaceGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    uint64_t generator_id = get_instance_id();

    uint64_t token = 0;
    if (WorldMesh* wm = WorldMesh::get_singleton()) {
        token = wm->current_token(shape_id, chunk_id);
    }

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return;

    const Vector3 chunk_pos = chunk->center;
    const float chunk_size = chunk->size;

    WorkerThreadPool::get_singleton()->add_task(
        callable_mp_static(&SurfaceGenerator::_build_mesh_task).bind(generator_id, shape_id, chunk_id, token, chunk_pos, chunk_size),
        false,
        "SurfaceGenerator::generate"
    );
}

void SurfaceGenerator::_build_mesh_task(uint64_t generator_id, uint64_t shape_id, uint64_t chunk_id, uint64_t token, Vector3 chunk_pos, float chunk_size) {
    Object* obj = ObjectDB::get_instance(generator_id);
    SurfaceGenerator* gen = Object::cast_to<SurfaceGenerator>(obj);

    if (!gen) return;

    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm && !wm->is_current(shape_id, chunk_id, token)) return;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return;

    Ref<SemanticShape> shape = sw->get_shape(shape_id);
    if (shape.is_null()) return;

    auto inp = std::make_shared<ChunkBuildInput>();
    inp->lod_level   = gen->_lod_level;
    inp->voxel_count = gen->_voxel_count;
    inp->chunk_size  = chunk_size;

    const int cells = Math::max(1, (int)Math::round(gen->_voxel_count));
    const Vector3 world_min = chunk_pos - Vector3(chunk_size, chunk_size, chunk_size) * 0.5f;
    inp->chunk_coord = Vector3i(
        (int)Math::floor(world_min.x),
        (int)Math::floor(world_min.y),
        (int)Math::floor(world_min.z)
    );

    const float cell_size_raw = chunk_size / cells;
    inp->cell_size = cell_size_raw;
    inp->step = inp->cell_size;
    inp->grid_cells = cells;
    inp->stride = cells + 4;
    inp->cache = std::make_shared<VoxelCache>(inp->stride, inp->cell_size, inp->chunk_coord);

    std::vector<Ref<SemanticShape>> shapes = {shape};
    VoxelBaker::bake(*(inp->cache), shapes);

    const MeshData data = build_neochunk_mesh(*inp);

    if (wm && !wm->is_current(shape_id, chunk_id, token)) return;

    if (data.vertices.is_empty()) {
        gen->call_deferred("_on_mesh_generated", shape_id, chunk_id, token, Ref<ArrayMesh>(), PackedVector3Array());
        return;
    }
    
    PackedColorArray colors;
    colors.resize(data.vertices.size());
    
    shape->paint_vertices(
        data.vertices.ptr(),
        data.normals.ptr(),
        colors.ptrw(),
        data.vertices.size(),
        chunk_pos
    );

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = data.vertices;
    arrays[Mesh::ARRAY_NORMAL] = data.normals;
    arrays[Mesh::ARRAY_COLOR] = colors;

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    gen->call_deferred("_on_mesh_generated", shape_id, chunk_id, token, mesh, data.vertices);
}

void SurfaceGenerator::_on_mesh_generated(uint64_t shape_id, uint64_t chunk_id, uint64_t token, Ref<ArrayMesh> mesh, PackedVector3Array collision_faces) {
    Ref<ConcavePolygonShape3D> collision_shape;
    if (mesh.is_valid() && !collision_faces.is_empty()) {
        collision_shape.instantiate();
        collision_shape->set_faces(collision_faces);
    }

    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->complete_mesh(shape_id, chunk_id, token, mesh, collision_shape);
    }
}