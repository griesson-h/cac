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
  virtual literal_t call(std::vector<literal_t> args);
  virtual std::string to_string();
};
class Function : public func_t {
private:
  FunDecl declaration;
  std::shared_ptr<Environment> closure;
public:
  Function(FunDecl declaration, std::shared_ptr<Environment> closure);
  int arity() override;
  literal_t call(std::vector<literal_t> args) override;
  std::string to_string() override;
};

namespace Foreigns {
struct Time : public func_t {
  int arity() override;
  literal_t call(std::vector<literal_t> args) override;
  std::string to_string() override;
};
struct ToString : public func_t {
  int arity() override;
  literal_t call(std::vector<literal_t> args) override;
  std::string to_string() override;
};
}

