#pragma once

#include "tokenization.hpp"

struct nodeExpr{
    Token int_lit;
};
struct nodeExit{
    nodeExpr expr;
};

class Parser{
public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

    std::optional<nodeExpr> parse_expr(){
        if (peek().has_value() && peek().value().type == TokenType::int_lit){
            return nodeExpr {.int_lit = consume()};
        } else return {};
    }

    std::optional<nodeExit> parse(){
        std::optional<nodeExit> exit_node;
        while(peek().has_value()){
            if (peek().value().type == TokenType::_exit){
                consume();
                if (auto node_expr = parse_expr()){
                    exit_node = nodeExit { .expr = node_expr.value() };
                } else {
                    std::cerr << "invalid expression\n";
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().type == TokenType::semi){
                    consume();
                } else{
                    std::cerr << "invalid expression\n";
                    exit(EXIT_FAILURE);
                }
            }

        }
        m_index = 0;
        return exit_node;
    }
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    [[nodiscard]] inline std::optional<Token> peek(int ahead = 1) const {
        if (m_index + ahead > m_tokens.size()) return {};
        else return m_tokens[m_index];
    }
    inline Token consume(){
        return m_tokens.at(m_index++);
    }
};