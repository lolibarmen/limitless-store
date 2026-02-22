#include "generated/auto_register.gen.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_game_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
        return;

    auto_register_classes();
}

void uninitialize_game_module(ModuleInitializationLevel p_level) {
    // Ничего не делаем при деинициализации
}

extern "C" {
// Правильная сигнатура для Godot 4.0+
GDExtensionBool GDE_EXPORT game_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, 
                                      GDExtensionClassLibraryPtr p_library, 
                                      GDExtensionInitialization *r_initialization) {
    // Создаем объект инициализации
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    // Регистрируем функции инициализации и деинициализации
    init_obj.register_initializer(initialize_game_module);
    init_obj.register_terminator(uninitialize_game_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    // Возвращаем результат инициализации
    return init_obj.init();
}
}