// BuildBlock.hpp — тоже виртуальный базовый блок
#pragma once
#include <godot_cpp/classes/static_body3d.hpp>

namespace godot {

class BuildBlock : public StaticBody3D {
    GDCLASS(BuildBlock, StaticBody3D)

protected:
    static void _bind_methods() {}

public:
    virtual ~BuildBlock() {}
};

} // namespace godot