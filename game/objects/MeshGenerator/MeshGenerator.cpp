#include "MeshGenerator.hpp"
#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MeshGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_shape", "shape"), &MeshGenerator::set_shape);
    ClassDB::bind_method(D_METHOD("get_shape"), &MeshGenerator::get_shape);
    
    // Добавляем свойство в редактор Godot с подсказкой типа ресурса
    ClassDB::add_property("MeshGenerator", 
        PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "SemanticShape"), 
        "set_shape", "get_shape");
}

void MeshGenerator::set_shape(Ref<SemanticShape> p_shape) {
    if (_shape == p_shape) return;
    
    // Очищаем ссылку на старый генератор у предыдущей фигуры
    if (_shape.is_valid()) {
        _shape->set_generator(Ref<MeshGenerator>());
    }

    _shape = p_shape;

    // Записываем указатель на этот генератор в новую фигуру
    if (_shape.is_valid()) {
        _shape->set_generator(this);
    }
}

Ref<SemanticShape> MeshGenerator::get_shape() const {
    return _shape;
}

void MeshGenerator::on_shape_changed(SemanticShape* p_shape) {
    // Базовая реализация: здесь можно запустить перегенерацию меша
    // Например: Ref<ArrayMesh> mesh = generate_mesh();
    // Или отправить сигнал, если генератор тоже будет наследоваться от Node в будущем
}

Ref<ArrayMesh> MeshGenerator::generate_mesh() const {
    // Возвращаем пустой меш по умолчанию. Наследники переопределят этот метод.
    return memnew(ArrayMesh);
}