#pragma once

#include <unordered_map>
#include <cassert>
#include "parser.hpp"

class Generator{
public:
    inline explicit Generator(nodeProg prog) : m_prog(std::move(prog)) {} //constructor:0

    void gen_term(const nodeTerm* term){
        struct TermVisitor{
            Generator* gen;
            void operator() (const nodeTermIntLit* term_int_lit) const{
                gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << '\n';
                gen->push("rax");
            }
            void operator() (const nodeTermIdent* term_ident) const{
                if (!gen->m_vars.contains(term_ident->ident.value.value())){
                    std::cerr << "undefined identifier: " << term_ident->ident.value.value() << '\n';
                    exit(EXIT_FAILURE);
                }
                const auto& var = gen->m_vars.at(term_ident->ident.value.value());
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }
            void operator() (const nodeTermParen* term_paren) const{
                gen->gen_expr(term_paren->expr);
            }
        };
        TermVisitor visitor{.gen = this};
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const nodeBinExpr* bin_expr)
    {
        struct ExprVisitor{
            Generator* gen;
            void operator() (const nodeBinExprSub* sub) const{
                gen->gen_expr(sub->rhs);
                gen->gen_expr(sub->lhs);
                gen->pop("rax");
                gen->m_output << "    sub rax, rbx\n";
                gen->push("rax");
            }
            void operator() (const nodeBinExprAdd* add) const{
                gen->gen_expr(add->rhs);
                gen->gen_expr(add->lhs);
                gen->pop("rax");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }
            void operator() (const nodeBinExprMulti* multi) const{
                gen->gen_expr(multi->rhs);
                gen->gen_expr(multi->lhs);
                gen->pop("rax");
                gen->m_output << "    mul rbx\n";
                gen->push("rax");
            }
            void operator() (const nodeBinExprDiv* div) const{
                gen->gen_expr(div->rhs);
                gen->gen_expr(div->lhs);
                gen->pop("rax");
                gen->m_output << "    div rbx\n";
                gen->push("rax");
            }
        };
        ExprVisitor visitor{.gen = this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const nodeExpr* expr)
    {
        struct ExprVisitor{
            Generator* gen;
            void operator() (const nodeTerm* term){
                gen->gen_term(term);
            }
            void operator() (const nodeBinExpr* bin_expr) const{
                gen->gen_bin_expr(bin_expr);
            }
        };
        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const nodeStmt* stmt)
    {
        struct StmtVisitor{
            Generator* gen; // a pointer
            void operator() (const nodeStmtExit* stmt_exit) const {
                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }
            void operator() (const nodeStmtLet* stmt_let){
                if (gen->m_vars.contains(stmt_let->ident.value.value())) {
                    std::cerr << "identifier already used: " << stmt_let->ident.value.value() << '\n';
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({stmt_let->ident.value.value(), Var {.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stmt_let->expr);
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string gen_prog()
    {
        m_output << "global _start\n_start:\n";

        for (const nodeStmt* stmt : m_prog.stmts){
            gen_stmt(stmt);
        }

        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";
        return m_output.str();
    }
private:
    void push(const std::string& reg){
        m_output << "    push " << reg << '\n';
        m_stack_size++;
    }

    void pop(const std::string& reg){
        m_output << "    pop " << reg << '\n';
        m_stack_size--;
    }

    struct Var{
        size_t stack_loc;
    };

    const nodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars;
};