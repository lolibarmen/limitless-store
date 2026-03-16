#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "TestTreeNode/TestTreeNode.hpp"
#include "ChunkCollider/ChunkCollider.hpp"
#include "ChunkManager/ChunkManager.hpp"
#include "ChunkNode/ChunkNode.hpp"
#include "BlockGenerator/BlockGenerator.hpp"
#include "BiomeGenerator/BiomeGenerator.hpp"
#include "ChunkMesh/ChunkMesh.hpp"
#include "PlanetData/PlanetData.hpp"


inline void auto_register_classes()
{
    ClassDB::register_class<TestTreeNode>();
    ClassDB::register_class<ChunkCollider>();
    ClassDB::register_class<ChunkManager>();
    ClassDB::register_class<ChunkNode>();
    ClassDB::register_class<BlockGenerator>();
    ClassDB::register_class<BiomeGenerator>();
    ClassDB::register_class<ChunkMesh>();
    ClassDB::register_class<PlanetData>();
}
