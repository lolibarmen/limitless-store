#pragma once

enum class BiomeType : int {
    PLAINS   = 0,
    FOREST   = 1,
    DESERT   = 2,
    TUNDRA   = 3,
    OCEAN    = 4,
    MOUNTAIN = 5,
};

struct BiomeData {
    BiomeType type       = BiomeType::PLAINS;
    float     humidity   = 0.5f;  // 0..1
    float     temperature = 0.5f; // 0..1
    float     height     = 0.0f;  // базовая высота рельефа
};