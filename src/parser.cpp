#include "parser.hpp"

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)), m_allocator(1024*1024*4){}

void Parser::error_expected(const std::string& msg) const{
  std::cerr << "[Parse Error] Expected " << msg << " on line " << peek(-1).value().line << std::endl;
  exit(EXIT_FAILURE);
}

std::optional<nodeTerm*> Parser::parse_term(){
    if (!peek().has_value()) {
        std::cerr << "parse_term: no token to parse" << std::endl;
        return {};
    }

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
    }
    else if (peek().has_value() && peek().value().type == TokenType::open_paren){
        consume();

        const auto expr = parse_expr(0);
        if (!expr.has_value()){
            error_expected("expression");
        }
        if (!(peek().has_value() && peek().value().type == TokenType::close_paren)) {
            error_expected("')'");
        }
        consume();

        auto term_paren = m_allocator.alloc<nodeTermParen>();
        term_paren->expr = expr.value();
        auto term = m_allocator.alloc<nodeTerm>();
        term->var = term_paren;
        return term;

    }
    else return {};
}

std::optional<nodeExpr*> Parser::parse_expr(int min_prec = 0){

    std::optional<nodeTerm*> term_lhs = parse_term();
    if (!term_lhs.has_value()) return {};

    auto expr_lhs = m_allocator.alloc<nodeExpr>();
    expr_lhs->var = term_lhs.value();

    while (true){
        std::optional<Token> curr_token = peek();

        if (!curr_token.has_value() || curr_token->type == TokenType::close_paren || curr_token->type == TokenType::close_curly || curr_token->type == TokenType::semi) {
            break;
        }

        std::optional<int> prec = bin_prec(curr_token->type);
        if (prec < min_prec) break;

        Token op = consume();
        int next_min_prec = prec.value() + 1;
        auto expr_rhs = parse_expr(next_min_prec);
        if (!expr_rhs.has_value()){
            std::cerr << "unable to parse an expression\n";
            exit(EXIT_FAILURE);
        }

        auto expr = m_allocator.alloc<nodeBinExpr>();
        auto expr_lhs2 = m_allocator.alloc<nodeExpr>();

        if (op.type == TokenType::plus){
            auto add = m_allocator.alloc<nodeBinExprAdd>();
            expr_lhs2->var = expr_lhs->var;
            add->lhs = expr_lhs2;
            add->rhs = expr_rhs.value();
            expr->var = add;
        } else if (op.type == TokenType::star){
            auto multi = m_allocator.alloc<nodeBinExprMulti>();
            expr_lhs2->var = expr_lhs->var;
            multi->lhs = expr_lhs2;
            multi->rhs = expr_rhs.value();
            expr->var = multi;
        } else if (op.type == TokenType::minus){
            auto sub = m_allocator.alloc<nodeBinExprSub>();
            expr_lhs2->var = expr_lhs->var;
            sub->lhs = expr_lhs2;
            sub->rhs = expr_rhs.value();
            expr->var = sub;
        } else if (op.type == TokenType::fslash){
            auto div = m_allocator.alloc<nodeBinExprDiv>();
            expr_lhs2->var = expr_lhs->var;
            div->lhs = expr_lhs2;
            div->rhs = expr_rhs.value();
            expr->var = div;
        } else if (op.type == TokenType::eqeq){
            auto eq_expr = m_allocator.alloc<nodeBinExprEq>();
            expr_lhs2->var = expr_lhs->var;
            eq_expr->lhs = expr_lhs2;
            eq_expr->rhs = expr_rhs.value();
            expr->var = eq_expr;
        }

        expr_lhs->var = expr;
    }
    return expr_lhs;
}

std::optional<nodeScope*> Parser::parse_scope(){
    auto scope = m_allocator.alloc<nodeScope>();
    while (auto stmt = parse_stmt()){
        scope->stmts.push_back(stmt.value());
    }
    if (peek().has_value() && peek().value().type == TokenType::close_curly){
        consume();
    } else {
        std::cerr << "expected '}'\n";
        exit(EXIT_FAILURE);
    }
    return scope;
}

std::optional<nodeIfPred*> Parser::parse_if_pred(){
    if (try_consume(TokenType::elif)){
        try_consume(TokenType::open_paren);
        const auto elif = m_allocator.alloc<nodeIfPredElif>();
        if (const auto expr = parse_expr()){
            elif->expr = expr.value();
        } else{
            std::cerr << "expected expression\n";
            exit(EXIT_FAILURE);
        }
        try_consume(TokenType::close_paren);
        if (const auto scope = parse_scope()){
            elif->scope = scope.value();
        } else {
            std::cerr << "expected scope\n";
            exit(EXIT_FAILURE);
        }
        elif->pred = parse_if_pred();
        auto pred = m_allocator.alloc<nodeIfPred>();
        pred->var = elif;
        return pred;
    }
    if (try_consume(TokenType::else_)){
        const auto else_pred = m_allocator.alloc<nodeIfPredElse>();
        if (const auto scope = parse_scope()){
            else_pred->scope = scope.value();
        } else {
            std::cerr << "expected scope\n";
            exit(EXIT_FAILURE);
        }
        auto pred = m_allocator.alloc<nodeIfPred>();
        pred->var = else_pred;
        return pred;
    }
    return {};
}

std::optional<nodeStmt*> Parser::parse_stmt(){
    while(peek().has_value()){

        // ======= возвращаем значение =========

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

        // ======= объявление переменных =======

        else if (peek().has_value() && peek().value().type == TokenType::let
                && peek(1).has_value() && peek(1).value().type == TokenType::ident
                && peek(2).has_value() && peek(2).value().type == TokenType::eq){
            consume();

            auto stmt_let = m_allocator.alloc<nodeStmtLet>();
            stmt_let->ident = consume();

            consume();

            if (auto n_expr = parse_expr()){
                stmt_let->expr = n_expr.value();
            } else {
                std::cerr << "invalid expression\n";
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                std::cerr << "expected ';'\n";
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.alloc<nodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }

        // ======= переприсваивание =======

        if (peek().has_value() && peek().value().type == TokenType::ident && peek(1).has_value() && peek(1).value().type == TokenType::eq){
            const auto assign = m_allocator.alloc<nodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (const auto expr = parse_expr()){
                assign->expr = expr.value();
            } else {
                std::cerr << "expected expression\n";
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi);
            auto stmt = m_allocator.alloc<nodeStmt>();
            stmt->var = assign;
            return stmt;
        }

        // ======= { блок } =======

        else if (peek().has_value() && peek().value().type == TokenType::open_curly){
            consume();
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.alloc<nodeStmt>();
                stmt->var = scope.value();
                return stmt;
            } else{
                std::cerr << "invalid scope'\n";
                exit(EXIT_FAILURE);
            }
        }

        // ======= условие =======

        else if (peek().has_value() && peek().value().type == TokenType::if_){
            consume();

            if (peek().has_value() && peek().value().type == TokenType::open_paren){
                consume();
            } else {
                std::cerr << "expected '('\n";
                exit(EXIT_FAILURE);
            }

            auto stmt_if = m_allocator.alloc<nodeStmtIf>();

            if (auto expr = parse_expr()){
                stmt_if->expr = expr.value();
            } else {
                std::cerr << "expected condition\n";
                exit(EXIT_FAILURE);
            }

            if (!peek().has_value() || peek().value().type != TokenType::close_paren){
                std::cerr << "expected ')', got " << (peek().has_value() ? to_string(peek()->type) : "EOF") << std::endl;
                exit(EXIT_FAILURE);
            }
            consume();

            if (peek().has_value() && peek().value().type == TokenType::open_curly) {
                consume();
            } else {
                std::cerr << "expected '{' after if condition\n";
                exit(EXIT_FAILURE);
            }
            if (auto scope = parse_scope()){
                stmt_if->scope = scope.value();
            } else {
                std::cerr << "invalid scope\n";
                exit(EXIT_FAILURE);
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.alloc<nodeStmt>();
            stmt->var = stmt_if;
            return stmt;
        }
        else return {};
    }

    return {};
}

std::optional<nodeProg> Parser::parse_prog(){
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