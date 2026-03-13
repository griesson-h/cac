#include "statements.h"

PrintStmt::PrintStmt(expr exp) : exp(exp) {}
ExprStmt::ExprStmt(expr exp) : exp(exp) {}
Var::Var(Token name, expr initializer) : name(name), initializer(initializer) {}
Block::Block(std::vector<Stmt> stmts) : stmts(stmts) {}
