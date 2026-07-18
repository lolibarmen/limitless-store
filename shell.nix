{ pkgs ? import <nixpkgs> {} }:


let
  mingw = pkgs.pkgsCross.mingwW64;
in
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

# Компилятор linux
clang

# Компилятор windows
mingw.buildPackages.gcc
mingw.windows.pthreads
mingw.windows.mcfgthreads

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

export CPATH="${mingw.windows.mcfgthreads.dev}/include:${mingw.windows.pthreads.dev or mingw.windows.pthreads}/include''${CPATH:+:$CPATH}"
    export LIBRARY_PATH="${mingw.windows.mcfgthreads}/lib:${mingw.windows.pthreads}/lib''${LIBRARY_PATH:+:$LIBRARY_PATH}"

'';
}
