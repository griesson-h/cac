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
  literal_t get_at(int distance, Token name);
  void assign_at(int distance, Token name, literal_t value);
  void assign(Token name, literal_t value);

  std::shared_ptr<Environment> enclosing;
private:
  Environment* ancestor(int distance);
  std::unordered_map<std::string, literal_t> variables;
};
