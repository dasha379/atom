#pragma once

#include <variant>
#include <iostream>
#include "arena.hpp"
#include "tokenization.hpp"
#include <string>

struct nodeExpr;
struct nodeTermParen;

struct nodeBinExprAdd{
    nodeExpr* lhs;
    nodeExpr* rhs;
};

struct nodeBinExprMulti{
    nodeExpr* lhs;
    nodeExpr* rhs;
};

struct nodeBinExprSub{
    nodeExpr* lhs;
    nodeExpr* rhs;
};

struct nodeBinExprDiv{
    nodeExpr* lhs;
    nodeExpr* rhs;
};

struct nodeBinExprEq{
    nodeExpr* lhs;
    nodeExpr* rhs;
};

struct nodeBinExpr{
    std::variant<nodeBinExprEq*, nodeBinExprAdd*, nodeBinExprMulti*, nodeBinExprDiv*, nodeBinExprSub*> var;
};

struct nodeTermIntLit{
    Token int_lit;
};

struct nodeTermIdent{
    Token ident;
};

struct nodeTerm{
    std::variant<nodeTermIntLit*, nodeTermIdent*, nodeTermParen*> var;
};

struct nodeExpr{
    std::variant<nodeTerm*, nodeBinExpr*> var;
};

struct nodeTermParen{
    nodeExpr* expr;
};

struct nodeStmtExit{
    nodeExpr* expr;
};
struct nodeStmtLet{
    Token ident;
    nodeExpr* expr{};
};

struct nodeStmt;
struct nodeIfPred;
struct nodeScope{
    std::vector<nodeStmt*> stmts;
};
struct nodeStmtIf{
    nodeExpr* expr;
    nodeScope* scope;
    std::optional<nodeIfPred*> pred;
};
struct nodeIfPredElif{
    nodeExpr* expr;
    nodeScope* scope;
    std::optional<nodeIfPred*> pred;
};
struct nodeIfPredElse{
    nodeScope* scope;
};
struct nodeIfPred{
    std::variant<nodeIfPredElif*, nodeIfPredElse*> var;
};
struct nodeStmtAssign{
    Token ident;
    nodeExpr* expr {};
};

struct nodeStmt{
    std::variant<nodeStmtExit*, nodeStmtLet*, nodeScope*, nodeStmtIf*, nodeStmtAssign*> var;
};
struct nodeProg{
    std::vector<nodeStmt*> stmts;
};

class Parser{
public:
    explicit Parser(std::vector<Token> tokens);

    void error_expected(const std::string& msg) const;

    std::optional<nodeTerm*> parse_term();

    std::optional<nodeExpr*> parse_expr(int min_prec);

    std::optional<nodeScope*> parse_scope();

    std::optional<nodeIfPred*> parse_if_pred();

    std::optional<nodeStmt*> parse_stmt();

    std::optional<nodeProg> parse_prog();

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) return {};
        return m_tokens[m_index + offset];
    }
    inline Token consume(){
        return m_tokens.at(m_index++);
    }
    Token try_consume_err(const TokenType type)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(to_string(type));
        return {};
    }

    std::optional<Token> try_consume(const TokenType type)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        return {};
    }
    ArenaAllocator m_allocator;
};