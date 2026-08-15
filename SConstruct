import os

env = SConscript("godot-cpp/SConstruct")

env.Tool('compilation_db')
env.CompilationDatabase('compile_commands.json')

if env["platform"] == "windows" and env.get("use_mingw"):
    if "CPATH" in os.environ:
        env["ENV"]["CPATH"] = os.environ["CPATH"]
    if "LIBRARY_PATH" in os.environ:
        env["ENV"]["LIBRARY_PATH"] = os.environ["LIBRARY_PATH"]

    lib_path_dirs = [p for p in os.environ.get("LIBRARY_PATH", "").split(":") if p]
    env.Append(LIBPATH=lib_path_dirs)

GAME_DIR = "game"
OBJECT_DIR = os.path.join(GAME_DIR, "objects")
SINGLETONS_DIR = os.path.join(GAME_DIR, "singletons")
VIRTUAL_DIR = os.path.join(GAME_DIR, "virtual")
GENERATED_DIR = os.path.join(GAME_DIR, "generated")

env.Append(CPPPATH=[
    "#game",
    "#game/common",
    "#game/objects",
    "#game/generated",
    "#game/singletons",
    "#game/virtual",
])
os.makedirs(GENERATED_DIR, exist_ok=True)

all_sources = []
includes = ""
class_registrations = ""
virtual_class_registrations = ""
singleton_decls = ""
singleton_inits = ""
singleton_frees = ""


def discover_cpp_classes(base_dir, is_singleton=False, is_virtual=False):
    global includes, class_registrations, virtual_class_registrations
    global singleton_decls, singleton_inits, singleton_frees, all_sources

    for root, dirs, files in os.walk(base_dir):
        cpp_files = [f for f in files if f.endswith(".cpp")]
        header_files = [f for f in files if f.endswith(".hpp")]
        folder_name = os.path.basename(root)

        all_sources += [os.path.join(root, cpp) for cpp in cpp_files]

        for h in header_files:
            class_name = os.path.splitext(h)[0]
            if class_name != folder_name:
                continue

            includes += f'#include "{folder_name}/{h}"\n'

            if is_virtual:
                virtual_class_registrations += f"    ClassDB::register_class<{class_name}>();\n"
            else:
                class_registrations += f"    ClassDB::register_class<{class_name}>();\n"

            if is_singleton:
                singleton_decls += f"static {class_name} *{class_name}_instance = nullptr;\n"
                singleton_inits += (
                    f'    {class_name}_instance = memnew({class_name});\n'
                    f'    Engine::get_singleton()->register_singleton("{class_name}", {class_name}_instance);\n'
                )
                singleton_frees += (
                    f'    Engine::get_singleton()->unregister_singleton("{class_name}");\n'
                    f'    memdelete({class_name}_instance);\n'
                    f'    {class_name}_instance = nullptr;\n'
                )


# Порядок важен: виртуальные (интерфейсные) классы регистрируются первыми,
# т.к. обычные "objects" зачастую от них наследуются.
# discover_cpp_classes(VIRTUAL_DIR, is_virtual=True)
discover_cpp_classes(OBJECT_DIR)
discover_cpp_classes(SINGLETONS_DIR, is_singleton=True)

auto_file = os.path.join(GENERATED_DIR, "auto_register.gen.h")
with open(auto_file, "w") as f:
    f.write(f"""#pragma once
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
using namespace godot;

{includes}
{singleton_decls}
inline void auto_register_virtual_classes() {{
{virtual_class_registrations}}}

inline void auto_register_classes() {{
{class_registrations}}}

inline void auto_register_singletons() {{
{singleton_inits}}}

inline void auto_unregister_singletons() {{
{singleton_frees}}}
""")
print(f"Generated {auto_file}")

register_types_file = os.path.join(GAME_DIR, "register_types.cpp")
if os.path.exists(register_types_file):
    all_sources.append(register_types_file)
else:
    print("Warning: register_types.cpp not found!")

platform = env["platform"]
target = env["target"]
arch = env["arch"] if "arch" in env else "x86_64"

output_dir = "godot/addons/game/bin"

os.makedirs(output_dir, exist_ok=True)

lib_filename = "libgame.{}.{}.{}{}".format(
    platform, target, arch, env["SHLIBSUFFIX"]
)

lib = env.SharedLibrary(
    target=os.path.join(output_dir, lib_filename),
    source=all_sources
)

Default(lib)