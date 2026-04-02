#include "resolver.h"
#include "executing.h"
#include "expressions.h"
#include "interpreter.h"
#include "lexer.h"
#include "statements.h"
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cassert>
#include <iostream>

std::vector<std::unordered_map<std::string, std::pair<RESOLVE, int>>> Resolver::scopes;
FUNC_TYPE Resolver::currentFuncType = FUNC_TYPE::NONE;
CLASS_TYPE Resolver::currentClassType = CLASS_TYPE::NONE;
long Resolver::current = 0;
std::vector<std::string> Resolver::included;

void Resolver::resolve(Stmt &stmt) {
  std::visit([&](auto&& value){return resolve_over(value);}, stmt);
}

void Resolver::resolve(expr &ex) {
  std::visit([&](auto&& value){return resolve_over(value);}, ex);
}
void Resolver::define_foreigns() {
  define(Token(IDENT, "str", nullptr, 0));
  define(Token(IDENT, "time", nullptr, 0));
  define(Token(IDENT, "int", nullptr, 0));
  define(Token(IDENT, "len", nullptr, 0));
  define(Token(IDENT, "FILE", nullptr, 0));
}
void Resolver::resolve_init(std::vector<Stmt> &stmts) {
  for (; current < stmts.size(); ++current) {
    resolve(stmts[current]);
  }
}
void Resolver::resolve(std::vector<Stmt> &stmts) {
  for (auto& stmt : stmts) {
    resolve(stmt);
  }
}


void Resolver::resolve_over(Block& stmt) {
  begin_scope();
  resolve(stmt.stmts);
  end_scope();
}

void Resolver::resolve_over(Var& stmt) {
  declare(stmt.name);
  if (is_not_null_expr(stmt.initializer)) {
    resolve(stmt.initializer);
  }
  define(stmt.name);
}

bool Resolver::is_not_defined(Token name) {
  auto temp_stack = scopes;
  while (!temp_stack.empty()) {
    try {
    if (temp_stack.front().at(name.lexeme).first == RESOLVE::DEFINED) {
      return false;
    }
    if (temp_stack.front().at(name.lexeme).first == RESOLVE::DECLARED) return true;
    } catch (std::out_of_range) {} // if 'std::stack::at()' fails
    temp_stack.pop_back();
  }
  return false;
}

void Resolver::resolve_over(Variable& ex) {
  if (!scopes.empty() && is_not_defined(ex.name)) {
    std::stringstream ss;
    ss << "Variable '" << ex.name.lexeme << "' in its own initializer";
    report(ex.name, ss.str().c_str());
  }

  resolve_local(reinterpret_cast<expr&>(ex), ex.name);
}

void Resolver::resolve_over(Assign& ex) {
  resolve(*ex.value);
  resolve_local(reinterpret_cast<expr&>(ex), ex.identifier);
}

void Resolver::resolve_over(FunDecl& stmt) {
  declare(stmt.name);
  define(stmt.name);

  resolve_func(stmt, FUNC_TYPE::FUNC);
}

void Resolver::resolve_over(Include& stmt) {
  included.push_back(stmt.path);
}

// boring stuff

void Resolver::resolve_over(std::monostate&) {}

void Resolver::resolve_over(PrintStmt& stmt) {resolve(stmt.exp);}

void Resolver::resolve_over(ScanStmt& stmt) {resolve_local(reinterpret_cast<expr&>(stmt.name), stmt.name.name);}

void Resolver::resolve_over(ExprStmt& stmt) {resolve(stmt.exp);}

void Resolver::resolve_over(IfStmt& stmt) {
  resolve(stmt.condition);
  resolve(*stmt.then);
  if (stmt.elsestmt) resolve(*stmt.elsestmt);
}

void Resolver::resolve_over(While& stmt) {
  resolve(stmt.condition);
  resolve(*stmt.body);
}

void Resolver::resolve_over(BreakStmt& stmt) {}

void Resolver::resolve_over(ContinueStmt& stmt) {}

void Resolver::resolve_over(ReturnStmt& stmt) {
  if (currentFuncType == FUNC_TYPE::NONE)
    report(stmt.tok, "Cannot return from a non-function scope (wth did you think was supposed to happen?)");
  if (is_not_null_expr(stmt.return_value)) {
    if (currentFuncType == FUNC_TYPE::INIT)
      report(stmt.tok, "Constructor cannot return non-null literal");
    resolve(stmt.return_value);
  }
}

void Resolver::resolve_over(ClassDecl& stmt) {
  CLASS_TYPE enclose = currentClassType;
  currentClassType = CLASS_TYPE::CLASS;

  declare(stmt.name);
  define(stmt.name);

  begin_scope();
  scopes.front()["this"] = std::pair<RESOLVE, int>(RESOLVE::USED, stmt.name.line);

  if (is_not_null_expr(stmt.base)) {
    if (std::get<Variable>(stmt.base).name.lexeme == stmt.name.lexeme)
      report(stmt.name, "Cannot inherit from itself");
    currentClassType = CLASS_TYPE::SUBCLASS;
    resolve(stmt.base);
    begin_scope();
    scopes.front()["super"] = std::pair<RESOLVE, int>(RESOLVE::USED, stmt.name.line);
  }

  for (auto &method : stmt.methods) {
    FUNC_TYPE declaration = FUNC_TYPE::METHOD;
    if (method.name.lexeme == stmt.name.lexeme)
      declaration = FUNC_TYPE::INIT;
    resolve_func(method, declaration);
  }
  if (is_not_null_expr(stmt.base))
    end_scope();
  end_scope();
  currentClassType = enclose;
}
void Resolver::resolve_over(Label& stmt) {
  Interpreter::labels[stmt.name.lexeme] = current;
}
void Resolver::resolve_over(Goto& stmt) {}


void Resolver::resolve_over(Literal& ex) {}

void Resolver::resolve_over(ListExpr& ex) {}

void Resolver::resolve_over(ListGet& ex) {
  resolve(*ex.list);
  resolve(*ex.index);
}

void Resolver::resolve_over(ListSet& ex) {
  resolve(*ex.list);
  resolve(*ex.index);
  resolve(*ex.value);
}

void Resolver::resolve_over(Binary& ex) {
  resolve(*ex.first);
  resolve(*ex.second);
}

void Resolver::resolve_over(Group& ex) {resolve(*ex.exp);}

void Resolver::resolve_over(Unary& ex) {resolve(*ex.postfix);}

void Resolver::resolve_over(LogicalBin& ex) {
  resolve(*ex.first);
  resolve(*ex.second);
}

void Resolver::resolve_over(Call& ex) {
  resolve(*ex.callee);
  for (auto& arg : ex.Args) {
    resolve(arg);
  }
}

void Resolver::resolve_over(Get& ex) {
  resolve(*ex.object);
}

void Resolver::resolve_over(Set& ex) {
  resolve(*ex.object);
  resolve(*ex.value);
}

void Resolver::resolve_over(This& ex) {
  if (currentClassType == CLASS_TYPE::NONE)
    report(ex.tok, "Cannot use 'this' outside a class definition");
  resolve_local(reinterpret_cast<expr&>(ex), ex.tok);
}

void Resolver::resolve_over(Super& ex) {
  if (currentClassType == CLASS_TYPE::NONE)
    report(ex.tok, "Cannot use 'superduper' outside a class definition");
  else if (currentClassType == CLASS_TYPE::CLASS) {
    report(ex.tok, "Cannot use 'superduper' inside a class that doesn't have a base");
  }
  resolve_local(reinterpret_cast<expr&>(ex), ex.tok);
}

void Resolver::resolve_over(Lambda& ex) {
  begin_scope();
  for (auto param : ex.decl->param) {
    declare(param);
    define(param);
  }
  resolve(ex.decl->body);
  end_scope();
}
// end of boring stuff

void Resolver::resolve_func(FunDecl& stmt, FUNC_TYPE declaration) {
  FUNC_TYPE enclose = currentFuncType;
  currentFuncType = declaration;
  begin_scope();
  for (auto param : stmt.param) {
    declare(param);
    define(param);
  }
  resolve(stmt.body);
  end_scope();
  currentFuncType = enclose;
}

void Resolver::resolve_local(expr& ex, Token name) {
  auto temp_stack = scopes;
  int i = temp_stack.size() - 1;
  while (!temp_stack.empty()) {
    if (temp_stack.front().contains(name.lexeme)) {
      scopes[scopes.size() - 1 - i][name.lexeme].first = RESOLVE::USED;
      Interpreter::resolve(ex, scopes.size() - 1 - i);
      return;
    }
    temp_stack.pop_back();
    i--;
  }
}


void Resolver::declare(Token name) {
  if (scopes.empty()) return;
  if (scopes.front().contains(name.lexeme)) {
    std::stringstream ss;
    ss << "Variable with the same name '" << name.lexeme << "' already exists in this scope";
    report(name, ss.str().c_str());
  }

  scopes.front()[name.lexeme] = std::pair<RESOLVE, int>(RESOLVE::DECLARED, name.line);
}

void Resolver::define(Token name) {
  if (scopes.empty()) return;

  scopes.front()[name.lexeme] = std::pair<RESOLVE, int>(RESOLVE::DEFINED, name.line);
}

void Resolver::begin_scope() {
  scopes.push_back(std::unordered_map<std::string, std::pair<RESOLVE, int>>());
}

void Resolver::end_scope() {
  for (auto& key : scopes.front()) {
    if (key.second.first == RESOLVE::DEFINED) {
      std::stringstream ss;
      ss << "Variable '" << key.first << "' is not used";
      report_warning(key.second.second, ss.str().c_str());
    }
  }
  scopes.pop_back();
}
