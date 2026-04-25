#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "OuterWildsCharacter/OuterWildsCharacter.hpp"
#include "WorldCoordinator/WorldCoordinator.hpp"
#include "NeochunkNode/NeochunkNode.hpp"
#include "NeochunkManager/NeochunkManager.hpp"
#include "Tool/Tool.hpp"
#include "BlockGenerator/BlockGenerator.hpp"
#include "BiomeSource/BiomeSource.hpp"
#include "BiomeGenerator/BiomeGenerator.hpp"
#include "BlockSource/BlockSource.hpp"
#include "SuperDigger/SuperDigger.hpp"


inline void auto_register_classes()
{
    ClassDB::register_class<OuterWildsCharacter>();
    ClassDB::register_class<WorldCoordinator>();
    ClassDB::register_class<NeochunkNode>();
    ClassDB::register_class<NeochunkManager>();
    ClassDB::register_class<Tool>();
    ClassDB::register_class<BlockGenerator>();
    ClassDB::register_class<BiomeSource>();
    ClassDB::register_class<BiomeGenerator>();
    ClassDB::register_class<BlockSource>();
    ClassDB::register_class<SuperDigger>();
}
