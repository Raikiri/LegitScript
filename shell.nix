with import <nixpkgs> {};

mkShell {
  name = "legit script stuff";
  packages = [
    cmake
    clang
    lldb
    gnumake
    emscripten
    python3
    static-web-server
  ];

  buildInputs = with pkgs; [
  ];
}
