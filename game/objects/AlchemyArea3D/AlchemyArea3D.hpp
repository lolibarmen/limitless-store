#pragma once

#include <godot_cpp/classes/area3d.hpp>
#include <cstdint>

namespace godot {

class AlchemyArea3D : public Area3D {
    GDCLASS(AlchemyArea3D, Area3D)

private:
    uint64_t _shape_id = 0;

    void _on_area_entered(Area3D* other);
    void _commit_alchemy(int64_t self_id, int64_t other_id);

protected:
    static void _bind_methods();

public:
    AlchemyArea3D();
    ~AlchemyArea3D() override = default;

    void set_shape_id(uint64_t id) { _shape_id = id; }
    uint64_t get_shape_id() const { return _shape_id; }
};

} // namespace godot
