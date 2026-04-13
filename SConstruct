import os

# ------------------------------------------------
# Подключение Godot C++ environment
# ------------------------------------------------
env = SConscript("godot-cpp/SConstruct")

GAME_DIR = "game"
OBJECT_DIR = os.path.join(GAME_DIR, "objects")
RESOURCE_DIR = os.path.join(GAME_DIR, "resources")
GENERATED_DIR = os.path.join(GAME_DIR, "generated")

# include paths
env.Append(CPPPATH=[
    "#game",
    "#game/common",
    "#game/objects",
    "#game/resources",
    "#game/generated"
])

# создаём директорию для автогенерации, если нет
if not os.path.exists(GENERATED_DIR):
    os.makedirs(GENERATED_DIR)

all_sources = []
includes = ""
registrations = ""

# ------------------------------------------------
# AUTO DISCOVERY FUNCTION
# ------------------------------------------------
def discover_cpp_classes(base_dir):
    global includes, registrations, all_sources
    for root, dirs, files in os.walk(base_dir):
        cpp_files = [f for f in files if f.endswith(".cpp")]
        header_files = [f for f in files if f.endswith(".hpp")]

        if not cpp_files:
            continue

        folder_name = os.path.basename(root)
        print(f"Discovered folder: {folder_name} | cpp files: {len(cpp_files)}")

        # добавляем cpp файлы в сборку
        for cpp in cpp_files:
            all_sources.append(os.path.join(root, cpp))

        # формируем includes и registration
        for h in header_files:
            class_name = os.path.splitext(h)[0]
            if class_name == folder_name:
                includes += f'#include "{folder_name}/{h}"\n'
                registrations += f'    ClassDB::register_class<{class_name}>();\n'

# ------------------------------------------------
# Discover objects and resources
# ------------------------------------------------
discover_cpp_classes(OBJECT_DIR)
discover_cpp_classes(RESOURCE_DIR)  # теперь учитываем PlanetData и другие ресурсы

# ------------------------------------------------
# GENERATE AUTO REGISTER FILE
# ------------------------------------------------
auto_file = os.path.join(GENERATED_DIR, "auto_register.gen.h")

with open(auto_file, "w") as f:
    f.write(f"""#pragma once
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

// === AUTO GENERATED INCLUDES ===
{includes}

inline void auto_register_classes()
{{
{registrations}}}
""")

print("Generated auto_register.gen.h")

# ------------------------------------------------
# REGISTER TYPES
# ------------------------------------------------
register_types_file = os.path.join(GAME_DIR, "register_types.cpp")
if os.path.exists(register_types_file):
    all_sources.append(register_types_file)
else:
    print("Warning: register_types.cpp not found!")

# ------------------------------------------------
# BUILD SHARED LIBRARY
# ------------------------------------------------
output_dir = "godot/addons/game/bin"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

lib = env.SharedLibrary(
    target=os.path.join(output_dir, "libgame"),
    source=all_sources
)

Default(lib)