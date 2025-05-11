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
    elif,
    else_,
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

inline std::string to_string(const TokenType type){
    switch(type){
        case TokenType::_exit:
            return "exit";
        case TokenType::int_lit:
            return "integer literal";
        case TokenType::semi:
            return ";";
        case TokenType::open_paren:
            return "(";
        case TokenType::close_paren:
            return ")";
        case TokenType::ident:
            return "identifier";
        case TokenType::let:
            return "let";
        case TokenType::eq:
            return "=";
        case TokenType::plus:
            return "+";
        case TokenType::star:
            return "*";
        case TokenType::minus:
            return "-";
        case TokenType::fslash:
            return "/";
        case TokenType::open_curly:
            return "{";
        case TokenType::close_curly:
            return "}";
        case TokenType::if_:
            return "if";
        case TokenType::elif:
            return "elif";
        case TokenType::else_:
            return "else";
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value;
    int line;
};

class Tokenizer{
    public:
        inline explicit Tokenizer(std::string src) : m_src(std::move(src)){} //constructor:)

        int line_count = 1;
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
                        tokens.push_back({.type = TokenType::_exit, .line = line_count});
                        buf.clear();
                    }
                    else if (buf == "let"){
                        tokens.push_back({.type = TokenType::let, .line = line_count});
                        buf.clear();
                    }
                    else if (buf == "if"){
                        tokens.push_back({.type = TokenType::if_, .line = line_count});
                        buf.clear();
                    }
                    else if (buf == "elif"){
                        tokens.push_back({.type = TokenType::elif, .line = line_count});
                        buf.clear();
                    }
                    else if (buf == "else"){
                        tokens.push_back({.type = TokenType::else_, .line = line_count});
                        buf.clear();
                    }
                    else {
                        tokens.push_back({.type = TokenType::ident, .value = buf, .line = line_count});
                        buf.clear();
                    }
                }
                else if (std::isdigit(peek().value())) {
                    buf.push_back(consume());
                    while(peek().has_value() && std::isdigit(peek().value()))
                        buf.push_back(consume());
                    tokens.push_back({.type = TokenType::int_lit, .value = buf, .line = line_count});
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
                    tokens.push_back({.type = TokenType::open_paren, .line = line_count});
                }
                else if (peek().value() == ')'){
                    consume();
                    tokens.push_back({.type = TokenType::close_paren, .line = line_count});
                }
                else if (peek().value() == ';'){
                    consume();
                    tokens.push_back({.type = TokenType::semi, .line = line_count});
                    continue;
                }
                else if (peek().value() == '='){
                    consume();
                    tokens.push_back({.type = TokenType::eq, .line = line_count});
                }
                else if (peek().value() == '+'){
                    consume();
                    tokens.push_back({.type = TokenType::plus, .line = line_count});
                }
                else if (peek().value() == '*'){
                    consume();
                    tokens.push_back({.type = TokenType::star, .line = line_count});
                }
                else if (peek().value() == '-'){
                    consume();
                    tokens.push_back({.type = TokenType::minus, .line = line_count});
                }
                else if (peek().value() == '/'){
                    consume();
                    tokens.push_back({.type = TokenType::fslash, .line = line_count});
                }
                else if (peek().value() == '{'){
                    consume();
                    tokens.push_back({.type = TokenType::open_curly, .line = line_count});
                }
                else if (peek().value() == '}'){
                    consume();
                    tokens.push_back({.type = TokenType::close_curly, .line = line_count});
                }
                else if (peek().value() == '\n'){
                    consume();
                    line_count++;
                }
                else if (std::isspace(peek().value())){
                    consume();
                }
                else {
                    std::cerr << "invalid token\n";
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