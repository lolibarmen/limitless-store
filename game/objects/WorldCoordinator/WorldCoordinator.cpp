#include "WorldCoordinator.hpp"
#include <ChunkMeshNode/ChunkMaterialManager.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <SemanticCurve/SemanticCurve.hpp>

using namespace godot;

void WorldCoordinator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_seed"), &WorldCoordinator::get_seed);
    ClassDB::bind_method(D_METHOD("set_seed", "v"), &WorldCoordinator::set_seed);
    ClassDB::add_property("WorldCoordinator",
        PropertyInfo(Variant::INT, "seed"),
        "set_seed", "get_seed");
    
    // ВАЖНО: Биндим новый метод, чтобы call_deferred мог его найти по имени
    ClassDB::bind_method(D_METHOD("_spawn_test_curve"), &WorldCoordinator::_spawn_test_curve);
}

void WorldCoordinator::_ready() {
    // 1. Инициализация менеджера материалов
    ChunkMaterialManager::get_singleton().initialize();

    // 2. Создаем и добавляем менеджер чанков
    chunk_manager = memnew(NeochunkManager);
    add_child(chunk_manager);

    // 3. ОТЛАГАЕМ создание кривой до конца кадра.
    // К этому моменту NeochunkManager уже успеет вызвать _process, 
    // найти камеру и заполнить словарь roots.
    call_deferred("_spawn_test_curve");
}

void WorldCoordinator::_spawn_test_curve() {
    // Теперь мы уверены, что сцена полностью проинициализирована
    
    Object* sw_obj = Engine::get_singleton()->get_singleton("SemanticWorld");
    SemanticWorld* semantic_world = Object::cast_to<SemanticWorld>(sw_obj);

    if (!semantic_world) {
        print_line("ERROR: SemanticWorld singleton not found!");
        return;
    }

    // 4. Создаем тестовую кривую
    Ref<SemanticCurve> test_curve;
    test_curve.instantiate();

    // 5. Добавляем точки для формирования дуги
    test_curve->add_point(Vector3(-16.0f, 0.0f, -16.0f), Vector3(0.0f, 0.0f, 5.0f), Vector3(0.0f, 0.0f, 5.0f), 3.0f);
    test_curve->add_point(Vector3(  0.0f, 8.0f,   0.0f), Vector3(0.0f, 5.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f), 3.0f);
    test_curve->add_point(Vector3( 16.0f, 0.0f,  16.0f), Vector3(0.0f, 0.0f,-5.0f), Vector3(0.0f, 0.0f,-5.0f), 3.0f);

    // 6. Устанавливаем ID материала
    test_curve->set_property(0, 128.0f);

    // 7. Регистрируем фигуру в мире
    uint64_t shape_id = semantic_world->register_shape(test_curve);

    print_line("SUCCESS: Shape registered with ID: ", shape_id);
}