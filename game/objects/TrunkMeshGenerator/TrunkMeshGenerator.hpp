#pragma once

#include <MeshGenerator/MeshGenerator.hpp>

namespace godot {

/// @brief Генератор меша ствола: низкополигональный конус из колец.
///
/// Форма читается из TrunkShape (параметры не хранятся в полях генератора):
/// центральная линия с боковым изгибом, конусность, вариация колец —
/// детерминированно из seed через Utils/RandomUtils.hpp.
///
/// Фигура маленькая, поэтому меш НЕ режется по чанкам (slice=false):
/// ствол генерируется целиком в локальных координатах того чанка,
/// в котором он закреплён (см. TrunkShape::on_zone_changed).
class TrunkMeshGenerator : public MeshGenerator {
    GDCLASS(TrunkMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

public:
    TrunkMeshGenerator() = default;
    virtual ~TrunkMeshGenerator() = default;

    void generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
