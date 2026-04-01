#pragma once
#include "lexer.h"
#include "statements.h"
#include "expressions.h"
#include <stack>
#include <unordered_map>
#include <vector>

enum class RESOLVE {
  DECLARED = 2, DEFINED = 3, USED = 4
};
enum class FUNC_TYPE {
  NONE, FUNC, INIT, METHOD
};
enum class CLASS_TYPE {
  NONE, CLASS, SUBCLASS
};

/*abstract*/ class Resolver {
public:
  static void resolve(Stmt &stmt);
  static void resolve(expr &ex);
  static void resolve(std::vector<Stmt> &stmts);
  static void resolve_local(expr &ex, Token name);
  static void resolve_func(FunDecl &stmt, FUNC_TYPE decl);
  static void define_foreigns();

private:
  static std::vector<std::unordered_map<std::string, std::pair<RESOLVE, int>>> scopes;
  static FUNC_TYPE currentFuncType;
  static CLASS_TYPE currentClassType;
  static long current;

  static void begin_scope();
  static void end_scope();
  static void declare(Token name);
  static void define(Token name);
  static bool is_not_defined(Token name);

  static void resolve_over(Block &stmt);
  static void resolve_over(std::monostate&);
  static void resolve_over(PrintStmt &stmt);
  static void resolve_over(ScanStmt &stmt);
  static void resolve_over(ExprStmt &stmt);
  static void resolve_over(Var &stmt);
  static void resolve_over(IfStmt &stmt);
  static void resolve_over(While &stmt);
  static void resolve_over(FunDecl &stmt);
  static void resolve_over(BreakStmt &stmt);
  static void resolve_over(ContinueStmt &stmt);
  static void resolve_over(ReturnStmt &stmt);
  static void resolve_over(ClassDecl &stmt);
  static void resolve_over(Label &stmt);
  static void resolve_over(Goto &stmt);

  static void resolve_over(Literal &ex);
  static void resolve_over(Binary &ex);
  static void resolve_over(Group &ex);
  static void resolve_over(Unary &ex);
  static void resolve_over(Variable &ex);
  static void resolve_over(Assign &ex);
  static void resolve_over(LogicalBin &ex);
  static void resolve_over(Call &ex);
  static void resolve_over(Get &ex);
  static void resolve_over(Set &ex);
  static void resolve_over(This &ex);
  static void resolve_over(Super &ex);
  static void resolve_over(Lambda &ex);
};
