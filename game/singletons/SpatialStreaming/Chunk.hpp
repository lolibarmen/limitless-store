#pragma once
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <array>
#include <vector>
#include <memory>

namespace godot {

// Forward declaration
class Chunk;

class Chunk {
public:
    Vector3 center;
    float size;
    int depth;
    
    // Владелец узла в дереве. Используем unique_ptr для автоматического удаления детей при удалении родителя
    std::array<std::unique_ptr<Chunk>, 8> children;
    
    // Родитель нужен только если мы ходим вверх по дереву (опционально)
    Chunk* parent = nullptr;

    // Хранилище для визуальных представлений (MeshInstance3D)
    // Они будут добавляться в сцену через WorldMesh/SpatialStreaming
    std::vector<MeshInstance3D*> mesh_instances;

    Chunk(Vector3 c = Vector3(), float s = 0.0f, int d = 0, Chunk* p = nullptr);
    ~Chunk();

    // Запрещаем копирование, так как у нас есть unique_ptr
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    bool is_leaf() const { return children[0] == nullptr; }

    void add_mesh_instance(MeshInstance3D* mesh);
    void clear_meshes();
    
    // Вспомогательный метод для получения AABB
    AABB get_aabb() const;
};

} // namespace godot