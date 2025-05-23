#pragma once

#include <vector>
#include <algorithm>
#include <cassert>
#include "parser.hpp"

class Generator{
public:
    inline explicit Generator(nodeProg prog) : m_prog(std::move(prog)) {} //constructor:0

    void gen_term(const nodeTerm* term){
        struct TermVisitor{
            Generator& gen;
            void operator() (const nodeTermIntLit* term_int_lit) const{
                gen.m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << '\n';
                gen.push("rax");
            }
            void operator() (const nodeTermIdent* term_ident) const{
                auto it = std::find_if(
                        gen.m_vars.cbegin(),
                        gen.m_vars.cend(),
                        [&](const Var& var) { return var.name == term_ident->ident.value.value(); }); // it - iterator
                if (it == gen.m_vars.cend()) {
                    std::cerr << "undefined identifier: " << term_ident->ident.value.value() << '\n';
                    exit(EXIT_FAILURE);
                }
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen.m_stack_size - (*it).stack_loc - 1) * 8 << "]";
                gen.push(offset.str());
            }
            void operator() (const nodeTermParen* term_paren) const{
                gen.gen_expr(term_paren->expr);
            }
        };
        TermVisitor visitor{.gen = *this};
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const nodeBinExpr* bin_expr)
    {
        struct ExprVisitor{
            Generator& gen;
            void operator() (const nodeBinExprSub* sub) const{
                gen.gen_expr(sub->rhs);
                gen.gen_expr(sub->lhs);
                gen.pop("rax");
                gen.m_output << "    sub rax, rbx\n";
                gen.push("rax");
            }
            void operator() (const nodeBinExprAdd* add) const{
                gen.gen_expr(add->rhs);
                gen.gen_expr(add->lhs);
                gen.pop("rax");
                gen.m_output << "    add rax, rbx\n";
                gen.push("rax");
            }
            void operator() (const nodeBinExprMulti* multi) const{
                gen.gen_expr(multi->rhs);
                gen.gen_expr(multi->lhs);
                gen.pop("rax");
                gen.m_output << "    mul rbx\n";
                gen.push("rax");
            }
            void operator() (const nodeBinExprDiv* div) const{
                gen.gen_expr(div->rhs);
                gen.gen_expr(div->lhs);
                gen.pop("rax");
                gen.m_output << "    div rbx\n";
                gen.push("rax");
            }
        };
        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const nodeExpr* expr)
    {
        struct ExprVisitor{
            Generator& gen;
            void operator() (const nodeTerm* term){
                gen.gen_term(term);
            }
            void operator() (const nodeBinExpr* bin_expr) const{
                gen.gen_bin_expr(bin_expr);
            }
        };
        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);
    }

    void gen_scope(const nodeScope* scope){
        begin_scope();
        for (const nodeStmt* stmt: scope->stmts){
            gen_stmt(stmt);
        }
        end_scope();
    }

    void gen_if_pred(const nodeIfPred* pred, const std::string& end_label){  // end_label is a place where we should "jump" when we pass through if, elif and else
        struct predVisitor{
            Generator& gen;
            const std::string& end_label;
            void operator() (const nodeIfPredElif* elif) const {
                gen.gen_expr(elif->expr);
                gen.pop("rax");
                std::string label = gen.create_label();
                gen.m_output << "    test rax, rax\n";
                gen.m_output << "    jz" << label << '\n';
                gen.gen_scope(elif->scope);
                gen.m_output << "    jmp " << end_label << '\n';
                if (elif->pred.has_value()){
                    gen.m_output << label << ':\n';
                    gen.gen_if_pred(elif->pred.value(), end_label);
                }
            }
            void operator() (const nodeIfPredElse* else_) const {
                gen.gen_scope(else_->scope);
            }
        };
        predVisitor visitor{.gen = *this, .end_label = end_label};
        std::visit(visitor, pred->var);
    }

    void gen_stmt(const nodeStmt* stmt)
    {
        struct StmtVisitor{
            Generator& gen; // a pointer
            void operator() (const nodeStmtExit* stmt_exit) const { // first const - we cant change the input pointer to the object, the second const - we cant change the object which method we are calling (in this case it is gen i guess)
                gen.gen_expr(stmt_exit->expr);
                gen.m_output << "    mov rax, 60\n";
                gen.pop("rdi");
                gen.m_output << "    syscall\n";
            }
            void operator() (const nodeStmtLet* stmt_let){
                auto it = std::find_if(
                        gen.m_vars.cbegin(),
                        gen.m_vars.cend(),
                        [&](const Var& var) { return var.name == stmt_let->ident.value.value(); });
                if (it != gen.m_vars.cend()) {
                    std::cerr << "identifier already used: " << stmt_let->ident.value.value() << '\n';
                    exit(EXIT_FAILURE);
                }
                gen.m_vars.push_back({ .name = stmt_let->ident.value.value(), .stack_loc = gen.m_stack_size });
                gen.gen_expr(stmt_let->expr);
            }

            void operator() (const nodeStmtAssign* assign) const {
                auto it = std::ranges::find_if(gen.m_vars, [&](const Var& var){
                    return (var.name == assign->ident.value.value());
                });
                if (it == gen.m_vars.cend()){
                    std::cerr << "undefined variable: " << assign->ident.value.value() << '\n';
                    exit(EXIT_FAILURE);
                }
                gen.gen_expr(assign->expr);
                gen.pop("rax");
                gen.m_output << "    mov [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "], rax\n";

            }

            void operator() (const nodeScope* scope) const{
                gen.gen_scope(scope);
            }
            void operator() (const nodeStmtIf* stmt_if) const{
                gen.gen_expr(stmt_if->expr);
                gen.pop("rax");
                std::string label = gen.create_label();
                gen.m_output << "    test rax, rax\n";
                gen.m_output << "    jz " << label << '\n';
                gen.gen_scope(stmt_if->scope);
                std::optional<std::string> end_label;
                if (stmt_if->pred.has_value()) { // если после else есть еще elif etc то мы их перепрыгиваем в силу того что первый else выполняется
                    end_label = gen.create_label();
                    gen.m_output << "    jmp " << end_label.value() << '\n';
                    gen.m_output << label << ':\n';
                    gen.gen_if_pred(stmt_if->pred.value(), end_label.value());
                    gen.m_output << end_label.value() << ':\n';
                }
                else{
                    gen.m_output << label << '\n';
                }
                gen.m_output << "    ;; /if\n";
            }
        };

        StmtVisitor visitor{.gen = *this};
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

    void begin_scope(){
        m_scopes.push_back(m_vars.size());
    }

    void end_scope(){
        size_t pop_count = m_vars.size() - m_scopes.back();
        m_output << "    add rsp, " << pop_count * 8 << '\n';
        m_stack_size -= pop_count;
        for (size_t i = 0; i < pop_count; ++i) m_vars.pop_back();
        m_scopes.pop_back();
    }

    std::string create_label(){
        std::stringstream ss;
        ss << "label" << m_label_count++;
        return ss.str();
    }

    struct Var{
        std::string name;
        size_t stack_loc;
    };

    const nodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::vector<Var> m_vars {};
    std::vector<size_t> m_scopes {};
    int m_label_count = 0;
};