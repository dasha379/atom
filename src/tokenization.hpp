#pragma once
#include <string>
#include <vector>
#include <optional>

enum class TokenType{
    _exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    plus,
    star,
    minus,
    fslash,
    open_curly,
    close_curly,
    if_,
};

std::optional<int> bin_prec(TokenType type){
    switch(type){
        case TokenType::minus:
        case TokenType::plus:
            return 0;
        case TokenType::fslash:
        case TokenType::star:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

class Tokenizer{
    public:
        inline explicit Tokenizer(std::string src) : m_src(std::move(src)){} //constructor:)

        inline std::vector<Token> tokenize(){
            std::string buf;
            std::vector<Token> tokens;
            while(peek().has_value()){
                if(std::isalpha(peek().value())){
                    buf.push_back(consume());
                    while(peek().has_value() && std::isalnum(peek().value())){
                        buf.push_back(consume());
                    }
                    if (buf == "exit"){
                        tokens.push_back({.type = TokenType::_exit});
                        buf.clear();
                    }
                    else if (buf == "let"){
                        tokens.push_back({.type = TokenType::let});
                        buf.clear();
                    }
                    else if (buf == "if"){
                        tokens.push_back({.type = TokenType::if_});
                        buf.clear();
                    }
                    else {
                        tokens.push_back({.type = TokenType::ident, .value = buf});
                        buf.clear();
                    }
                }
                else if (std::isdigit(peek().value())) {
                    buf.push_back(consume());
                    while(peek().has_value() && std::isdigit(peek().value()))
                        buf.push_back(consume());
                    tokens.push_back({.type = TokenType::int_lit, .value = buf});
                    buf.clear();
                    continue;
                }
                // comments (one line)
                else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/'){
                    consume();
                    consume();
                    while (peek().has_value() && peek().value() != '\n'){
                        consume();
                    }
                }
                // multi-line comments /* ................ */
                else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*'){
                    consume();
                    consume();
                    while (peek().has_value()){
                        if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') break;
                        consume();
                    }   
                    if (peek().has_value()) consume();
                    if (peek().has_value()) consume();
                }
                else if (peek().value() == '('){
                    consume();
                    tokens.push_back({.type = TokenType::open_paren});
                }
                else if (peek().value() == ')'){
                    consume();
                    tokens.push_back({.type = TokenType::close_paren});
                }
                else if (peek().value() == ';'){
                    consume();
                    tokens.push_back({.type = TokenType::semi});
                    continue;
                }
                else if (peek().value() == '='){
                    consume();
                    tokens.push_back({.type = TokenType::eq});
                }
                else if (peek().value() == '+'){
                    consume();
                    tokens.push_back({.type = TokenType::plus});
                }
                else if (peek().value() == '*'){
                    consume();
                    tokens.push_back({.type = TokenType::star});
                }
                else if (peek().value() == '-'){
                    consume();
                    tokens.push_back({.type = TokenType::minus});
                }
                else if (peek().value() == '/'){
                    consume();
                    tokens.push_back({.type = TokenType::fslash});
                }
                else if (peek().value() == '{'){
                    consume();
                    tokens.push_back({.type = TokenType::open_curly});
                }
                else if (peek().value() == '}'){
                    consume();
                    tokens.push_back({.type = TokenType::close_curly});
                }
                else if (std::isspace(peek().value())){
                    consume();
                }
                else {
                    std::cerr << "you messed up!" << '\n';
                    exit(EXIT_FAILURE);
                }
            }
            m_index = 0;
            return tokens;
        }
    private:
        [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
            if (m_index + offset >= m_src.length()) return {};
            else return m_src[m_index + offset];
        }
        const std::string m_src; 
        size_t m_index = 0;
        inline char consume(){
            return m_src.at(m_index++);
        }
};