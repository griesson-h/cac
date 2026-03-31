#include "environment.h"
#include "executing.h"
#include "expressions.h"
#include "function.h"
#include "lexer.h"
#include <memory>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <variant>
//std::unordered_map<std::string, literal_t> Environment::variables;

Environment::Environment() {
  enclosing = nullptr;
}
Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

void Environment::define(Token name, literal_t initializer) {
  variables[name.lexeme] = initializer;
}
void Environment::assign(Token name, literal_t value) {
  if (variables.contains(name.lexeme)) {
    variables[name.lexeme] = value;
    return;
  }

  if (enclosing) {
    enclosing->assign(name, value);
    return;
  }

  std::stringstream ss;
  ss << "Unknown identifier '" << name.lexeme << "'";
  throw Interpreter::RuntimeError(name, ss.str());
}
literal_t Environment::get(Token name) {
  if (variables.contains(name.lexeme)) {
    return variables[name.lexeme];
  }
  if (enclosing) return enclosing->get(name);
  std::stringstream ss;
  ss << "Unknown identifier '" << name.lexeme << "'";
  throw Interpreter::RuntimeError(name, ss.str());
}
literal_t Environment::get_at(int distance, Token name) {
  auto val = ancestor(distance)->get(name);
  return val;
}
void Environment::assign_at(int distance, Token name, literal_t value) {
  ancestor(distance)->variables[name.lexeme] = value;
}
Environment* Environment::ancestor(int distance) {
  Environment* env = this;
  for (int i = 0; i < distance; i++) {
    env = env->enclosing.get();
  }

  return env;
}
