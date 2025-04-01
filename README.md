# Atom C++ Compiler.

Atom is my test programming language. It is really simple. 
This compiler is written in C++.

## Building.

Requires `nasm` and `ld` on Linux operating system.

```bash
git clone https://github.com/dasha379/atom
cd atom
mkdir build
cmake -S . -B build
cmake --build build
```

comp is an executable file in the ./build directory.
