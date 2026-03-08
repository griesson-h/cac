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

std::string literal_to_string(literal_t lit);
