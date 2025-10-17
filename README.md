# Atom C++ Compiler

Atom is my test programming language. It is really simple. 

## Building


```bash
git clone https://github.com/dasha379/atom
cd atom
mkdir build
cmake -S . -B build
cmake --build build
```

## Structure

**src/tokenization.hpp** is a file providing a lexical analysis. Lexer takes a stream of characters as input and produces a stream of tokens as output. \
**src/parser.hpp** provides a syntax analysis - analyzes the syntactic structure of the given token stream to verify whether the source code was syntactically correct according to the rules of the source language, and produces a syntax tree corresponding to the source code.
