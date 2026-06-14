// BuildBlock.hpp — тоже виртуальный базовый блок
#pragma once
#include <godot_cpp/classes/static_body3d.hpp>

namespace godot {

class BuildBlock : public StaticBody3D {
    GDCLASS(BuildBlock, StaticBody3D)

protected:
    static void _bind_methods() {}

public:
    // Найти ближайший свободный коннектор к позиции
    virtual Node3D *get_nearest_connector(const Vector3 &world_pos) const;

    // Примагнитить этот блок к коннектору другого блока
    virtual void snap_to_connector(Node3D *connector);

    virtual ~BuildBlock() {}
};

} // namespace godot