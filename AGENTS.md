# Project

This project is a modern take on Minecraft using the Godot engine. The main objectives of the project are to be abstractly simple and performant.

## Architectural rules
- The relationship between shapes via add_owned_shape must be bidirectional (since a house cannot exist without walls and a roof, and a roof cannot exist if the house is destroyed)
- The owned list defines the traversal direction: change notifications and reactions only visit shapes explicitly listed in the current shape's owned list. A parent is never called unless it is itself listed in the child's owned list.
- propose_shape_change is the only entry point for a fundamental replacement (e.g. a branch becoming a burnt branch). Plain parameter changes are propagated by the shape itself down its owned list via notify_shape_changed().

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