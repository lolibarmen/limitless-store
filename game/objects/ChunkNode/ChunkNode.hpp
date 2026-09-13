#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <array>
#include <vector>

namespace godot {

class ChunkNode : public Node3D {
    GDCLASS(ChunkNode, Node3D)

public:
    // Поля, перенесенные из старой структуры Neochunk
    Vector3 center;
    float size;
    int depth;
    ChunkNode* parent = nullptr;
    std::array<ChunkNode*, 8> children = {};

    ChunkNode(Vector3 c = Vector3(), float s = 0.0f, int d = 0, ChunkNode* p = nullptr);
    ~ChunkNode();

    bool is_leaf() const { return children[0] == nullptr; }

    // Курирование мешей
    std::vector<MeshInstance3D*> mesh_instances;
    void add_mesh_instance(MeshInstance3D* mesh);
    void clear_meshes();

private:
    int _lod_level = 0;
    float _chunk_size = 0.0f;
    float _voxel_count = 0.0f;
    bool _show_bounds = false;

    void _update_bounds_mesh();

protected:
    static void _bind_methods();

public:
    void _ready() override; // Пустой, как требовалось
    void generate();        // Пустой, как требовалось

    // Геттеры и сеттеры
    void set_lod_level(int level);
    int get_lod_level() const;

    void set_chunk_size(float size);
    float get_chunk_size() const;

    void set_voxel_count(float count);
    float get_voxel_count() const;

    void set_show_bounds(bool show);
    bool get_show_bounds() const;
};

} // namespace godot