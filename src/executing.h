#pragma once

#include "expressions.h"
#include "lexer.h"
#include "statements.h"
#include <exception>
#include <vector>

/*abstract*/ class Interpreter {
public:
  static void interpret(std::vector<Stmt> ex);

  class RuntimeError : public std::exception {
  private:
    const char* msg;
  public:
    Token token;
    RuntimeError(Token token, const char* msg);
    const char* what() const noexcept;
  };
private:
  static void execute(Stmt stmt);
  static void execute_over(PrintStmt stmt);
  static void execute_over(ExprStmt stmt);

  static literal_t evaluate(expr ex);
  static literal_t evaluate_over(Literal ex);
  static literal_t evaluate_over(Binary ex);
  static literal_t evaluate_over(Group ex);
  static literal_t evaluate_over(Unary ex);
};
