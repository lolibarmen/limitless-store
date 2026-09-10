#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

namespace godot {

class SemanticShape; // Предварительное объявление для избежания циклических зависимостей

class MeshGenerator : public Resource {
    GDCLASS(MeshGenerator, Resource)

protected:
    static void _bind_methods();

private:
    Ref<SemanticShape> _shape;

public:
    MeshGenerator() = default;
    virtual ~MeshGenerator() = default;

    // Установка фигуры (автоматически регистрирует этот генератор в фигуре)
    void set_shape(Ref<SemanticShape> p_shape);
    Ref<SemanticShape> get_shape() const;

    // Вызывается самой фигурой при своём изменении
    virtual void on_shape_changed(SemanticShape* p_shape);

    // Виртуальный метод для переопределения в наследниках (не делает класс чисто абстрактным)
    virtual Ref<ArrayMesh> generate_mesh() const;
};

} // namespace godot