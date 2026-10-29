#include "SurfaceGenerator.hpp"
#include <Utils/VoxelBaker.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/core/math.hpp>
#include <algorithm>

using namespace godot;

#include <godot_cpp/variant/utility_functions.hpp>

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

void SurfaceGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    uint64_t generator_id = get_instance_id();

    WorkerThreadPool::get_singleton()->add_task(
        callable_mp_static(&SurfaceGenerator::_build_mesh_task).bind(generator_id, shape_id, chunk_id),
        false,
        "SurfaceGenerator::generate"
    );
}

void SurfaceGenerator::_build_mesh_task(uint64_t generator_id, uint64_t shape_id, uint64_t chunk_id) {
    Object* obj = ObjectDB::get_instance(generator_id);
    SurfaceGenerator* gen = Object::cast_to<SurfaceGenerator>(obj);
    
    if (!gen) return;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    Ref<SemanticShape> shape = sw->get_shape(shape_id);
    if (shape.is_null()) return;

    // --- НОВАЯ ЛОГИКА ПОИСКА ЧАНКА ---
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    // Получаем октодерево, которое использует эта фигура (свое или родителя)
    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return; // У фигуры нет пространственного представления (например, она листовая)

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return;
    // -----------------------------------

    Vector3 chunk_pos = chunk->center;
    float chunk_size = chunk->size; 
    
    auto inp = std::make_shared<ChunkBuildInput>();
    inp->lod_level   = gen->_lod_level;
    inp->voxel_count = gen->_voxel_count;
    inp->chunk_size  = chunk_size;

    // Мировой минимум чанка. Границы чанков выровнены по решётке voxel_count,
    // поэтому world_min — целый узел решётки и округление вниз ничего не меняет.
    const int cells = Math::max(1, (int)Math::round(gen->_voxel_count));
    const Vector3 world_min = chunk_pos - Vector3(chunk_size, chunk_size, chunk_size) * 0.5f;
    inp->chunk_coord = Vector3i(
        (int)Math::floor(world_min.x),
        (int)Math::floor(world_min.y),
        (int)Math::floor(world_min.z)
    );

    // Шаг выборки SDF = размер ячейки решётки = world_size / voxel_count.
    // Шаг теперь float: на максимальном LOD (чанк 16 м, 16 точек) он ровно 1 м,
    // а на мелких чанках (<16 м) может быть меньше 1 — раньше int-шаг округлял
    // его до 1 и меш растягивался за границы чанка.
    const float cell_size_raw = chunk_size / cells;
    inp->cell_size = cell_size_raw;
    // Мировые позиции решётки задаются через VoxelCache::step (float) = cell_size,
    // поэтому шаг должен быть строго положительным.
    inp->step = Math::max(1, (int)Math::round(inp->cell_size));
    inp->grid_cells = cells;
    inp->stride = cells + 4;

    // ВРЕМЕННАЯ ДИАГНОСТИКА: размер чанка на данной глубине и реальный шаг между точками меша.
    // world_step — фактическое расстояние в метрах между соседними точками решётки в мире:
    // VoxelBaker строит мировые позиции как base_coord + (i-2)*step, т.е. шаг = float(cell_size).
    const float world_step = inp->cell_size; // вещественный шаг, реально используемый в мире
    // Восстанавливаем размер корневого чанка октодерева по цепочке родителей.
    float root_size = chunk_size;
    for (const Chunk* n = chunk->parent; n; n = n->parent) root_size *= 2.0f;
    UtilityFunctions::print(
        "[ChunkDiag] shape_id=", shape_id,
        " octree_root_size=", root_size,
        " depth=", chunk->depth,
        " chunk_size=", chunk_size,
        " points_per_edge=", cells,
        " raw_step(cell_size)=", cell_size_raw,
        " world_step=", world_step,
        " mesh_extent=", world_step * (float)cells,
        " ratio(chunk/mesh)=", chunk_size / (world_step * (float)cells),
        " min_leaf_would_be=", root_size / (float)(1 << 8)
    );

    inp->cache = std::make_shared<VoxelCache>(inp->stride, inp->cell_size, inp->chunk_coord);

    std::vector<Ref<SemanticShape>> shapes = {shape};
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