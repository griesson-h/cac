#include "statements.h"

PrintStmt::PrintStmt(expr exp) : exp(exp) {}
ExprStmt::ExprStmt(expr exp) : exp(exp) {}
Var::Var(Token name, expr initializer) : name(name), initializer(initializer) {}
Block::Block(std::vector<Stmt> stmts) : stmts(stmts) {}
IfStmt::IfStmt(expr condition, std::shared_ptr<Stmt> then, std::shared_ptr<Stmt> elsestmt) : condition(condition) {
  then.swap(this->then);
  elsestmt.swap(this->elsestmt);
}
While::While(expr condition, std::shared_ptr<Stmt> body) : condition(condition) {
  body.swap(this->body);
}

Stmt null_stmt = ExprStmt(Literal(literal_t(_NULL)));
