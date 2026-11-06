#pragma once

#include <MeshGenerator/MeshGenerator.hpp>

namespace godot {

/// @brief Генератор меша ветки: один прямоугольный четырёхугольник с заданными UV.
///
/// Длинная ось карточки направлена вдоль локального верха планеты (зенит карточки
/// равен нулю); наклон ветки запекается в текстуру материалом.
/// UV следует конвенции растительности (см. assets/Grass/grass.gdshader):
/// V=0 у основания (корень), V=1 у кончика; U поперёк ветки.
/// База: (0,0), (1,0); кончик: (1,1), (0,1).
///
/// Фигура маленькая, поэтому меш НЕ режется по чанкам (slice=false):
/// ветка генерируется целиком в локальных координатах того чанка,
/// в котором она закреплена (см. BranchShape::on_zone_changed).
class BranchMeshGenerator : public MeshGenerator {
    GDCLASS(BranchMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

public:
    BranchMeshGenerator() = default;
    virtual ~BranchMeshGenerator() = default;

    void generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
