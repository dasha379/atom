#pragma once

#include <variant>
#include "arena.hpp"
#include "tokenization.hpp"

struct nodeExpr;

struct nodeBinExprAdd{
    nodeExpr* lhs;
    nodeExpr* rhs;
};

/*struct nodeBinExprMulti{
    nodeExpr* lhs;
    nodeExpr* rhs;
};*/

struct nodeBinExpr{
    nodeBinExprAdd* add;
};

struct nodeTermIntLit{
    Token int_lit;
};

struct nodeTermIdent{
    Token ident;
};

struct nodeTerm{
    std::variant<nodeTermIntLit*, nodeTermIdent*> var;
};

struct nodeExpr{
    std::variant<nodeTerm*, nodeBinExpr*> var;
};

struct nodeStmtExit{
    nodeExpr* expr;
};
struct nodeStmtLet{
    Token ident;
    nodeExpr* expr{};
};
struct nodeStmt{
    std::variant<nodeStmtExit*, nodeStmtLet*> var;
};
struct nodeProg{
    std::vector<nodeStmt*> stmts;
};

class Parser{
public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)), m_allocator(1024*1024*4) //4 mb
    {}

    std::optional<nodeBinExpr*> parse_bin_expr(){
        if (auto lhs = parse_expr()){

        }
    }

    std::optional<nodeTerm*> parse_term(){
        if (peek().has_value() && peek().value().type == TokenType::int_lit){
            auto term_int_lit = m_allocator.alloc<nodeTermIntLit>();
            term_int_lit->int_lit = consume();
            auto term = m_allocator.alloc<nodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        else if (peek().has_value() && peek().value().type == TokenType::ident){
            auto term_ident = m_allocator.alloc<nodeTermIdent>();
            term_ident->ident = consume();
            auto term = m_allocator.alloc<nodeTerm>();
            term->var = term_ident;
            return term;
        } else return {};
    }

    std::optional<nodeExpr*> parse_expr(){

        if (auto term = parse_term()){
            if (peek().has_value() && peek().value().type == TokenType::plus){
                auto bin_expr = m_allocator.alloc<nodeBinExpr>();
                auto bin_expr_add = m_allocator.alloc<nodeBinExprAdd>();
                auto lhs_expr = m_allocator.alloc<nodeExpr>();
                lhs_expr->var = term.value();
                bin_expr_add->lhs = lhs_expr;
                consume();
                if (auto rhs = parse_expr()){
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->add = bin_expr_add;
                    auto expr = m_allocator.alloc<nodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                } else {
                    std::cerr << "expected expression\n";
                    exit(EXIT_FAILURE);
                }
            }
            else{
                auto expr = m_allocator.alloc<nodeExpr>();
                expr->var = term.value();
                return expr;
            }
        }
        else if (auto bin_expr = parse_bin_expr()){
            auto expr = m_allocator.alloc<nodeExpr>();
            expr->var = bin_expr.value();
            return expr;
        }
        else return {};
    }

    std::optional<nodeStmt*> parse_stmt(){
        while(peek().has_value()){
            if (peek().value().type == TokenType::_exit && peek(1).has_value() && peek(1).value().type == TokenType::open_paren){
                consume();
                consume();
                auto stmt_exit = m_allocator.alloc<nodeStmtExit>();
                if (auto node_expr = parse_expr()){
                    stmt_exit->expr = node_expr.value();
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
                auto stmt = m_allocator.alloc<nodeStmt>();
                stmt->var = stmt_exit;
                return stmt;
            }
            else if (peek().has_value() && peek().value().type == TokenType::let
                    && peek(1).has_value() && peek(1).value().type == TokenType::ident
                    && peek(2).has_value() && peek(2).value().type == TokenType::eq){
                consume();
                consume();
                auto stmt_let = m_allocator.alloc<nodeStmtLet>();
                if (auto expr = parse_expr()){
                    stmt_let->expr = expr.value();
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
                auto stmt = m_allocator.alloc<nodeStmt>();
                stmt->var = stmt_let;
                return stmt;
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
    ArenaAllocator m_allocator;
};