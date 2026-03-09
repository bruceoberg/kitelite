{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = [
    pkgs.platformio-core
    pkgs.claude-code
    pkgs.repomix
  ];

  PLATFORMIO_CORE_DIR = toString ./. + "/.platformio";
}
