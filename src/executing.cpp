#include "executing.h"
#include "expressions.h"
#include "interpreter.h"
#include "lexer.h"
#include "statements.h"
#include <iostream>
#include <variant>
#include <vector>

void Interpreter::interpret(std::vector<Stmt> statements) {
  try {
    for (auto statement : statements) {
      execute(statement);
    }
  } catch(RuntimeError e) {
    report_at_runtime(e.token, e.what());
  }
}

void Interpreter::execute(Stmt stmt) {
  std::visit([&](auto&& value){execute_over(value);}, stmt);
}

void Interpreter::execute_over(PrintStmt stmt) {
  literal_t arg = evaluate(stmt.exp);
  std::cout << LitOp::literal_to_string(arg);
  /*TEMP*/ std::cout << std::endl;
}
void Interpreter::execute_over(ExprStmt stmt) {
  evaluate(stmt.exp);
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
      if (LitOp::is_numbers(first, second)) {
        return LitOp::add(first, second);
      } else if (LitOp::contains<std::string>(first) && LitOp::contains<std::string>(second)) {
        return literal_t(std::get<std::string>(first).append(std::get<std::string>(second)));
      } else {
        throw RuntimeError(ex._operator, "Operants must be either numbers or strings");
      }
      break;
    case MINUS:
      if (!(LitOp::is_numbers(first, second)))
        goto err_not_number;
      return LitOp::sub(first, second);
    case SLASH: {
      if (!(LitOp::is_numbers(first, second)))
        goto err_not_number;
      literal_t ret = LitOp::div(first, second);
      if (LitOp::contains<token_type>(ret)) throw RuntimeError(ex._operator, "Division by zero is undefined");
      return ret;
    } // yes to initialize a variable in a switch statement i gotta use this weird braces on case statement idk why c++'s weird ig (also it's 1:15 pm i needa go to bed)
    case STAR:
      if (!(LitOp::is_numbers(first, second)))
        goto err_not_number;
      return LitOp::mul(first, second);
    default:
      break;
  }

  std::cout << "how the fuck" << std::endl;
  return literal_t(_NULL);
err_not_number:
  throw RuntimeError(ex._operator, "Operants must be numbers");
}
Interpreter::RuntimeError::RuntimeError(Token token, const char* msg) : token(token), msg(msg) {}
const char* Interpreter::RuntimeError::what() const noexcept {
  return msg;
}
