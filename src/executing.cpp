#include "executing.h"
#include "expressions.h"
#include "lexer.h"
#include <cstddef>
#include <iostream>
#include <variant>

void Interpreter::interpret(expr ex) {
  try {
    literal_t result = evaluate(ex);
    std::cout << LitOp::literal_to_string(result) << std::endl;
  } catch(std::nullptr_t) {}
}

literal_t Interpreter::evaluate(expr ex) {
  // evil c++ visitor (never let em in...)
  return std::visit([&](auto&& value){return evaluate_over(value);}, ex);
}

literal_t Interpreter::evaluate_over(Literal ex) {
  return ex.literal;
}

literal_t Interpreter::evaluate_over(Group ex) {
  return evaluate(*ex.exp);
}

literal_t Interpreter::evaluate_over(Unary ex) {
  literal_t right = evaluate(*(ex.postfix));

  switch (ex._operator.type) {
    case MINUS:
      return LitOp::negative(right);
      break;
    case EXCL:
      return LitOp::negative(LitOp::if_true(right));
    default:
      break;
  }
  
  std::cout << "how the fuck" << std::endl;
  return literal_t(_NULL);
}

literal_t Interpreter::evaluate_over(Binary ex) {
  literal_t first = evaluate(*(ex.first));
  literal_t second = evaluate(*(ex.second));

  switch (ex._operator.type) {
    case EXCL_EQUAL:
      return LitOp::negative(LitOp::if_equal(first, second));
    case DOUBLE_EQUAL:
      return LitOp::if_equal(first, second);
    case GREATER:
      return LitOp::greater(first, second);
    case GREATER_EQUAL:
      return LitOp::greater_equal(first, second);
    case LESS:
      return LitOp::less(first, second);
    case LESS_EQUAL:
      return LitOp::less_equal(first, second);
    case PLUS:
      if (std::holds_alternative<int>(first) && std::holds_alternative<int>(second)) {
        return LitOp::add(first, second);
      }
      if (std::holds_alternative<double>(first) && std::holds_alternative<double>(second)) {
        return LitOp::add(first, second);
      }
      if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second)) {
        return literal_t(std::get<std::string>(first).append(std::get<std::string>(second)));
      }
      break;
    case MINUS:
      return LitOp::sub(first, second);
    case SLASH:
      return LitOp::div(first, second);
    case STAR:
      return LitOp::mul(first, second);
    default:
      break;
  }

  std::cout << "how the fuck" << std::endl;
  return literal_t(_NULL);
}
