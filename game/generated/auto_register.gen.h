#pragma once
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
using namespace godot;

#include "MeshPrimitive/MeshPrimitive.hpp"
#include "ChunkMeshNode/ChunkMeshNode.hpp"
#include "Tool/Tool.hpp"
#include "BlockGenerator/BlockGenerator.hpp"
#include "PlayerCamera/PlayerCamera.hpp"
#include "SemanticCurve/SemanticCurve.hpp"
#include "PlayerMovement/PlayerMovement.hpp"
#include "BiomeSource/BiomeSource.hpp"
#include "BuildTool/BuildTool.hpp"
#include "MeshYJoint/MeshYJoint.hpp"
#include "BiomeGenerator/BiomeGenerator.hpp"
#include "BlockSource/BlockSource.hpp"
#include "SuperDigger/SuperDigger.hpp"
#include "BuildSocket/BuildSocket.hpp"
#include "MeshCylinder/MeshCylinder.hpp"
#include "PotentialEditor/PotentialEditor.hpp"
#include "BuildBlock/BuildBlock.hpp"
#include "PotentialSource/PotentialSource.hpp"
#include "TreeGenerator/TreeGenerator.hpp"
#include "Player/Player.hpp"
#include "PickableTool/PickableTool.hpp"
#include "WorldCoordinator/WorldCoordinator.hpp"
#include "PlayerInteraction/PlayerInteraction.hpp"
#include "NeochunkNode/NeochunkNode.hpp"
#include "SemanticShape/SemanticShape.hpp"
#include "NeochunkManager/NeochunkManager.hpp"
#include "ToolManager/ToolManager.hpp"
#include "SemanticWorld/SemanticWorld.hpp"
#include "BuildGraph/BuildGraph.hpp"

static SemanticWorld *SemanticWorld_instance = nullptr;
static BuildGraph *BuildGraph_instance = nullptr;

inline void auto_register_virtual_classes() {
}

inline void auto_register_classes() {
    ClassDB::register_class<SemanticShape>();
    ClassDB::register_class<MeshPrimitive>();
    ClassDB::register_class<ChunkMeshNode>();
    ClassDB::register_class<Tool>();
    ClassDB::register_class<BlockGenerator>();
    ClassDB::register_class<PlayerCamera>();
    ClassDB::register_class<SemanticCurve>();
    ClassDB::register_class<PlayerMovement>();
    ClassDB::register_class<BiomeSource>();
    ClassDB::register_class<BuildTool>();
    ClassDB::register_class<MeshYJoint>();
    ClassDB::register_class<BiomeGenerator>();
    ClassDB::register_class<BlockSource>();
    ClassDB::register_class<SuperDigger>();
    ClassDB::register_class<BuildSocket>();
    ClassDB::register_class<MeshCylinder>();
    ClassDB::register_class<PotentialEditor>();
    ClassDB::register_class<BuildBlock>();
    ClassDB::register_class<PotentialSource>();
    ClassDB::register_class<TreeGenerator>();
    ClassDB::register_class<Player>();
    ClassDB::register_class<PickableTool>();
    ClassDB::register_class<WorldCoordinator>();
    ClassDB::register_class<PlayerInteraction>();
    ClassDB::register_class<NeochunkNode>();
    ClassDB::register_class<NeochunkManager>();
    ClassDB::register_class<ToolManager>();
    ClassDB::register_class<SemanticWorld>();
    ClassDB::register_class<BuildGraph>();
}

inline void auto_register_singletons() {
    SemanticWorld_instance = memnew(SemanticWorld);
    Engine::get_singleton()->register_singleton("SemanticWorld", SemanticWorld_instance);
    BuildGraph_instance = memnew(BuildGraph);
    Engine::get_singleton()->register_singleton("BuildGraph", BuildGraph_instance);
}

inline void auto_unregister_singletons() {
    Engine::get_singleton()->unregister_singleton("SemanticWorld");
    memdelete(SemanticWorld_instance);
    SemanticWorld_instance = nullptr;
    Engine::get_singleton()->unregister_singleton("BuildGraph");
    memdelete(BuildGraph_instance);
    BuildGraph_instance = nullptr;
}
