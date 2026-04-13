#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "TestTreeNode/TestTreeNode.hpp"
#include "ChunkCollider/ChunkCollider.hpp"
#include "ChunkManager/ChunkManager.hpp"
#include "ChunkNode/ChunkNode.hpp"
#include "WorldCoordinator/WorldCoordinator.hpp"
#include "NeochunkNode/NeochunkNode.hpp"
#include "NeochunkManager/NeochunkManager.hpp"
#include "BlockLODSource/BlockLODSource.hpp"
#include "BlockGenerator/BlockGenerator.hpp"
#include "ChunkMeshGenerator/ChunkMeshGenerator.hpp"
#include "BiomeSource/BiomeSource.hpp"
#include "BiomeGenerator/BiomeGenerator.hpp"
#include "BlockSource/BlockSource.hpp"
#include "PlanetData/PlanetData.hpp"


inline void auto_register_classes()
{
    ClassDB::register_class<TestTreeNode>();
    ClassDB::register_class<ChunkCollider>();
    ClassDB::register_class<ChunkManager>();
    ClassDB::register_class<ChunkNode>();
    ClassDB::register_class<WorldCoordinator>();
    ClassDB::register_class<NeochunkNode>();
    ClassDB::register_class<NeochunkManager>();
    ClassDB::register_class<BlockLODSource>();
    ClassDB::register_class<BlockGenerator>();
    ClassDB::register_class<ChunkMeshGenerator>();
    ClassDB::register_class<BiomeSource>();
    ClassDB::register_class<BiomeGenerator>();
    ClassDB::register_class<BlockSource>();
    ClassDB::register_class<PlanetData>();
}
