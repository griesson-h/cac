#pragma once

#include "environment.h"
#include "expressions.h"
#include "lexer.h"
#include "statements.h"
#include <exception>
#include <memory>
#include <variant>
#include <vector>

/*abstract*/ class Interpreter {
public:
  static void interpret(std::vector<Stmt> ex);

  class RuntimeError : public std::exception {
  private:
    std::string msg;
  public:
    Token token;
    RuntimeError(Token token, std::string msg);
    const char* what() const noexcept;
  };
private:
  static std::shared_ptr<Environment> env;
  static std::shared_ptr<Environment> backup_env; // pointer to the global environment to recover it after block statement/s

  static void execute(Stmt stmt);
  static void execute_over(std::monostate);
  static void execute_over(PrintStmt stmt);
  static void execute_over(ScanStmt stmt);
  static void execute_over(ExprStmt stmt);
  static void execute_over(Var stmt);
  static void execute_over(Block stmt);
  static void execute_over(IfStmt stmt);
  static void execute_over(While stmt);

  static literal_t evaluate(expr ex);
  static literal_t evaluate_over(Literal ex);
  static literal_t evaluate_over(Binary ex);
  static literal_t evaluate_over(Group ex);
  static literal_t evaluate_over(Unary ex);
  static literal_t evaluate_over(Variable ex);
  static literal_t evaluate_over(Assign ex);
  static literal_t evaluate_over(LogicalBin ex);
};
