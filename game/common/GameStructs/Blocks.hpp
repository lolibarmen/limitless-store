#pragma once

namespace godot {

enum BlockMaterial {
    VOID         = 0,
    STONE        = 1,
    DIRT         = 2,
    GRASS        = 3,
    SAND         = 4,
    SNOW         = 5,
    GRAVEL       = 6,
    WATER        = 7,
    COUNT        = 8
};

struct BlockData {
    BlockMaterial material = BlockMaterial::VOID;
    float density = 1.0f;
    // uint8_t light = 0;

    bool operator==(const BlockData& o) const {
        return material == o.material && density == o.density;
    }
};

} // namespace godot