#pragma once
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector2i.hpp>

struct Vector3iHash {
    size_t operator()(const godot::Vector3i& v) const {
        size_t h = 1469598103934665603ULL;
        h ^= (int64_t)v.x; h *= 1099511628211ULL;
        h ^= (int64_t)v.y; h *= 1099511628211ULL;
        h ^= (int64_t)v.z; h *= 1099511628211ULL;
        return h;
    }
};

struct Vector2iHash {
    size_t operator()(const godot::Vector2i& v) const {
        size_t h = 1469598103934665603ULL;
        h ^= (int64_t)v.x; h *= 1099511628211ULL;
        h ^= (int64_t)v.y; h *= 1099511628211ULL;
        return h;
    }
};