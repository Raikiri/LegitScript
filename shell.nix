with import <nixpkgs> {};

mkShell {
  name = "legit script stuff";
  packages = [
    cmake
    clang
    lldb
    gnumake
  ];

  buildInputs = with pkgs; [
  ];
}
