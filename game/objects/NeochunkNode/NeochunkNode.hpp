#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <ChunkMeshNode/ChunkMeshNode.hpp>

namespace godot {

class NeochunkNode : public Node3D {
    GDCLASS(NeochunkNode, Node3D)

private:
    int _lod_level = 0;
    float _chunk_size = 16.0f;
    float _voxel_count = 16.0f;
    bool _show_bounds = true; // Новая галочка

    ChunkMeshNode* _mesh_node = nullptr;
    MeshInstance3D* _bounds_mesh_node = nullptr; // Узел для отрисовки коробки

protected:
    static void _bind_methods();

public:
    NeochunkNode();
    ~NeochunkNode() override;

    void _ready() override;
    void generate();

    void set_lod_level(int level);
    void set_chunk_size(float size);
    void set_voxel_count(float count);
    void set_show_bounds(bool show); // Сеттер для галочки
    
    int get_lod_level() const { return _lod_level; }
    float get_chunk_size() const { return _chunk_size; }
    float get_voxel_count() const { return _voxel_count; }
    bool get_show_bounds() const { return _show_bounds; } // Геттер для галочки

    void _update_bounds_mesh(); // Вспомогательный метод для обновления визуализации
};

} // namespace godot