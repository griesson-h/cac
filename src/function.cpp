#include "function.h"
#include "environment.h"
#include "executing.h"
#include "expressions.h"
#include "lexer.h"
#include "statements.h"
#include "classes.h"
#include <cmath>
#include <ctime>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iostream>

literal_t func_t::call(std::vector<literal_t>, Token) {return _NULL;}
int func_t::arity() {return 0;}
std::string func_t::to_string() {return "";}
std::shared_ptr<func_t> func_t::bind(std::shared_ptr<Instance> instance) {return nullptr;}

Function::Function(FunDecl& declaration, std::shared_ptr<Environment> closure, bool isConstructor) : declaration(declaration), closure(closure), isConstructor(isConstructor) {}

int Function::arity() {return declaration.param.size();}

literal_t Function::call(std::vector<literal_t> args, Token) {
  auto backup = Interpreter::env;
  Environment new_env = Environment(closure);
  for (auto i = 0; i < args.size(); i++) {
    new_env.define(declaration.param[i], args[i]);
  }
  try {
    Interpreter::execute_block(declaration.body, new_env);
  } catch (Interpreter::Return val) {
    if (isConstructor) return closure->get_at(0, Token(THIS, "this", nullptr, 0));
    Interpreter::env = backup;
    return val.value;
  }

  if (isConstructor) return closure->get_at(0, Token(THIS, "this", nullptr, 0));
  return _NULL;
}
std::string Function::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print '" << declaration.name.lexeme << "' function, that's...weird>";
  return ss.str();
}

std::shared_ptr<func_t> Function::bind(std::shared_ptr<Instance> instance) {
  std::shared_ptr<Environment> env(new Environment(closure));
  env->define(Token(THIS, "this", nullptr, 0), instance);
  return std::shared_ptr<func_t>(new Function(declaration, env, isConstructor));
}

namespace Foreigns {
int Time::arity() {return 0;}
literal_t Time::call(std::vector<literal_t>, Token) {
  return static_cast<int>(std::time(nullptr));
}
std::string Time::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'time' function, that's...weird>";
  return ss.str();
}

int ToString::arity() {return 1;}
literal_t ToString::call(std::vector<literal_t> args, Token) {
  literal_t val = args[0];
  return LitOp::literal_to_string(val);
}
std::string ToString::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'str' function, that's...weird>";
  return ss.str();
}


int ToInteger::arity() {
  return 1;
}
literal_t ToInteger::call(std::vector<literal_t> args, Token tok) {
  try {
    if (auto val = std::get_if<std::string>(&args[0])) {
      return std::stoi(*val);
    } else if (auto val = std::get_if<double>(&args[0])) {
      return static_cast<int>(*val);
    } else {
      throw Interpreter::RuntimeError(tok, "Unavailable cast to int");
    }
  } catch (std::out_of_range) {
    throw Interpreter::RuntimeError(tok, "Integer overflow while casting value to int");
  } catch (std::invalid_argument) {
    throw Interpreter::RuntimeError(tok, "Unavailable cast to int");
  }
}
std::string ToInteger::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'int' function, that's...weird>";
  return ss.str();
}

int GetLength::arity() {return 1;}
literal_t GetLength::call(std::vector<literal_t> args, Token tok) {
  if (auto val = std::get_if<int>(&args[0])) {
    return *val;
  } else if (auto val = std::get_if<double>(&args[0])) {
    return std::round(*val);
  } else if (auto val = std::get_if<std::string>(&args[0])) {
    return static_cast<int>((*val).size());
  } else if (auto val = std::get_if<std::shared_ptr<List>>(&args[0])) {
    return static_cast<int>((*val)->values.size());
  } else {
    return 0;
  }
}
std::string GetLength::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'len' function, that's...weird>";
  return ss.str();
}
}
