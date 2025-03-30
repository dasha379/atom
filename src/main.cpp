#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include <string>

#include "parser.hpp"
#include "tokenization.hpp"
#include "generation.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "incorrect usage\n";
        std::cerr << "comp <input.at>\n";
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    if (contents.empty()) {
        std::cerr << "Error: Input file is empty\n";
        return EXIT_FAILURE;
    }
    
    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<nodeExit> tree = parser.parse();
    if (!tree.has_value()){
        std::cerr << "no exit statement found\n";
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.generate();
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");
    return EXIT_SUCCESS;
}