#include "function.h"
#include "environment.h"
#include "executing.h"
#include "expressions.h"
#include "lexer.h"
#include "statements.h"
#include <ctime>
#include <memory>
#include <sstream>
#include <vector>

literal_t func_t::call(std::vector<literal_t>) {return _NULL;}
int func_t::arity() {return 0;}
std::string func_t::to_string() {return "";}

Function::Function(FunDecl declaration, std::shared_ptr<Environment> closure) : declaration(declaration), closure(closure) {}
int Function::arity() {return declaration.param.capacity();}
literal_t Function::call(std::vector<literal_t> args) {
  auto backup = Interpreter::env;
  Environment new_env = Environment(closure);
  for (auto i = 0; i < args.capacity(); i++) {
    new_env.define(declaration.param[i], args[i]);
  }
  try {
    Interpreter::execute_block(declaration.body, new_env);
  } catch (Interpreter::Return val) {
    Interpreter::env = backup;
    return val.value;
  }

  return _NULL;
}
std::string Function::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print '" << declaration.name.lexeme << "' function, that's...weird>";
  return ss.str();
}

namespace Foreigns {
int Time::arity() {return 0;}
literal_t Time::call(std::vector<literal_t>) {
  return static_cast<int>(std::time(nullptr));
}
std::string Time::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'time' function, that's...weird>";
  return ss.str();
}

int ToString::arity() {return 1;}
literal_t ToString::call(std::vector<literal_t> args) {
  literal_t val = args[0];
  return LitOp::literal_to_string(val);
}
std::string ToString::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'str' function, that's...weird>";
  return ss.str();
}
}
