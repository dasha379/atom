#pragma once

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include "parser.hpp"

class Generator{
public:
    explicit Generator(nodeProg prog);

    void gen_term(const nodeTerm* term);

    void gen_bin_expr(const nodeBinExpr* bin_expr);

    void gen_expr(const nodeExpr* expr);

    void gen_scope(const nodeScope* scope);

    void gen_if_pred(const nodeIfPred* pred, const std::string& end_label);

    void gen_stmt(const nodeStmt* stmt);

    std::string gen_prog();
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