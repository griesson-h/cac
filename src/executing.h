#pragma once

#include "environment.h"
#include "expressions.h"
#include "lexer.h"
#include "statements.h"
#include <exception>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

/*abstract*/ class Interpreter {
public:
  static void interpret(std::vector<Stmt> &ex);

  static void resolve(expr &ex, int depth);

  static std::shared_ptr<Environment> env;
  static std::shared_ptr<Environment> backup_env; // pointer to the global environment to recover it after block statement/s

  static void execute_block(std::vector<Stmt> &stmts, Environment &new_env);

  class RuntimeError : public std::exception {
  private:
    std::string msg;
  public:
    Token token;
    RuntimeError(Token token, std::string msg);
    const char* what() const noexcept;
  };
  class Return : public std::exception {
  public:
    literal_t value;
    Return(literal_t value);
  };
private:
  static bool in_loop;
  static bool passed_first_init;
  static std::unordered_map<expr*, int> locals;

  static void init_foreigns();

  static void execute(Stmt &stmt);
  static void execute_over(std::monostate&);
  static void execute_over(PrintStmt &stmt);
  static void execute_over(ScanStmt &stmt);
  static void execute_over(ExprStmt &stmt);
  static void execute_over(Var &stmt);
  static void execute_over(Block &stmt);
  static void execute_over(IfStmt &stmt);
  static void execute_over(While &stmt);
  static void execute_over(FunDecl &stmt);
  static void execute_over(BreakStmt &stmt);
  static void execute_over(ContinueStmt &stmt);
  static void execute_over(ReturnStmt &stmt);

  static literal_t evaluate(expr &ex);
  static literal_t evaluate_over(Literal &ex);
  static literal_t evaluate_over(Binary &ex);
  static literal_t evaluate_over(Group &ex);
  static literal_t evaluate_over(Unary &ex);
  static literal_t evaluate_over(Variable &ex);
  static literal_t evaluate_over(Assign &ex);
  static literal_t evaluate_over(LogicalBin &ex);
  static literal_t evaluate_over(Call &ex);
  static literal_t evaluate_over(Lambda &ex);

  class Break : public std::exception {};
  class Continue : public std::exception {};
};
