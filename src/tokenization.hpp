#pragma once
#include <string>
#include <vector>
#include <optional>
#include <iostream>

enum class TokenType{
    _exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    eqeq,
    plus,
    star,
    minus,
    fslash,
    open_curly,
    close_curly,
    if_,
    elif,
    else_,
};

std::optional<int> bin_prec(TokenType type);

std::string to_string(const TokenType type);

struct Token {
    TokenType type;
    std::optional<std::string> value;
    int line;
};

class Tokenizer{
    public:
        explicit Tokenizer(std::string src);

        std::vector<Token> tokenize();
    private:
        std::optional<char> peek(int offset = 0) const {
            if (m_index + offset >= m_src.length()) return {};
            else return m_src[m_index + offset];
        }
        const std::string m_src; 
        size_t m_index = 0;
        char consume(){
            return m_src.at(m_index++);
        }
};