#pragma once
#include "expressions.h"
#include "lexer.h"
#include <memory>
#include <variant>
#include <vector>

struct ExprStmt;
struct PrintStmt;
struct Var;
struct Block;
struct IfStmt;
struct While;
struct ScanStmt;
struct FunDecl;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;


using Stmt = std::variant<std::monostate, ExprStmt, PrintStmt, Var, Block, IfStmt, While, ScanStmt, FunDecl, ReturnStmt, BreakStmt, ContinueStmt>;


struct ExprStmt {
  ExprStmt(expr exp);
  expr exp;
};
struct PrintStmt {
  PrintStmt(expr exp);
  expr exp;
};
struct Var {
  Var(Token name, expr initializer);
  Token name;
  expr initializer;
};
struct Block {
  Block(std::vector<Stmt> stmts);
  std::vector<Stmt> stmts;
};
struct IfStmt {
  IfStmt(expr condition, std::shared_ptr<Stmt> then, std::shared_ptr<Stmt> elsestmt);
  expr condition;
  std::shared_ptr<Stmt> then;
  std::shared_ptr<Stmt> elsestmt;
};
struct While {
  While(expr condition, std::shared_ptr<Stmt> body);
  expr condition;
  std::shared_ptr<Stmt> body;
};
struct ScanStmt {
  ScanStmt(Token name);
  Token name;
};
struct FunDecl {
  FunDecl(Token name, std::vector<Token> param, std::vector<Stmt> body);
  Token name;
  std::vector<Token> param;
  std::vector<Stmt> body;
};
struct ReturnStmt {
  ReturnStmt(Token tok, expr return_value);
  Token tok;
  expr return_value;
};
struct BreakStmt {
  BreakStmt(Token tok);
  Token tok;
};
struct ContinueStmt {
  ContinueStmt(Token tok);
  Token tok;
};

struct InnerExprStmt {Stmt stmt; InnerExprStmt(Stmt stmt); void operator=(Stmt stmt);};

extern Stmt null_stmt;
