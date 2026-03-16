#pragma once
#include "function.h"
#include "lexer.h"
#include <memory>
#include <unordered_map>

class Environment {
public:
  Environment();
  Environment(std::shared_ptr<Environment> enclosing);

  void define(Token name, literal_t initializer);
  literal_t get(Token name);
  void assign(Token name, literal_t value);

  std::shared_ptr<Environment> enclosing;
private:
  std::unordered_map<std::string, literal_t> variables;
};
