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

using Stmt = std::variant<std::monostate, ExprStmt, PrintStmt, Var, Block, IfStmt, While>;

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

extern Stmt null_stmt;
