{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = [
    pkgs.platformio-core
  ];

  PLATFORMIO_CORE_DIR = toString ./. + "/.platformio";
}