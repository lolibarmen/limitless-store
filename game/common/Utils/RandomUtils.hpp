#pragma once

namespace godot {
namespace Random {

/// Детерминированный 64-битный хеш (splitmix64-финализер).
inline uint64_t hash(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

/// Вещественное [0,1) по (seed, канал). Каналы разных параметров не пересекаются.
inline real_t randf01(uint64_t seed, int channel) {
    uint64_t h = hash(seed * 0x100000001ULL + (uint64_t)channel * 0x9e3779b1ULL);
    return (real_t)(h & 0xFFFFFFu) / (real_t)0x1000000u;
}

/// Целое [0,n) по (seed, канал). n<=0 -> 0.
inline int    rand_int(uint64_t seed, int channel, int n) {
    if (n <= 0) return 0;
    uint64_t h = hash(seed * 0x100000001ULL + (uint64_t)channel * 0x9e3779b1ULL + 0x51ULL);
    return (int)(h % (uint64_t)n);
}

} // namespace Random
} // namespace godot