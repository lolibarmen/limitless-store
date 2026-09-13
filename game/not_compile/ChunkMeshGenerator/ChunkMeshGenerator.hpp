#pragma once

#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

namespace godot {

class ChunkMeshGenerator : public MeshGenerator {
    GDCLASS(ChunkMeshGenerator, MeshGenerator)

private:
    // Жестко заданный путь к ноде NeochunkManager
    static constexpr const char* MANAGER_NODE_PATH = "/root/WorldCoordinator/NeochunkManager";

protected:
    static void _bind_methods();

public:
    ChunkMeshGenerator();
    ~ChunkMeshGenerator() override = default;

    // Переопределение метода генерации
    Ref<ArrayMesh> generate(Ref<SemanticShape> shape, SemanticWorld* world) const override;
};

} // namespace godot