#pragma once
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
using namespace godot;

#include "ToolManager/ToolManager.hpp"
#include "WorldCoordinator/WorldCoordinator.hpp"
#include "SurfaceGenerator/SurfaceGenerator.hpp"
#include "MaterialGenerator/MaterialGenerator.hpp"
#include "TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp"
#include "MeshGenerator/MeshGenerator.hpp"
#include "PlanetTerrainShape/PlanetTerrainShape.hpp"
#include "PlayerCamera/PlayerCamera.hpp"
#include "Tool/Tool.hpp"
#include "SemanticShape/SemanticShape.hpp"
#include "PickableTool/PickableTool.hpp"
#include "PlanetShape/PlanetShape.hpp"
#include "PlayerInteraction/PlayerInteraction.hpp"
#include "PlayerMovement/PlayerMovement.hpp"
#include "Player/Player.hpp"


inline void auto_register_virtual_classes() {
}

inline void auto_register_classes() {
    ClassDB::register_class<SemanticShape>();
    ClassDB::register_class<MeshGenerator>();
    ClassDB::register_class<MaterialGenerator>();
    ClassDB::register_class<ToolManager>();
    ClassDB::register_class<WorldCoordinator>();
    ClassDB::register_class<SurfaceGenerator>();
    ClassDB::register_class<TriplanarMaterialGenerator>();
    ClassDB::register_class<PlanetTerrainShape>();
    ClassDB::register_class<PlayerCamera>();
    ClassDB::register_class<Tool>();
    ClassDB::register_class<PickableTool>();
    ClassDB::register_class<PlanetShape>();
    ClassDB::register_class<PlayerInteraction>();
    ClassDB::register_class<PlayerMovement>();
    ClassDB::register_class<Player>();
}

inline void auto_register_singletons() {
}

inline void auto_unregister_singletons() {
}
