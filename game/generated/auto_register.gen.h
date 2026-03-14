#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
#include "objects/ChunkMesh/ChunkMesh.hpp"
#include "objects/TreeNode/TreeNode.hpp"
#include "objects/ChunkCollider/ChunkCollider.hpp"
#include "objects/ChunkNode/ChunkNode.hpp"
#include "resources/PlanetData/PlanetData.hpp"
#include "managers/PlanetManager/PlanetManager.hpp"
#include "managers/WorldManager/WorldManager.hpp"
#include "managers/ChunkManager/ChunkManager.hpp"
#include "generators/BiomeGenerator/BiomeGenerator.hpp"

inline void auto_register_classes()
{
    ClassDB::register_class<ChunkMesh>();
    ClassDB::register_class<TreeNode>();
    ClassDB::register_class<ChunkCollider>();
    ClassDB::register_class<ChunkNode>();
    ClassDB::register_class<PlanetData>();
    ClassDB::register_class<PlanetManager>();
    ClassDB::register_class<WorldManager>();
    ClassDB::register_class<ChunkManager>();
    ClassDB::register_class<BiomeGenerator>();
}
