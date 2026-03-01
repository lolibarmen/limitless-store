#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <cstdint>

namespace godot {

class PlanetData : public Resource {
    GDCLASS(PlanetData, Resource)

private:
    static constexpr float MIN_NODE_SIZE = 1.0f;

    struct OctreeNode {
        Vector3 center;
        float half_size;

        float value;
        bool is_leaf;

        OctreeNode* children[8];

        OctreeNode(const Vector3& p_center, float p_half_size);
        ~OctreeNode();
    };

    OctreeNode* root;

    float get_block_recursive(const OctreeNode* node, const Vector3& pos) const;
    void set_block_recursive(OctreeNode* node, const Vector3& pos, float value);

    float get_density_recursive(
        const OctreeNode* node,
        const Vector3& pos,
        float target_size
    ) const;

    void expand_root_to_fit(const Vector3& pos);

protected:
    static void _bind_methods();

public:
    PlanetData();
    ~PlanetData();

    void initialize_default();

    void set_block(const Vector3& pos, uint8_t type);
    float get_block(const Vector3& pos) const;

    float get_density(const Vector3i& pos, int step) const;
};

} // namespace godot