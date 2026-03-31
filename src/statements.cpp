#include "statements.h"
#include "expressions.h"
#include "lexer.h"

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
ScanStmt::ScanStmt(Variable name) : name(name) {}
FunDecl::FunDecl(Token name, std::vector<Token> param, std::vector<Stmt> body) : name(name), param(param), body(body) {}
BreakStmt::BreakStmt(Token tok) : tok(tok) {}
ContinueStmt::ContinueStmt(Token tok) : tok(tok) {}
ReturnStmt::ReturnStmt(Token tok, expr return_value) : tok(tok), return_value(return_value) {}

//InnerExprStmt::InnerExprStmt(Stmt stmt) : stmt(stmt) {}
//void InnerExprStmt::operator=(Stmt stmt) {this->stmt = stmt;}

Stmt null_stmt = ExprStmt(Literal(literal_t(_NULL)));
