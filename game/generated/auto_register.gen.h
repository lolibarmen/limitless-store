#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "PlayerSimple/PlayerSimple.hpp"
#include "TestTreeNode/TestTreeNode.hpp"
#include "WorldCoordinator/WorldCoordinator.hpp"
#include "NeochunkNode/NeochunkNode.hpp"
#include "NeochunkManager/NeochunkManager.hpp"
#include "BlockGenerator/BlockGenerator.hpp"
#include "BiomeSource/BiomeSource.hpp"
#include "BiomeGenerator/BiomeGenerator.hpp"
#include "BlockSource/BlockSource.hpp"


inline void auto_register_classes()
{
    ClassDB::register_class<PlayerSimple>();
    ClassDB::register_class<TestTreeNode>();
    ClassDB::register_class<WorldCoordinator>();
    ClassDB::register_class<NeochunkNode>();
    ClassDB::register_class<NeochunkManager>();
    ClassDB::register_class<BlockGenerator>();
    ClassDB::register_class<BiomeSource>();
    ClassDB::register_class<BiomeGenerator>();
    ClassDB::register_class<BlockSource>();
}
