with import <nixpkgs> {};
#note: don't forget to delete the "build" directory after updating the compiler, otherwise it keeps some paths cached
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
