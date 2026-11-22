# Project

This project is a modern take on Minecraft using the Godot engine. The main objectives of the project are to be abstractly simple and performant.

## Architecture

The game is a C++ GDExtension split into a semantic model, a spatial index and a render pipeline. Semantic shapes are `Resource`s that describe what exists; generators are `Resource`s that build geometry and materials; Godot nodes own the scene representation. `WorldCoordinator` is the core scene-side bridge that connects authored `SemanticShape` resources to the global simulation and render layer.

### Layers

- **Semantic layer:** `SemanticShape` subclasses store shape parameters, local position, lazy AABB, SDF and an `owned` graph. They define behaviour on change and do not create geometry or Godot nodes.
- **Registry and transaction layer:** `SemanticWorld` owns shape IDs, shape lifetime, zone dispatch and fundamental replacement. `ShapeSpatialManager` owns octrees, shape-to-octree bindings and global chunk IDs.
- **Spatial LOD layer:** `ChunkOctree` is a `Node3D` that splits and collapses `Chunk`s by camera distance and notifies subscribed shapes about active chunks. `Chunk` is storage and an update trigger; it does not own the octree.
- **Render layer:** `WorldMesh` batches invalidations, queues render jobs, enforces tokens and concurrency limits, and assembles final nodes. `RenderRequest` owns the visual, collision and area nodes for one shape in one chunk.
- **Scene bridge:** `WorldCoordinator` attaches `WorldMesh`, collects shapes from itself and child `WorldShapeNode`s, and registers planet shapes before dependent shapes.

### Runtime flows

1. **Registration:** `WorldCoordinator::_enter_tree()` attaches `WorldMesh`, collects effective shapes and calls `SemanticWorld::register_shape()`. Registration assigns an ID, stores the shape and calls `SemanticShape::on_register()`. An owner shape creates or rebuilds its octree through `ShapeSpatialManager::create_octree()` / `rebuild_octree()`; dependent shapes call `bind_spatially(parent_id)`. The parent must be registered first.
2. **LOD and rendering:** `ChunkOctree::_process()` converts the camera to octree-local space with `to_local()`, updates LOD and calls `SemanticWorld::on_zone_changed()`. The world forwards activated chunks to each shape’s `on_zone_changed(chunk_id)`. Shapes request rendering through `WorldMesh::request_render()`; generators produce meshes, and `WorldMesh::complete_mesh()` applies them through the current `RenderRequest`.
3. **Parameter change:** Shape setters call `notify_shape_changed()` after changing parameters. The base implementation marks the AABB dirty, calls `WorldMesh::invalidate_shape()` and notifies only shapes listed in the current shape’s `owned` list. `WorldMesh` flushes invalidations, clears affected chunk representations and marks octree users pending so the next LOD update re-requests active chunks. Overrides must call the base implementation.
4. **Fundamental replacement:** `SemanticWorld::propose_shape_change()` is the only entry point. It evaluates `evaluate_reaction()` along `owned` lists only, then `_commit_changes()` unregisters the old shape, registers the new one, rebinds owned IDs and invalidates owners. Plain parameter changes must not use this path. `evaluate_alchemy()` and `AlchemyArea3D` only produce replacement proposals. This is how a destroyed tree propagates changes to its branches and how a wall turning into a destroyed wall can notify a dependent house. See `Ownership (owned)` and `Alchemy` below.
5. **Unregistration:** `SemanticWorld::unregister_shape()` drops render representations before spatial bindings are removed, clears the shape ID and calls `on_unregister()`. `unregister_shape_with_owned()` removes owned child shapes first.

### Ownership (owned)

The `owned` graph is the semantic backbone of the engine. An edge `A → B` (`A.owned B`) means that `B` exists because of `A`: when `A` is semantically destroyed or fundamentally replaced, `B` is notified through `SemanticWorld::propose_shape_change()` and may propose its own replacement through `evaluate_reaction()`. For example, `TreeShape` owns its branches because a branch cannot exist without the tree.

`SemanticShape::add_owned_shape()` adds exactly one directed edge. It never creates the reverse edge automatically. If the dependency is semantically bidirectional, call it on both shapes: `house.add_owned_shape(wall)` and `wall.add_owned_shape(house)`. Then a wall becoming a destroyed wall can notify the house, and a house becoming a destroyed house can notify the wall. If the reverse edge is absent, the parent is not called: traversal always follows the current shape’s `owned` list.

There are two distinct propagation mechanisms:

- `notify_shape_changed()` handles parameter changes. It marks the AABB dirty, invalidates rendering and recursively notifies the current shape’s `owned` children. It never replaces shapes.
- `propose_shape_change()` handles fundamental replacement. It evaluates `evaluate_reaction()` on owned shapes, accepts concrete replacement shapes and atomically commits them: old shapes are unregistered, new shapes are registered, every `owned` list in the world is rebound from old IDs to new IDs, and previous owners are notified.

`unregister_shape_with_owned()` recursively unregisters owned children first; its visited set prevents cycles in bidirectional graphs from looping.

### Alchemy

Alchemy is interaction-driven semantic change between two shapes. It starts when two `AlchemyArea3D`s overlap; the area calls `shape->evaluate_alchemy(other_id)` on the shape it represents.

`evaluate_alchemy()` has two valid outcomes:

- **Internal change:** the shape mutates its own parameters with normal setters and returns `nullptr`. The resulting `notify_shape_changed()` calls invalidate AABB and render state; no replacement transaction is started.
- **Replacement proposal:** the shape returns a new concrete `SemanticShape`. `AlchemyArea3D` forwards it to `SemanticWorld::propose_shape_change()`, which runs the owned cascade. For example, a wall interacting with an explosion returns a destroyed wall shape.

Returning `nullptr` is the normal way to say “no replacement is needed”. A generic `"shape"` type, the same object, or a stale shape ID is ignored as a proposal. `evaluate_alchemy()` never mutates other shapes directly and is not the transaction layer itself.

### Coordinate model

- For shapes bound to an octree, `SemanticShape::_position`, AABB values and `evaluate_sdf()` arguments are octree-local.
- `PlanetShape` owns the octree: its `_position` is the octree `Node3D` transform, and the planet’s semantic center is octree-local zero.
- Chunk centers are storage identifiers. They define only SDF sampling bounds and mesh filtering, not semantic shape positions.
- SDF sampling stays octree-local. Low-level meshing receives the shape origin and emits vertices in shape-local coordinates: octree-local coordinates minus the shape’s `_position`.
- Mesh, collision and `Area3D` nodes are children of the octree and are positioned at the registered shape’s `_position`. `Node3D` applies the octree transform, so no manual offset arithmetic is needed.
- `ChunkOctree::_process()` converts the camera to octree-local space before LOD evaluation.
- `SurfaceShape::_position` is zero, so its semantic center matches `PlanetShape`.

## Core class map

| Class / file | Responsibility |
|---|---|
| `SemanticShape` — `game/objects/SemanticShape/SemanticShape.hpp` | Base contract for shape parameters, local position, lazy AABB, SDF, the directed `owned` graph, change notification and lifecycle hooks. `evaluate_reaction()` and `evaluate_alchemy()` return `nullptr` when no replacement is needed; it never creates geometry or nodes. |
| `SemanticWorld` — `game/singletons/SemanticWorld/SemanticWorld.hpp` | Global shape registry, ID allocation, zone notification entry point, replacement cascade and global `owned` ID rebinding. |
| `WorldCoordinator` — `game/objects/WorldCoordinator/WorldCoordinator.hpp` | Collects shapes from the scene, attaches `WorldMesh` and registers or unregisters shapes in dependency order. |
| `WorldShapeNode` — `game/objects/WorldShapeNode/WorldShapeNode.hpp` | Scene/editor adapter that exposes a `SemanticShape` as a `Node3D` and refreshes the coordinator. |
| `ShapeSpatialManager` — `game/singletons/ShapeSpatialManager/ShapeSpatialManager.hpp` | Creates, rebuilds and destroys octrees; binds shapes to an octree or a parent’s grid; provides global chunk IDs and octree/chunk lookups. |
| `ChunkOctree` — `game/objects/ChunkOctree/ChunkOctree.hpp` | Octree `Node3D`: camera-based LOD, split/collapse rules, active-chunk tracking and user notification. |
| `Chunk` — `game/singletons/ShapeSpatialManager/Chunk.hpp` | Octree node storage: local AABB and per-shape `RenderRequest`s; removes or clears per-shape representations. |
| `WorldMesh` — `game/singletons/WorldMesh/WorldMesh.hpp` | Render orchestrator: invalidation batching, request queue, tokens, cancellation, concurrency limit, sync/async dispatch and final node assembly. |
| `RenderRequest` — `game/singletons/WorldMesh/RenderRequest.hpp` | One shape-in-chunk representation; owns `MeshInstance3D`, collision and `Area3D` nodes and applies token-checked results. |
| `MeshGenerator` — `game/objects/MeshGenerator/MeshGenerator.hpp` | Base synchronous geometry generator: `builder()`, triangle `emit()` and `submit()`; handles chunk clipping and shape-local vertex coordinates. |
| `SurfaceGenerator` — `game/objects/SurfaceGenerator/SurfaceGenerator.hpp` | Asynchronous terrain mesh generator that runs SDF baking and Surface Nets in `WorkerThreadPool`. |
| `SurfaceNets`, `VoxelBaker`, `VoxelCache` — `game/objects/SurfaceGenerator/`, `game/common/Utils/` | SDF sampling buffer, AABB prefilter and `build_neochunk_mesh()` extraction. Output vertices are shape-local. |
| `TrunkMeshGenerator`, `BranchMeshGenerator` — `game/objects/TrunkMeshGenerator/`, `game/objects/BranchMeshGenerator/` | Procedural geometry for trunk rings and branch cards; parameters are read from the owning shapes. |
| `MaterialGenerator` and subclasses — `game/objects/MaterialGenerator/`, `game/objects/PlanetMaterialGenerator/`, `game/objects/BranchMaterialGenerator/`, `game/objects/TriplanarMaterialGenerator/` | Material creation from shape data; terrain biome/color data is encoded by `SemanticShape::paint_vertices()` and consumed by shaders. |
| `GravityAreaGenerator` — `game/objects/GravityAreaGenerator/` | Builds the sphere mesh used by a gravity `Area3D`. |
| `PlanetShape` — `game/objects/PlanetShape/PlanetShape.hpp` | Planet owner: radius/amplitude parameters, octree creation/rebuild and creation of gravity and surface children. |
| `SurfaceShape` — `game/objects/SurfaceShape/SurfaceShape.hpp` | Terrain SDF, directional noise, biome colors and surface mesh requests for active chunks. |
| `TreeShape` — `game/objects/TreeShape/TreeShape.hpp` | Composite shape: spawns and synchronizes trunk and branches, aggregates owned AABBs and owns branch dependencies. |
| `TrunkShape`, `BranchShape` — `game/objects/TrunkShape/`, `game/objects/BranchShape/` | Parameters and AABBs for tree parts; request their own generated meshes on chunk activation. |
| `GravityShape` — `game/objects/GravityShape/GravityShape.hpp` | Radial up vector, gravity-area radius and gravity `Area3D` requests; stays at octree-local zero. |
| `AlchemyArea3D` — `game/objects/AlchemyArea3D/AlchemyArea3D.hpp` | `Area3D` bound to a shape ID; turns overlaps into `evaluate_alchemy()` and either internal parameter changes or a `propose_shape_change()` proposal. |

## Where to look

- Shape parameters, AABB or SDF changes: the relevant `SemanticShape` subclass; call `notify_shape_changed()` after mutation.
- Directed `owned` edges and bidirectional dependencies: `SemanticShape::add_owned_shape()`, `SemanticWorld::_commit_changes()` and the `Ownership (owned)` section.
- Replacement cascades and reactions: `SemanticWorld::propose_shape_change()`, `SemanticShape::evaluate_reaction()` and `AlchemyArea3D`.
- Alchemy interactions: `SemanticShape::evaluate_alchemy()` and the target shape’s setters for internal parameter changes.
- Octree creation, LOD or chunk activation: `PlanetShape::on_register()`, `ShapeSpatialManager` and `ChunkOctree`.
- Mesh clipping or vertex coordinate bugs: `MeshGenerator::builder()`, `SurfaceGenerator`, `VoxelBaker` and `SurfaceNets`.
- Render, collision or area node lifecycle and stale async results: `WorldMesh`, `RenderRequest` and `Chunk`.
- Materials and biome colors: the `MaterialGenerator` hierarchy and `SemanticShape::paint_vertices()`.
- Registration order and scene integration: `WorldCoordinator` and `WorldShapeNode`.

## Invariants

- `add_owned_shape()` adds one directed edge. Bidirectional dependency is expressed by two explicit calls, one on each shape; there is no automatic reverse edge.
- The `owned` list defines traversal direction. Change notifications and reactions visit only shapes explicitly listed in the current shape’s `owned` list. A parent is never called unless it is itself listed in the child’s `owned` list.
- `propose_shape_change()` is the only entry point for a fundamental replacement. Plain parameter changes propagate through `notify_shape_changed()`.
- Returning `nullptr` from `evaluate_alchemy()` or `evaluate_reaction()` means no replacement; internal parameter changes still use setters and `notify_shape_changed()`.
- Only a different concrete `SemanticShape` is a replacement proposal. Generic `"shape"`, the same object or a stale ID are ignored.
- Shapes are the source of truth for their parameters; generators must read parameters from shapes and chunks instead of duplicating them.
- `shape_id == 0` means unregistered and is ignored by render endpoints.
- A parent shape must be registered before a child calls `bind_spatially()`.
- Small shapes that can span multiple chunks must suppress duplicate effects by checking that their AABB center lies inside the notified chunk.
- Tokens are the only protection against stale asynchronous mesh results; every completion path must respect them.

## Code rules

- Avoid creating new fields, classes, or functions unless clearly necessary.
- Do not adhere to explicit and preemptively slow solutions, except in response to explicit requests.

## Build rules

- The project must be built using the command `clear && scons platform=linux debug_symbols=true`
- Code implemented in accordance with the task and a build completed without errors is considered a completed task
- Running the Godot project is possible only if debugging is requested and only in headless only mode

## Work rules

- Creating additional agents is prohibited without the user’s permission or request
- Don’t use git