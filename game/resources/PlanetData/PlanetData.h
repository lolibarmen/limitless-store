#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <vector>
#include <cstdint>

namespace godot {

class PlanetData : public Resource {
    GDCLASS(PlanetData, Resource)

private:
    std::vector<float> blocks_data;
    Vector3i world_size;

    int to_index(const Vector3i& pos) const;
    Vector3i get_offset() const;

protected:
    static void _bind_methods();

public:
    PlanetData();

    void initialize_default();
    void set_block(const Vector3i& pos, uint8_t type);
    float get_block(const Vector3i& pos) const;
    bool is_valid_position(const Vector3i& pos) const;

    void set_world_size(const Vector3i& p_size);
    Vector3i get_world_size() const;
};

} // namespace godot