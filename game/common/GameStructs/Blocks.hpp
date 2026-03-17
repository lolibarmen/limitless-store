#pragma once

enum class BlockMaterial : int {
    AIR          = 0,
    STONE        = 1,
    DIRT         = 2,
    GRASS        = 3,
    SAND         = 4,
    SNOW         = 5,
    GRAVEL       = 6,
    WATER        = 7,
};

struct BlockData {
    BlockMaterial material = BlockMaterial::AIR;
    float density = 1.0f; // 0.0 - 1.0

    bool operator==(const BlockData& o) const {
        return material == o.material && density == o.density;
    }
};