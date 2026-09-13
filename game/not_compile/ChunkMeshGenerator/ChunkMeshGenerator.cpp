#include "ChunkMeshGenerator.hpp"

#include <NeochunkManager/NeochunkManager.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp> // <-- ДОБАВЛЕНО: для Window*
#include <godot_cpp/classes/node.hpp>

namespace godot {

constexpr const char* ChunkMeshGenerator::MANAGER_NODE_PATH;

void ChunkMeshGenerator::_bind_methods() {
    // Свойства не требуются, так как путь зашит в код
}

ChunkMeshGenerator::ChunkMeshGenerator() {
}

Ref<ArrayMesh> ChunkMeshGenerator::generate(Ref<SemanticShape> shape, SemanticWorld* world) const {
    if (!shape.is_valid() || !world) {
        WARN_PRINT("ChunkMeshGenerator: shape или world невалидны. Генерация прервана.");
        return Ref<ArrayMesh>();
    }

    AABB bounds = shape->get_aabb();
    uint64_t world_id = world->get_instance_id();

    SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
    if (!tree) {
        WARN_PRINT("ChunkMeshGenerator: Не удалось получить SceneTree.");
        return Ref<ArrayMesh>();
    }

    // ИСПРАВЛЕНИЕ 1: get_root() возвращает Window*, так как корень дерева сцены в Godot 4 - это окно
    Window* root = tree->get_root();
    if (!root) {
        WARN_PRINT("ChunkMeshGenerator: Не удалось получить корень сцены (Window).");
        return Ref<ArrayMesh>();
    }

    // ИСПРАВЛЕНИЕ 2: get_node в Godot 4 C++ требует явного указания шаблонного параметра типа
    Node* manager_node = root->get_node<Node>(NodePath(MANAGER_NODE_PATH));
    NeochunkManager* manager = Object::cast_to<NeochunkManager>(manager_node);

    if (manager) {
        // ИСПРАВЛЕНИЕ 3: Этот вызов сработает только если метод public (см. шаг 2 ниже)
        manager->refresh_chunks_in_aabb(bounds, world_id);
    } else {
        WARN_PRINT(String("ChunkMeshGenerator: NeochunkManager не найден по пути: ") + MANAGER_NODE_PATH);
    }

    Ref<ArrayMesh> result;
    result.instantiate();
    
    return result;
}

} // namespace godot