#pragma once
#include "lexer.h"
#include "function.h"
#include <memory>
#include <string>
#include <unordered_map>

struct class_t : public func_t {
  virtual int arity() override;
  virtual literal_t call(std::vector<literal_t> args, Token) override;
  virtual std::string to_string() override;
};

class Classs : public class_t {
private:
  std::unordered_map<std::string, std::shared_ptr<func_t>> methods;
public:
  Token name;
  Classs(Token name);
  Classs(Token name, std::unordered_map<std::string, std::shared_ptr<func_t>> methods);

  int arity() override;
  literal_t call(std::vector<literal_t> args, Token) override;
  std::string to_string() override;
  std::shared_ptr<func_t> lookup_method(Token name);
};

class Instance : public std::enable_shared_from_this<Instance> {
private:
  std::shared_ptr<Classs> klass;
  std::unordered_map<std::string, literal_t> fields;
public:
  Instance(std::shared_ptr<Classs> klass);
  std::string to_string();
  literal_t get(Token name);
  void set(Token name, literal_t value);
};
