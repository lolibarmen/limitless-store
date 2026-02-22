#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "PlanetNode/PlanetNode.h"
#include "TestTreeNode/TestTreeNode.h"
#include "ChunkNode/ChunkNode.h"
#include "PlanetData/PlanetData.h"


inline void auto_register_classes()
{
    ClassDB::register_class<PlanetNode>();
    ClassDB::register_class<TestTreeNode>();
    ClassDB::register_class<ChunkNode>();
    ClassDB::register_class<PlanetData>();
}
