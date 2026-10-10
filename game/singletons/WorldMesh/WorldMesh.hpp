#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <cstdint>

namespace godot {

class MeshGenerator;
class MaterialGenerator;
class Chunk;

/// @brief Управляет визуальным представлением (меши и материалы) семантических фигур в октодереве чанков.
/// Отвечает за создание, обновление и очистку MeshInstance3D, предотвращая артефакты LOD.
class WorldMesh : public Node3D {
    GDCLASS(WorldMesh, Node3D)
    
private:
    /// @brief Удаляет визуальное представление фигуры из конкретного чанка.
    void _remove_mesh_from_chunk(Chunk* chunk, uint64_t shape_id);
    
    /// @brief Рекурсивно удаляет меш фигуры из чанка и всех его дочерних элементов.
    /// Используется для корректного перехода LOD, чтобы дочерние чанки не рендерили то, что уже рендерит родитель.
    void _clear_shape_in_subtree(Chunk* node, uint64_t shape_id);

protected:
    static void _bind_methods();

public:
    /// @brief Возвращает глобальный экземпляр WorldMesh, зарегистрированный в Godot Engine.
    static WorldMesh* get_singleton();

    void _ready() override;
    void _process(double delta) override;

    /// @brief Инициирует процесс рендеринга фигуры в заданном чанке.
    /// Очищает старые меши этой фигуры в текущем чанке и его родителях для предотвращения визуальных артефактов.
    /// Создает базовый MeshInstance3D и сохраняет генераторы для последующего применения материалов.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, в котором требуется отрисовка.
    /// @param mesh_generator Генератор геометрии.
    /// @param material_generator Генератор материалов.
    void request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> mesh_generator, Ref<MaterialGenerator> material_generator);
    
    /// @brief Отменяет запрос на рендеринг и удаляет визуальное представление фигуры из чанка.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, из которого нужно удалить меш.
    void cancel_render(uint64_t shape_id, uint64_t chunk_id);
    
    /// @brief Завершает асинхронный процесс рендеринга, применяя сгенерированный Mesh и Material.
    /// Вызывается (обычно из главного потока), когда тяжелая генерация меша завершена.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, к которому применяется меш.
    /// @param mesh Готовый к использованию Ref<ArrayMesh>.
    void complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh);
};

} // namespace godot