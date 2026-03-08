#include "expressions.h"
#include "lexer.h"
#include <iostream>
#include <memory>
#include <variant>

Literal::Literal(literal_t literal) : literal(literal) {}
Group::Group(std::shared_ptr<::expr> exp) {
  exp.swap(this->exp);
} // "unknown type expr, did you mean ::expr?" compiler wake up youre drunk
Binary::Binary(std::shared_ptr<expr> first, std::shared_ptr<expr> second, Token _operator) : _operator(_operator) {
  first.swap(this->first);
  second.swap(this->second);
}
Unary::Unary(Token _operator, std::shared_ptr<expr> postfix) : _operator(_operator) {
  postfix.swap(this->postfix);
}

std::string literal_to_string(literal_t lit) {
  if (auto val = std::get_if<std::string>(&lit)) {
    return *val;
  } else if (auto val = std::get_if<int>(&lit)) {
    return std::to_string(*val);
  } else if (auto val = std::get_if<double>(&lit)) {
    return std::to_string(*val);
  } else {
    return "";
  }
}
