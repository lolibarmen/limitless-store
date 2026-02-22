#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "PlanetNode/PlanetNode.hpp"
#include "TestTreeNode/TestTreeNode.hpp"
#include "ChunkNode/ChunkNode.hpp"
#include "PlanetData/PlanetData.hpp"


inline void auto_register_classes()
{
    ClassDB::register_class<PlanetNode>();
    ClassDB::register_class<TestTreeNode>();
    ClassDB::register_class<ChunkNode>();
    ClassDB::register_class<PlanetData>();
}
