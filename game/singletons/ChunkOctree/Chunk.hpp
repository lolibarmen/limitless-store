#pragma once
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <array>
#include <vector>
#include <memory>
#include <cstdint>
#include <unordered_map>

namespace godot {

class Chunk {
public:
    uint64_t id;
    Vector3 center;
    float size;
    int depth;
    
    std::array<std::unique_ptr<Chunk>, 8> children;
    Chunk* parent = nullptr;

    std::unordered_map<uint64_t, MeshInstance3D*> shape_meshes;
    std::unordered_map<uint64_t, Ref<MaterialGenerator>> shape_material_generators;

    Chunk(uint64_t id, Vector3 c = Vector3(), float s = 0.0f, int d = 0, Chunk* p = nullptr);
    ~Chunk();

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    bool is_leaf() const { return children[0] == nullptr; }
    
    AABB get_aabb() const;

    void remove_shape_mesh(uint64_t shape_id);

    void clear_all_shape_meshes();
};

} // namespace godot