#pragma once
#include <MeshPrimitive/MeshPrimitive.hpp>

namespace godot {

class MeshCylinder : public MeshPrimitive {
    GDCLASS(MeshCylinder, MeshPrimitive)

protected:
    static void _bind_methods() {}

public:
    MeshCylinder() = default;
    ~MeshCylinder() override = default;

    // Переопределяем инициализацию
    virtual void setup_markers() override;

    // Пример API высокого уровня: изменить радиус верхнего кольца
    // Это автоматически сдвинет все маркеры группы "top" и пересчитает меш
    void set_top_radius(real_t p_new_radius);
    void set_bottom_radius(real_t p_new_radius);
};

} // namespace godot