#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "Tool/Tool.hpp"
#include "BuildTool/BuildTool.hpp"
#include "SuperDigger/SuperDigger.hpp"
#include "BuildBlock/BuildBlock.hpp"
#include "Player/Player.hpp"
#include "PickableTool/PickableTool.hpp"
#include "WorldCoordinator/WorldCoordinator.hpp"
#include "NeochunkNode/NeochunkNode.hpp"
#include "NeochunkManager/NeochunkManager.hpp"
#include "ToolManager/ToolManager.hpp"
#include "BlockGenerator/BlockGenerator.hpp"
#include "PlayerCamera/PlayerCamera.hpp"
#include "PlayerMovement/PlayerMovement.hpp"
#include "BiomeSource/BiomeSource.hpp"
#include "BiomeGenerator/BiomeGenerator.hpp"
#include "BlockSource/BlockSource.hpp"
#include "PlayerInteraction/PlayerInteraction.hpp"


inline void auto_register_classes()
{
    ClassDB::register_class<Tool>();
    ClassDB::register_class<BuildTool>();
    ClassDB::register_class<SuperDigger>();
    ClassDB::register_class<BuildBlock>();
    ClassDB::register_class<Player>();
    ClassDB::register_class<PickableTool>();
    ClassDB::register_class<WorldCoordinator>();
    ClassDB::register_class<NeochunkNode>();
    ClassDB::register_class<NeochunkManager>();
    ClassDB::register_class<ToolManager>();
    ClassDB::register_class<BlockGenerator>();
    ClassDB::register_class<PlayerCamera>();
    ClassDB::register_class<PlayerMovement>();
    ClassDB::register_class<BiomeSource>();
    ClassDB::register_class<BiomeGenerator>();
    ClassDB::register_class<BlockSource>();
    ClassDB::register_class<PlayerInteraction>();
}
