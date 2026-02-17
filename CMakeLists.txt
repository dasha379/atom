cmake_minimum_required(VERSION 3.20)

project (atom)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(atom 
        src/main.cpp
        src/arena.cpp
        src/generation.cpp
        src/parser.cpp
        src/tokenization.cpp
        )

target_include_directories(atom PRIVATE src)

target_compile_options(atom PRIVATE -Wall -Wextra)


