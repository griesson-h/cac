#pragma once
#include "environment.h"
#include "lexer.h"
#include "expressions.h"
#include "statements.h"
#include <memory>
#include <variant>
#include <vector>

namespace Foreigns {
struct Time;
}

class Environment;

struct func_t {
  virtual int arity();
  virtual literal_t call(std::vector<literal_t> args, Token tok);
  virtual std::string to_string();
  virtual std::shared_ptr<func_t> bind(std::shared_ptr<Instance> instance);
};

class Function : public func_t {
private:
  FunDecl& declaration;
  std::shared_ptr<Environment> closure;
  bool isConstructor;
public:
  Function(FunDecl& declaration, std::shared_ptr<Environment> closure, bool isConstructor);
  int arity() override;
  literal_t call(std::vector<literal_t> args, Token) override;
  std::string to_string() override;
  std::shared_ptr<func_t> bind(std::shared_ptr<Instance> instance) override;
};

namespace Foreigns {
struct Time : public func_t {
  int arity() override;
  literal_t call(std::vector<literal_t> args, Token) override;
  std::string to_string() override;
};
struct ToString : public func_t {
  int arity() override;
  literal_t call(std::vector<literal_t> args, Token) override;
  std::string to_string() override;
};
struct ToInteger : public func_t {
  int arity() override;
  literal_t call(std::vector<literal_t> args, Token tok) override;
  std::string to_string() override;
};
}

