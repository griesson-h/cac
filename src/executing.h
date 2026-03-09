#pragma once

#include "expressions.h"
#include "lexer.h"

/*abstract*/ class Interpreter {
public:
  static void interpret(expr ex);
private:
  static literal_t evaluate(expr ex);
  static literal_t evaluate_over(Literal ex);
  static literal_t evaluate_over(Binary ex);
  static literal_t evaluate_over(Group ex);
  static literal_t evaluate_over(Unary ex);
};
