#include "NeochunkNode.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

void NeochunkNode::_bind_methods() {
    // Регистрация свойств с геттерами и сеттерами для отображения в инспекторе Godot
    ClassDB::bind_method(D_METHOD("set_lod_level", "level"), &NeochunkNode::set_lod_level);
    ClassDB::bind_method(D_METHOD("get_lod_level"), &NeochunkNode::get_lod_level);
    ClassDB::add_property("NeochunkNode", PropertyInfo(Variant::INT, "lod_level"), "set_lod_level", "get_lod_level");

    ClassDB::bind_method(D_METHOD("set_chunk_size", "size"), &NeochunkNode::set_chunk_size);
    ClassDB::bind_method(D_METHOD("get_chunk_size"), &NeochunkNode::get_chunk_size);
    ClassDB::add_property("NeochunkNode", PropertyInfo(Variant::FLOAT, "chunk_size"), "set_chunk_size", "get_chunk_size");

    ClassDB::bind_method(D_METHOD("set_voxel_count", "count"), &NeochunkNode::set_voxel_count);
    ClassDB::bind_method(D_METHOD("get_voxel_count"), &NeochunkNode::get_voxel_count);
    ClassDB::add_property("NeochunkNode", PropertyInfo(Variant::FLOAT, "voxel_count"), "set_voxel_count", "get_voxel_count");

    // Привязка новой галочки "Show Bounds"
    ClassDB::bind_method(D_METHOD("set_show_bounds", "show"), &NeochunkNode::set_show_bounds);
    ClassDB::bind_method(D_METHOD("get_show_bounds"), &NeochunkNode::get_show_bounds);
    ClassDB::add_property("NeochunkNode", PropertyInfo(Variant::BOOL, "show_bounds", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_show_bounds", "get_show_bounds");
}

NeochunkNode::NeochunkNode() {}

NeochunkNode::~NeochunkNode() {}

void NeochunkNode::_ready() {
    add_to_group("mineable");
    
    _mesh_node = memnew(ChunkMeshNode);
    add_child(_mesh_node);

    _mesh_node->set_lod_level(_lod_level);
    _mesh_node->set_chunk_size(_chunk_size);
    _mesh_node->set_voxel_count(_voxel_count);

    _update_bounds_mesh();
}

void NeochunkNode::generate() {
    if (_mesh_node) {
        _mesh_node->generate_mesh();
    }
}

void NeochunkNode::set_lod_level(int level) {
    _lod_level = level;
    if (_mesh_node) _mesh_node->set_lod_level(level);
}

void NeochunkNode::set_chunk_size(float size) {
    _chunk_size = size;
    if (_mesh_node) _mesh_node->set_chunk_size(size);
    _update_bounds_mesh(); // Обновляем размер коробки при изменении размера чанка
}

void NeochunkNode::set_voxel_count(float count) {
    _voxel_count = count;
    if (_mesh_node) _mesh_node->set_voxel_count(count);
}

void NeochunkNode::set_show_bounds(bool show) {
    _show_bounds = show;
    _update_bounds_mesh();
}

void NeochunkNode::_update_bounds_mesh() {
    // 1. Создаем узел и меш, если их еще нет
    if (!_bounds_mesh_node) {
        _bounds_mesh_node = memnew(MeshInstance3D);
        add_child(_bounds_mesh_node);
        
        Ref<ImmediateMesh> immediate_mesh;
        immediate_mesh.instantiate();
        _bounds_mesh_node->set_mesh(immediate_mesh);
    }
    
    // 2. Управляем видимостью узла
    _bounds_mesh_node->set_visible(_show_bounds);
    
    // 3. Обновляем геометрию
    Ref<ImmediateMesh> immediate_mesh = _bounds_mesh_node->get_mesh();
    if (immediate_mesh.is_valid()) {
        if (_show_bounds) {
            immediate_mesh->clear_surfaces();
            
            // Создаем материал для линий
            Ref<StandardMaterial3D> material;
            material.instantiate();
            material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
            material->set_albedo(Color(0.0, 1.0, 0.0, 0.8)); // Зеленый полупрозрачный
            material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED); // Без освещения, чтобы всегда был ярким
            
            // ВАЖНО: Передаем материал прямо сюда, при создании поверхности
            immediate_mesh->surface_begin(Mesh::PRIMITIVE_LINES, material);
            
            float half_size = _chunk_size * 0.5f;
            Vector3 min(-half_size, -half_size, -half_size);
            Vector3 max(half_size, half_size, half_size);
            
            // Нижняя грань (4 линии)
            immediate_mesh->surface_add_vertex(Vector3(min.x, min.y, min.z));
            immediate_mesh->surface_add_vertex(Vector3(max.x, min.y, min.z));
            
            immediate_mesh->surface_add_vertex(Vector3(max.x, min.y, min.z));
            immediate_mesh->surface_add_vertex(Vector3(max.x, min.y, max.z));
            
            immediate_mesh->surface_add_vertex(Vector3(max.x, min.y, max.z));
            immediate_mesh->surface_add_vertex(Vector3(min.x, min.y, max.z));
            
            immediate_mesh->surface_add_vertex(Vector3(min.x, min.y, max.z));
            immediate_mesh->surface_add_vertex(Vector3(min.x, min.y, min.z));
            
            // Верхняя грань (4 линии)
            immediate_mesh->surface_add_vertex(Vector3(min.x, max.y, min.z));
            immediate_mesh->surface_add_vertex(Vector3(max.x, max.y, min.z));
            
            immediate_mesh->surface_add_vertex(Vector3(max.x, max.y, min.z));
            immediate_mesh->surface_add_vertex(Vector3(max.x, max.y, max.z));
            
            immediate_mesh->surface_add_vertex(Vector3(max.x, max.y, max.z));
            immediate_mesh->surface_add_vertex(Vector3(min.x, max.y, max.z));
            
            immediate_mesh->surface_add_vertex(Vector3(min.x, max.y, max.z));
            immediate_mesh->surface_add_vertex(Vector3(min.x, max.y, min.z));
            
            // Вертикальные ребра (4 линии)
            immediate_mesh->surface_add_vertex(Vector3(min.x, min.y, min.z));
            immediate_mesh->surface_add_vertex(Vector3(min.x, max.y, min.z));
            
            immediate_mesh->surface_add_vertex(Vector3(max.x, min.y, min.z));
            immediate_mesh->surface_add_vertex(Vector3(max.x, max.y, min.z));
            
            immediate_mesh->surface_add_vertex(Vector3(max.x, min.y, max.z));
            immediate_mesh->surface_add_vertex(Vector3(max.x, max.y, max.z));
            
            immediate_mesh->surface_add_vertex(Vector3(min.x, min.y, max.z));
            immediate_mesh->surface_add_vertex(Vector3(min.x, max.y, max.z));
            
            immediate_mesh->surface_end();
        } else {
            // Если галочка снята, очищаем поверхности, чтобы не тратить ресурсы рендера
            immediate_mesh->clear_surfaces();
        }
    }
}