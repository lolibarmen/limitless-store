#pragma once
#include <MeshPrimitive/MeshPrimitive.hpp>

namespace godot {

// Y-образное соединение: один вход снизу, два выхода сверху.
// base_mesh — меш, имеющий хотя бы 3 кольца вершин по высоте
// (низ, середина, верх), чтобы деформация маркерами могла
// "развести" верхнюю часть в две ветки.
//
// Группы маркеров:
//   "base"     — нижнее кольцо (вход трубы), остаётся неподвижным.
//   "junction" — среднее кольцо, точка начала разветвления.
//   "left"     — верхнее левое кольцо (левая ветка).
//   "right"    — верхнее правое кольцо (правая ветка).
class MeshYJoint : public MeshPrimitive {
    GDCLASS(MeshYJoint, MeshPrimitive)

protected:
    static void _bind_methods();

public:
    MeshYJoint() = default;
    ~MeshYJoint() override = default;

    virtual void setup_markers() override;

    // --- API высокого уровня ---
    void set_base_radius(real_t p_radius);
    void set_junction_radius(real_t p_radius);
    void set_left_radius(real_t p_radius);
    void set_right_radius(real_t p_radius);

    // Изменить угол разведения веток (1.0 = дефолт, >1 = шире, <1 = уже)
    void set_spread(real_t p_factor);

    // Сдвинуть точку разветвления вверх/вниз (0 = низ, 1 = верх)
    void set_junction_height(real_t p_normalized);
};

} // namespace godot