#pragma once
#include "lexer.h"
#include <memory>
#include <variant>
#include <string>
#include <vector>

struct Assign;
struct Literal;
struct Group;
struct Binary;
struct Unary;
struct Variable;
struct LogicalBin;
struct Call;
struct Lambda;
struct Get;
struct Set;
struct This;

using expr = std::variant<Literal, Group, Binary, Unary, Variable, Assign, LogicalBin, Call, Lambda, Get, Set, This>;
enum expr_type {
  LITERAL, GROUP, BIN, UNARY
};

struct Assign {
  Assign(Token identifier, std::shared_ptr<expr> value);
  Token identifier;
  std::shared_ptr<expr> value;
};
struct Literal {
  Literal(literal_t literal);
  literal_t literal;
};
struct Group {
  Group(std::shared_ptr<expr> exp);
  std::shared_ptr<expr> exp; // i love you cpp compiler <3 (well now i use shared ptrs but i still love you darling <3)
};
struct Binary {
  Binary(std::shared_ptr<expr> first, std::shared_ptr<expr> second, Token _operator);
  std::shared_ptr<expr> first;
  Token _operator;
  std::shared_ptr<expr> second;
};
struct Set {
  Set(std::shared_ptr<expr> object, Token name, std::shared_ptr<expr> value);
  std::shared_ptr<expr> object;
  Token name;
  std::shared_ptr<expr> value;
};
struct This {
  This(Token tok);
  Token tok;
};
struct Unary {
  Unary(Token _operator, std::shared_ptr<expr> postfix);
  Token _operator;
  std::shared_ptr<expr> postfix;
};
struct Call {
  Call(std::shared_ptr<expr> callee, Token tok, std::vector<expr> Args);
  std::shared_ptr<expr> callee;
  Token tok;
  std::vector<expr> Args;
};
struct Get {
  Get(std::shared_ptr<expr> object, Token name);
  std::shared_ptr<expr> object;
  Token name;
};
struct Variable {
  Variable(Token name);
  Token name;
};
struct LogicalBin {
  LogicalBin(std::shared_ptr<expr> first, std::shared_ptr<expr> second, Token _operator);
  std::shared_ptr<expr> first;
  Token _operator;
  std::shared_ptr<expr> second;
};

struct FunDecl;

struct Lambda {
  Lambda(std::shared_ptr<FunDecl> decl); 
  std::shared_ptr<FunDecl> decl;
};

bool is_not_null_expr(expr ex);

/*abstact*/ struct LitOp {
  static std::string literal_to_string(literal_t lit);
  static literal_t negative(literal_t lit);

  static bool if_true(literal_t lit);
  static bool if_true_over(token_type lit);
  static bool if_true_over(int lit);
  static bool if_true_over(double lit);
  static bool if_true_over(std::string lit);
  static bool if_true_over(std::shared_ptr<func_t> lit);
  static bool if_true_over(std::shared_ptr<Instance> lit);
  
  static literal_t add(literal_t lit1, literal_t li2);
  static literal_t sub(literal_t lit1, literal_t li2);
  static literal_t mul(literal_t lit1, literal_t li2);
  static literal_t div(literal_t lit1, literal_t li2);

  static literal_t if_equal(literal_t lit1, literal_t lit2);

  static literal_t greater(literal_t first, literal_t second);
  static literal_t greater_equal(literal_t first, literal_t second);
  static literal_t less(literal_t first, literal_t second);
  static literal_t less_equal(literal_t first, literal_t second);

  template<typename T>
  inline static bool contains(literal_t lit) {
    return std::holds_alternative<T>(lit);
  }

  inline static literal_t bool_make_lit(bool b) {
    return b ? literal_t(TRUE) : literal_t(FALSE);
  }

  static bool is_numbers(literal_t lit1, literal_t lit2);
};
