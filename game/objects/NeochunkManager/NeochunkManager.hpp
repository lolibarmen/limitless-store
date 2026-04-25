#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <NeochunkNode/NeochunkNode.hpp>
#include <BlockSource/BlockSource.hpp>
#include <memory>
#include <unordered_map>

namespace godot {

constexpr int   MAX_DEPTH = 2;
constexpr float ROOT_SIZE = 64.0f;

class NeochunkManager : public Node3D {
    GDCLASS(NeochunkManager, Node3D)

    Ref<BlockSource> block_source;

    struct Neochunk {
        Vector3 center;
        float   size;
        int     depth;
        Neochunk* parent = nullptr;
        std::array<Neochunk*, 8> children = {};
        NeochunkNode* node = nullptr;

        Neochunk(Vector3 c, float s, int d, Neochunk* p = nullptr)
        : center(c), size(s), depth(d), parent(p) {}

        ~Neochunk() {
            if (node) {
                node->queue_free();
                node = nullptr;
            }
            for (auto& c : children) {
                delete c;
                c = nullptr;
            }
        }
        bool is_leaf() const { return children[0] == nullptr; }
    };

    struct V3iHash {
        size_t operator()(const Vector3i& v) const {
            return std::hash<int>()(v.x) ^ std::hash<int>()(v.y)<<8 ^ std::hash<int>()(v.z)<<16;
        }
    };

    int root_radius = 3;
    std::unordered_map<Vector3i, Neochunk*, V3iHash> roots;
    Vector3 player_pos = {};

    void update_roots();

    void update_recurs(Neochunk* n);
    void delete_childrens(Neochunk* n);
    
    void spawn_mesh(Neochunk* n);
    void despawn_mesh(Neochunk* n);

protected:
    static void _bind_methods() {}

public:
    void _ready() override;
    void _process(double delta) override;
    void set_block_source(Ref<BlockSource> bs) { block_source = bs; }
    void refresh_mesh(NeochunkNode* node);
};

} // namespace godot