#pragma once
#include "expressions.h"
#include "lexer.h"
#include <variant>
#include <vector>

struct ExprStmt;
struct PrintStmt;
struct Var;
struct Block;

using Stmt = std::variant<std::monostate, ExprStmt, PrintStmt, Var, Block>;

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
