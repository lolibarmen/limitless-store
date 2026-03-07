#include "ChunkCollider.hpp"
#include <ChunkNode/ChunkNode.hpp>

#include <godot_cpp/classes/concave_polygon_shape3d.hpp>

using namespace godot;

ChunkCollider::ChunkCollider() {}

ChunkCollider::~ChunkCollider() {}

void ChunkCollider::_bind_methods() {
    // Биндим основные методы
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &ChunkCollider::set_mesh);
    
    // Биндим методы для лучей (чтобы их можно было вызывать из GDScript)
    ClassDB::bind_method(D_METHOD("on_ray_hit", "result"), &ChunkCollider::on_ray_hit);
    ClassDB::bind_method(D_METHOD("on_ray_enter"), &ChunkCollider::on_ray_enter);
    ClassDB::bind_method(D_METHOD("on_ray_exit"), &ChunkCollider::on_ray_exit);
    ClassDB::bind_method(D_METHOD("on_ray_hover", "position"), &ChunkCollider::on_ray_hover);
    
    // Опционально: добавим сигналы
    ADD_SIGNAL(MethodInfo("ray_hit", PropertyInfo(Variant::DICTIONARY, "result")));
    ADD_SIGNAL(MethodInfo("ray_entered"));
    ADD_SIGNAL(MethodInfo("ray_exited"));
}

void ChunkCollider::_ready() {
    // Создаём CollisionShape3D как дочерний узел
    collision_shape = memnew(CollisionShape3D);
    add_child(collision_shape);
}

void ChunkCollider::set_chunk(ChunkNode* p_chunk) {
    chunk_node = p_chunk;
}

void ChunkCollider::set_mesh(const Ref<ArrayMesh> mesh) {
    if (mesh.is_valid()) {
        Ref<ConcavePolygonShape3D> shape;
        shape.instantiate();
        shape->set_faces(mesh->get_faces());
        collision_shape->set_shape(shape);
    }
}

void ChunkCollider::on_ray_hit(const Dictionary &result) {
    // Вызывается, когда луч попадает в этот коллайдер
    // UtilityFunctions::print("ChunkCollider: Ray hit at position: ", result["position"]);
    
    // Здесь можно добавить логику:
    // - Изменение цвета
    // - Воспроизведение звука
    // - Применение урона
    // - И т.д.
    
    chunk_node->on_ray_hit(result);
    
    // Если нужно вызвать сигнал или изменить состояние
    // _on_ray_hit(result);
}

void ChunkCollider::on_ray_enter() {
    // Вызывается, когда луч только начал указывать на этот объект
    // UtilityFunctions::print("ChunkCollider: Ray entered");
    
    // Например, подсветить объект
    // set_highlight(true);
    
    // _on_ray_enter();
}

void ChunkCollider::on_ray_exit() {
    // Вызывается, когда луч перестал указывать на этот объект
    // UtilityFunctions::print("ChunkCollider: Ray exited");
    
    // Убрать подсветку
    // set_highlight(false);
    
    // _on_ray_exit();
}

void ChunkCollider::on_ray_hover(const Vector3 &position) {
    // Вызывается каждый кадр, пока луч указывает на объект
    // UtilityFunctions::print("ChunkCollider: Ray hovering at ", position);
    
    // Можно обновлять позицию прицела или эффекты
}