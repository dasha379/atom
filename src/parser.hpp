#pragma once

#include <variant>
#include "tokenization.hpp"

struct nodeExprIntLit{
    Token int_lit;
};
struct nodeExprIdent{
    Token ident;
};
struct nodeExpr{
    std::variant<nodeExprIdent, nodeExprIntLit> var;
};
struct nodeStmtExit{
    nodeExpr expr;
};
struct nodeStmtLet{
    Token ident;
    nodeExpr expr;
};
struct nodeStmt{
    std::variant<nodeStmtExit, nodeStmtLet> var;
};
struct nodeProg{
    std::vector<nodeStmt> stmts;
};

class Parser{
public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

    std::optional<nodeExpr> parse_expr(){
        if (peek().has_value() && peek().value().type == TokenType::int_lit){
            return nodeExpr {.var = nodeExprIntLit {.int_lit = consume()}};
        }
        else if (peek().has_value() && peek().value().type == TokenType::ident){
            return nodeExpr {.var = nodeExprIdent {.ident = consume()}};
        }
        else return {};
    }

    std::optional<nodeStmt> parse_stmt(){
        while(peek().has_value()){
            if (peek().value().type == TokenType::_exit && peek(1).has_value() && peek(1).value().type == TokenType::open_paren){
                consume();
                consume();
                nodeStmtExit stmt_exit;
                if (auto node_expr = parse_expr()){
                    stmt_exit = { .expr = node_expr.value() };
                } else {
                    std::cerr << "invalid expression\n";
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().type == TokenType::close_paren){
                    consume();
                } else {
                    std::cerr << "expected ')'\n";
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().type == TokenType::semi){
                    consume();
                } else{
                    std::cerr << "expected ';'\n";
                    exit(EXIT_FAILURE);
                }
                return nodeStmt {.var = stmt_exit};
            }
            else if (peek().has_value() && peek().value().type == TokenType::let
                    && peek(1).has_value() && peek(1).value().type == TokenType::ident
                    && peek(2).has_value() && peek(2).value().type == TokenType::eq){
                consume();
                auto stmt_let = nodeStmtLet {.ident = consume()};
                consume();
                if (auto expr = parse_expr()){
                    stmt_let.expr = expr.value();
                } else {
                    std::cerr << "invalid expression\n";
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().type == TokenType::semi) {
                    consume();
                } else {
                    std::cerr << "invalid ';'\n";
                    exit(EXIT_FAILURE);
                }
                return nodeStmt {.var = stmt_let};
            }
            else return {};
        }
    }

    std::optional<nodeProg> parse_prog(){
        nodeProg prog;
        while(peek().has_value()){
            if (auto stmt = parse_stmt()){
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "invalid statement\n";
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) return {};
        else return m_tokens[m_index + offset];
    }
    inline Token consume(){
        return m_tokens.at(m_index++);
    }
};