{ pkgs ? import <nixpkgs> {} }:

let
  mingw = pkgs.pkgsCross.mingwW64;
in
pkgs.mkShell {
  name = "godot-engine-dev";

  buildInputs = with pkgs; [
    godot
    git cmake ninja pkg-config scons python3 gdb perf
    gcc clang
    clang-tools  # ← ДОБАВЬТЕ ЭТО! Содержит clangd
    mingw.buildPackages.gcc
    mingw.windows.pthreads
    mingw.windows.mcfgthreads
    vulkan-headers vulkan-loader vulkan-validation-layers
    xorg.libX11 xorg.libXcursor xorg.libXrandr xorg.libXi xorg.libXinerama xorg.libXext xorg.libXfixes
    wayland wayland-protocols
    alsa-lib libudev-zero dbus zlib openssl
  ];

  shellHook = ''
    echo "Godot C++ development environment ready 🚀"
    echo " - scons (build godot)"
    echo " - cmake / ninja (build game)"
    # НЕ используйте exec fish здесь!
  '';
}