{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
name = "godot-engine-dev";

buildInputs = with pkgs; [
# Среды
godot

# Основные инструменты
git
cmake
ninja
pkg-config
scons
python3
gdb
perf

# Компиляторы
gcc
clang
llvm

# Vulkan
vulkan-headers
vulkan-loader
vulkan-validation-layers

# X11
xorg.libX11
xorg.libXcursor
xorg.libXrandr
xorg.libXi
xorg.libXinerama
xorg.libXext
xorg.libXfixes

# Wayland
wayland
wayland-protocols

# ALSA (звук)
alsa-lib

# Прочее
libudev-zero
dbus
zlib
openssl
];

shellHook = ''
echo "Godot C++ development environment ready 🚀"
echo ""
echo "Available tools:"
echo " - scons (build godot)"
echo " - cmake / ninja (build game)"
'';
}
