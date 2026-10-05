#include "MaterialTerrainShape.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <ChunkOctree/ChunkOctree.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

MaterialTerrainShape::MaterialTerrainShape() {
    _noise.instantiate();
    _noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX);
    _noise->set_frequency(_frequency);
    _noise->set_seed(_seed);

    _generator_lod0.instantiate();
    _generator_lod0->set_lod_level(0);
    
    _generator_lod1.instantiate();
    _generator_lod1->set_lod_level(1);
    
    _generator_lod2.instantiate();
    _generator_lod2->set_lod_level(3);
}

void MaterialTerrainShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_material_type", "type"), &MaterialTerrainShape::set_material_type);
    ClassDB::bind_method(D_METHOD("get_material_type"), &MaterialTerrainShape::get_material_type);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "material_type"), "set_material_type", "get_material_type");

    ClassDB::bind_method(D_METHOD("set_base_height", "height"), &MaterialTerrainShape::set_base_height);
    ClassDB::bind_method(D_METHOD("get_base_height"), &MaterialTerrainShape::get_base_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_height"), "set_base_height", "get_base_height");

    ClassDB::bind_method(D_METHOD("set_amplitude", "amplitude"), &MaterialTerrainShape::set_amplitude);
    ClassDB::bind_method(D_METHOD("get_amplitude"), &MaterialTerrainShape::get_amplitude);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");

    ClassDB::bind_method(D_METHOD("set_frequency", "frequency"), &MaterialTerrainShape::set_frequency);
    ClassDB::bind_method(D_METHOD("get_frequency"), &MaterialTerrainShape::get_frequency);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frequency"), "set_frequency", "get_frequency");

    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &MaterialTerrainShape::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &MaterialTerrainShape::get_seed);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
}

float MaterialTerrainShape::evaluate_sdf(const Vector3& world_pos) const {
    if (!_noise.is_valid()) {
        return world_pos.y - _base_height;
    }

    float noise_val = _noise->get_noise_2d(world_pos.x, world_pos.z);
    
    float terrain_height = _base_height + (noise_val * _amplitude);
    
    return world_pos.y - terrain_height;
}

Ref<MeshGenerator> MaterialTerrainShape::get_mesh_generator() const {
    return _generator_lod0;
}

void MaterialTerrainShape::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) return;

    Chunk* chunk = ChunkOctree::get_singleton()->find(chunk_id);
    if (!chunk || !chunk->is_leaf()) return;

    Ref<MeshGenerator> active_generator;
    // Чем больше depth, тем меньше размер чанка и тем выше детализация ему нужна
    switch (chunk->depth) {
        case 2: active_generator = _generator_lod0; break;
        case 1: active_generator = _generator_lod1; break;
        case 0: active_generator = _generator_lod2; break;
        default: return; // Игнорируем слишком мелкие или крупные чанки, если не настроены
    }

    if (active_generator.is_null()) return;

    wm->request_render(get_id(), chunk_id, active_generator);
}

void MaterialTerrainShape::recompute_aabb() {
    float max_h = _base_height + _amplitude;
    float min_h = _base_height - _amplitude;
    float size_y = (max_h + 10.0f) - (min_h - 10.0f); 
    
    _aabb.position = Vector3(-2000.0f, min_h - 10.0f, -2000.0f);
    _aabb.size = Vector3(4000.0f, size_y, 4000.0f);
    
    _aabb_dirty = false;
}

} // namespace godot