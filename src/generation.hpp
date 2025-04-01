#pragma once

#include "parser.hpp"

class Generator{
public:
    inline explicit Generator(nodeProg prog) : m_prog(std::move(prog)) {} //constructor:0

    void gen_expr(const nodeExpr& expr)
    {
        struct ExprVisitor{
            Generator* gen;

            void operator() (const nodeExprIdent& expr_ident){

            }
            void operator() (const nodeExprIntLit& expr_int_lit) const {
                gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << '\n';
                gen->m_output << "    push rax\n";
            }
        };
        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr.var);
    }

    void gen_stmt(const nodeStmt& stmt)
    {
        struct StmtVisitor{
            Generator* gen; // a pointer
            void operator() (const nodeStmtExit& stmt_exit) const {
                gen->gen_expr(stmt_exit.expr);
                gen->m_output << "    mov rax, 60\n";
                gen->m_output << "    pop rdi\n";
                gen->m_output << "    syscall\n";
            }
            void operator() (const nodeStmtLet& stmt_let){

            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] std::string gen_prog()
    {
        m_output << "global _start\n_start:\n";

        for (const nodeStmt& stmt : m_prog.stmts){
            gen_stmt(stmt);
        }

        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";
        return m_output.str();
    }
private:
    const nodeProg m_prog;
    std::stringstream m_output;
};