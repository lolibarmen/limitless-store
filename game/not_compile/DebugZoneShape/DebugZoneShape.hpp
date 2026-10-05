#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <AABBBoxGenerator/AABBBoxGenerator.hpp> // Путь может отличаться

namespace godot {

class DebugZoneShape : public SemanticShape {
    GDCLASS(DebugZoneShape, SemanticShape)

private:
    Ref<AABBBoxGenerator> _generator;

protected:
    static void _bind_methods();

public:
    DebugZoneShape();
    ~DebugZoneShape() override = default;

    // Реагируем на любую зону
    void on_zone_changed(const AABB& zone, int lod_level) override;
};

} // namespace godot