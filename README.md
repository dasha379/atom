# Atom C++ Compiler

Atom is my test programming language. It is really simple. 

## Сборка


```bash
git clone https://github.com/dasha379/atom
cd atom
mkdir build
cmake -S . -B build
cmake --build build
```

## Запуск
```bash
cd build
./atom ../test.at
./out
echo $?
```

## Структура

**src/tokenization.hpp** -- отвечает за лексический анализ. Текст преобразуется в токены. \
**src/parser.hpp** -- отвечает за синтаксческий анализ, построение синтаксического дерева. \
**src/arena.hpp** -- отвечает за управление памятью. Выделяется большой блок памяти и далее по мере необходимости выделяются более мелкие кусочки памяти для синтаксческого дерева. \
**src/generation.hpp** -- отвечает за генерацию кода ассемблера. Синтаксическое дерево обходится сверху вниз: (для выражения let x = 5 + 3)

```mermaid
graph TD
    Prog[nodeProg<br/>(корень)] --> Stmt[nodeStmt<br/>(let)]
    Stmt --> StmtLet[nodeStmtLet]
    StmtLet --> Ident[ident<br/>"x"]
    StmtLet --> Expr[nodeExpr]
    Expr --> BinExpr[nodeBinExpr<br/>(сложение)]
    BinExpr --> Term1[nodeTerm]
    BinExpr --> Term2[nodeTerm]
    Term1 --> Int1[nodeTermIntLit<br/>5]
    Term2 --> Int2[nodeTermIntLit<br/>3]
```
