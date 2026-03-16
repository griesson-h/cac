#include "environment.h"
#include "executing.h"
#include "function.h"
#include "lexer.h"
#include <memory>
#include <sstream>
#include <unordered_map>
#include <iostream>

//std::unordered_map<std::string, literal_t> Environment::variables;

Environment::Environment() {
  enclosing = nullptr;
}
Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

void Environment::define(Token name, literal_t initializer) {
  if (!variables.contains(name.lexeme)) {
    variables[name.lexeme] = initializer;
    return;
  }

  std::stringstream ss;
  ss << "Redifinition of '" << name.lexeme << "', try assigning instead";
  throw Interpreter::RuntimeError(name, ss.str());
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
