#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {

/// @brief Генератор материала ветки: рисует пиксель-арт текстуру ветки в Image.
///
/// Карточка ветки — вертикальный прямоугольник (длинная ось вдоль локального верха
/// планеты, зенит карточки нулевой). Силуэт ветки: коричневая полоса с шумом коры и
/// тёмным контуром, сужающаяся от основания (верх изображения, UV.y=0) к кончику
/// (низ, UV.y=1), прозрачный фон. Наклон ветки (зенит, BranchShape::get_zenith())
/// запекается горизонтальным сдвигом силуэта вдоль карточки.
/// Материал — StandardMaterial3D с альфа-ножницами (ALPHA_SCISSOR), ближайшей
/// фильтрацией (пиксель-арт) и отключенным отсечением задних граней (карточка
/// видна с обеих сторон).
class BranchMaterialGenerator : public MaterialGenerator {
    GDCLASS(BranchMaterialGenerator, MaterialGenerator)

protected:
    static void _bind_methods();

public:
    BranchMaterialGenerator() = default;
    virtual ~BranchMaterialGenerator() = default;

    Ref<Material> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
