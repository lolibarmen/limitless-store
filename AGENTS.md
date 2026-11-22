# Project

This project is a modern take on Minecraft using the Godot engine. The main objectives of the project are to be abstractly simple and performant.

## Architectural rules
- The relationship between shapes via add_owned_shape must be bidirectional (since a house cannot exist without walls and a roof, and a roof cannot exist if the house is destroyed)
- The owned list defines the traversal direction: change notifications and reactions only visit shapes explicitly listed in the current shape's owned list. A parent is never called unless it is itself listed in the child's owned list.
- propose_shape_change is the only entry point for a fundamental replacement (e.g. a branch becoming a burnt branch). Plain parameter changes are propagated by the shape itself down its owned list via notify_shape_changed().
- Coordinates: For shapes bound to an octree, `SemanticShape::_position`, AABB and SDF arguments are octree-local; the octree-owning `PlanetShape` uses `_position` as the octree `Node3D` transform and its semantic center is octree-local zero. The octree is a `Node3D` child of `WorldMesh`. Chunks are storage/update triggers; chunk centers define only SDF sampling bounds and mesh filtering. Mesh/collision/Area3D nodes are children of the octree, positioned at the registered shape's `_position`, and generated vertices are shape-local (octree-local minus `_position`), so Node3D multiplies that position by the octree transform. `ChunkOctree::_process` converts the camera to octree-local with `to_local()` before LOD. SurfaceShape's local position is zero, so its center matches PlanetShape. SDF sampling stays octree-local; low-level meshing receives `shape_origin` and emits shape-local vertices in one pass. Relative transforms are handled by Node3D, not manual offset arithmetic.

## Code rules
- Avoid creating new fields, classes, or functions unless clearly necessary.
- Do not adhere to explicit and preemptively slow solutions, except in response to explicit requests.

## Build rules
- The project must be built using the command `clear && scons platform=linux debug_symbols=true`
- Code implemented in accordance with the task and a build completed without errors is considered a completed task
- Running the Godot project is possible only if debugging is requested and only in headless only mode

## Work rules
- Creating additional agents is not recommended for performing sequential tasks
- Don’t use git