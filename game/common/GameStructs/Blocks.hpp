#pragma once

namespace godot {

enum BlockMaterial {
    AIR          = 0,
    STONE        = 1,
    DIRT         = 2,
    GRASS        = 3,
    SAND         = 4,
    SNOW         = 5,
    GRAVEL       = 6,
    WATER        = 7
};

constexpr auto BLOCK_MATERIAL_COUNT = std::size(BlockMaterial);

struct BlockData {
    BlockMaterial material = BlockMaterial::AIR;
    float density = 1.0f;
    // uint8_t light = 0;

    bool operator==(const BlockData& o) const {
        return material == o.material && density == o.density;
    }
};

} // namespace godot