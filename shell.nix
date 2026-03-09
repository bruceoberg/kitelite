{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = [
    pkgs.platformio-core
    pkgs.claude-code
  ];

  PLATFORMIO_CORE_DIR = toString ./. + "/.platformio";
}