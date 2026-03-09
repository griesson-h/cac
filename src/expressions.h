#pragma once
#include "lexer.h"
#include <memory>
#include <variant>
#include <string>

struct Literal;
struct Group;
struct Binary;
struct Unary;

using expr = std::variant<Literal, Group, Binary, Unary>;

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
struct Unary {
  Unary(Token _operator, std::shared_ptr<expr> postfix);
  Token _operator;
  std::shared_ptr<expr> postfix;
};

/*abstact*/ struct LitOp {
  static std::string literal_to_string(literal_t lit);
  static literal_t negative(literal_t lit);

  static literal_t if_true(literal_t lit);
  static literal_t if_true_over(token_type lit);
  static literal_t if_true_over(int lit);
  static literal_t if_true_over(double lit);
  static literal_t if_true_over(std::string lit);
  
  static literal_t add(literal_t lit1, literal_t li2);
  static literal_t sub(literal_t lit1, literal_t li2);
  static literal_t mul(literal_t lit1, literal_t li2);
  static literal_t div(literal_t lit1, literal_t li2);

  static literal_t if_equal(literal_t lit1, literal_t lit2);

  static literal_t greater(literal_t first, literal_t second);
  static literal_t greater_equal(literal_t first, literal_t second);
  static literal_t less(literal_t first, literal_t second);
  static literal_t less_equal(literal_t first, literal_t second);
};
