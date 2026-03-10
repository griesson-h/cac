#pragma once
#include "expressions.h"
#include <variant>

struct ExprStmt;
struct PrintStmt;

using Stmt = std::variant<ExprStmt, PrintStmt>;

struct ExprStmt {
  ExprStmt(expr exp);
  expr exp;
};
struct PrintStmt {
  PrintStmt(expr exp);
  expr exp;
};
