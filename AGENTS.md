# Project

This project is a modern take on Minecraft using the Godot engine. The main objectives of the project are to be abstractly simple and performant.

## Architectural rules
- The relationship between shapes via add_owned_shape must be bidirectional (since a house cannot exist without walls and a roof, and a roof cannot exist if the house is destroyed)

## Code rules
- Avoid creating new fields, classes, or functions unless clearly necessary.
- Do not adhere to explicit and preemptively slow solutions, except in response to explicit requests.

## Build rules
- The project must be built using the command `clear && scons platform=linux debug_symbols=true`
- Code implemented in accordance with the task and a build completed without errors is considered a completed task
- Running the Godot project is possible only if debugging is requested and only in headless only mode

## Work rules
- Creating additional agents for analyzing the project architecture is not encouraged.