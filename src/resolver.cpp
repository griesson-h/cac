#include "resolver.h"
#include "executing.h"
#include "expressions.h"
#include "function.h"
#include "interpreter.h"
#include "lexer.h"
#include "statements.h"
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <stack>
#include <cassert>
#include <iostream>

std::stack<std::unordered_map<std::string, RESOLVE>> Resolver::scopes;

void Resolver::resolve(Stmt &stmt) {
  std::visit([&](auto&& value){return resolve_over(value);}, stmt);
}

void Resolver::resolve(expr &ex) {
  std::visit([&](auto&& value){return resolve_over(value);}, ex);
}

void Resolver::resolve(std::vector<Stmt> &stmts) {
  define(Token(IDENT, "str", nullptr, 0));
  define(Token(IDENT, "time", nullptr, 0));
  int i = 0;
  for (auto& stmt : stmts) {
    assertm(&stmt == &stmts[i], "Adresses are not equal");
    resolve(stmt);
    i++;
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
    if (temp_stack.top().at(name.lexeme) == RESOLVE::DEFINED) {
      return false;
    }
    if (temp_stack.top().at(name.lexeme) == RESOLVE::DECLARED) return true;
    } catch (std::out_of_range) {} // if 'std::stack::at()' fails
    temp_stack.pop();
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

  resolve_func(stmt);
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
  if (is_not_null_expr(stmt.return_value)) resolve(stmt.return_value);
}


void Resolver::resolve_over(Literal& ex) {}

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

void Resolver::resolve_func(FunDecl& stmt) {
  begin_scope();
  for (auto param : stmt.param) {
    declare(param);
    define(param);
  }
  resolve(stmt.body);
  end_scope();
}

void Resolver::resolve_local(expr& ex, Token name) {
  auto temp_stack = scopes;
  int i = temp_stack.size() - 1;
  while (!temp_stack.empty()) {
    if (temp_stack.top().contains(name.lexeme)) {
      Interpreter::resolve(ex, scopes.size() - 1 - i);
      return;
    }
    temp_stack.pop();
    i--;
  }
}


void Resolver::declare(Token name) {
  if (scopes.empty()) return;

  scopes.top()[name.lexeme] = RESOLVE::DECLARED;
}

void Resolver::define(Token name) {
  if (scopes.empty()) return;

  scopes.top()[name.lexeme] = RESOLVE::DEFINED;
}

void Resolver::begin_scope() {
  scopes.push(std::unordered_map<std::string, RESOLVE>());
}

void Resolver::end_scope() {
  scopes.pop();
}
