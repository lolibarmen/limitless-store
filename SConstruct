import os

# ------------------------------------------------
# Подключение Godot C++ environment
# ------------------------------------------------
env = SConscript("godot-cpp/SConstruct")

GAME_DIR = "game"
GENERATED_DIR = os.path.join(GAME_DIR, "generated")

# Все директории с исходниками
SOURCE_DIRS = [
    os.path.join(GAME_DIR, "objects"),
    os.path.join(GAME_DIR, "resources"),
    os.path.join(GAME_DIR, "managers"),
    os.path.join(GAME_DIR, "generators"),
]

# include paths — корень + все source dirs + core
env.Append(CPPPATH=[
    "#game",
    "#game/objects",
    "#game/resources",
    "#game/managers",
    "#game/generators",
    "#game/generated",
])

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

    if not os.path.exists(base_dir):
        print(f"Skipping (not found): {base_dir}")
        return

    for root, dirs, files in os.walk(base_dir):
        cpp_files = [f for f in files if f.endswith(".cpp")]
        header_files = [f for f in files if f.endswith(".hpp")]

        if not cpp_files:
            continue

        folder_name = os.path.basename(root)
        print(f"Discovered: {folder_name} | cpp: {len(cpp_files)}")

        for cpp in cpp_files:
            all_sources.append(os.path.join(root, cpp))

        for h in header_files:
            # путь относительно game/ для include
            rel_path = os.path.relpath(os.path.join(root, h), GAME_DIR)
            rel_path = rel_path.replace("\\", "/")
            includes += f'#include "{rel_path}"\n'
            class_name = os.path.splitext(h)[0]
            registrations += f'    ClassDB::register_class<{class_name}>();\n'

# ------------------------------------------------
# Discover all source directories
# ------------------------------------------------
for source_dir in SOURCE_DIRS:
    discover_cpp_classes(source_dir)

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