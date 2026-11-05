#include "SurfaceShape.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void SurfaceShape::_bind_methods() {}

SurfaceShape::SurfaceShape() {
    _noise.instantiate();
    _noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX);
    _noise->set_fractal_octaves(2);

    _shared_voronoi.instantiate();
    _shared_voronoi->set_noise_type(FastNoiseLite::TYPE_CELLULAR);
    _shared_voronoi->set_cellular_return_type(FastNoiseLite::RETURN_CELL_VALUE);

    _mesh_generator.instantiate();
    _material_generator.instantiate();
}

void SurfaceShape::bind_planet(const Ref<PlanetShape>& planet) {
    _planet = planet;
    notify_shape_changed();
}

void SurfaceShape::notify_shape_changed() {
    _sync_from_planet();
    SemanticShape::notify_shape_changed();
}

void SurfaceShape::_sync_from_planet() {
    if (_planet.is_null()) {
        return;
    }

    _noise->set_seed(_planet->get_id());
    _shared_voronoi->set_seed(_planet->get_id());
    const float frequency = _planet->get_radius() / 100.0f;
    _noise->set_frequency(frequency);
    _shared_voronoi->set_frequency(frequency);
}

int SurfaceShape::get_material_id(const Vector3& world_pos) const {
    if (_planet.is_null()) {
        return 0;
    }

    // Используем направление от центра, как и в evaluate_sdf,
    // чтобы биомы не "плыли" по вертикали вместе с рельефом.
    const Vector3 d = world_pos - _planet->get_center();
    const float len = d.length();
    const Vector3 dir = len > 0.0f ? d / len : Vector3(0.0f, 1.0f, 0.0f);

    float val = _shared_voronoi->get_noise_3dv(dir);
    // Нормализуем значение Cellular Noise в диапазон [0, 1]
    float normalized = (val + 1.0f) * 0.5f;
    normalized = CLAMP(normalized, 0.0f, 1.0f);

    const int kMat = 2;
    return 5;
    return (int)(normalized * kMat);
}

float SurfaceShape::evaluate_sdf(const Vector3& world_pos) const {
    if (_planet.is_null()) {
        return 1e10f;
    }

    const Vector3 d = world_pos - _planet->get_center();
    // Шум берётся по направлению (нормализованный вектор), а не по полной точке:
    // иначе высота зависит от расстояния до центра и радиус «плывёт» вместе с удалением.
    const float len = d.length();
    const Vector3 dir = len > 0.0f ? d / len : Vector3(0.0f, 1.0f, 0.0f);
    const float height = _planet->get_radius() + _noise->get_noise_3dv(dir) * _planet->get_amplitude();
    return len - height;
}

void SurfaceShape::paint_vertices(const Vector3* positions, const Vector3* normals, Color* out_colors, int count, const Vector3& posoff) const {
    for (int i = 0; i < count; ++i) {
        Vector3 world_pos = positions[i] + posoff;

        int biome_id = get_material_id(world_pos);
        out_colors[i] = Color((float)biome_id / 7.0f, 0.0f, 0.0f, 1.0f);
    }
}

void SurfaceShape::recompute_aabb() {
    if (_planet.is_valid()) {
        _aabb = _planet->get_aabb();
    } else {
        _aabb = AABB();
    }
    _aabb_dirty = false;
}

void SurfaceShape::on_register() {
    _sync_from_planet();

    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (mgr && _planet.is_valid()) {
        mgr->bind_spatially(get_id(), _planet->get_id());
    }
}

void SurfaceShape::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
    }
}
