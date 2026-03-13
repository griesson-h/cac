#pragma once
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
private:
  std::shared_ptr<Environment> enclosing;
  std::unordered_map<std::string, literal_t> variables;
};
