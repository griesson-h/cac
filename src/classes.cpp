#include "classes.h"
#include "executing.h"
#include "function.h"
#include "lexer.h"
#include <memory>
#include <sstream>

std::string class_t::to_string() {return "";}
literal_t class_t::call(std::vector<literal_t> args, Token) {return _NULL;}
int class_t::arity() {return 0;}

Classs::Classs(Token name) : name(name) {}
Classs::Classs(Token name, std::unordered_map<std::string, std::shared_ptr<func_t>> methods) : name(name), methods(methods) {}
literal_t Classs::call(std::vector<literal_t> args, Token) {
  Classs thiss = *this;
  std::shared_ptr<Instance> instance (new Instance(std::make_shared<Classs>(thiss)));
  std::shared_ptr<func_t> initializer = lookup_method(Token(IDENT, name.lexeme, nullptr, name.line));
  if (initializer) {
    initializer->bind(instance)->call(args, null_token);
  }
  return instance;
}
int Classs::arity() {
  std::shared_ptr<func_t> initializer = lookup_method(Token(IDENT, name.lexeme, nullptr, name.line));
  if (initializer) return initializer->arity();
  return 0;
}
std::string Classs::to_string() {
  std::stringstream ss;
  ss << "<why whould you even print class '" << name.lexeme << "'? Uh, people are so weird ):>";
  return ss.str();
}
std::shared_ptr<func_t> Classs::lookup_method(Token name) {
  if (methods.contains(name.lexeme)) {
    return methods[name.lexeme];
  }

  return nullptr;
}

Instance::Instance(std::shared_ptr<Classs> klass) : klass(klass) {}
std::string Instance::to_string() {
  std::stringstream ss;
  ss << "<why whould you even print an instance of class '" << klass->name.lexeme << "'? Uh, people are so weird ):>";
  return ss.str();
}
literal_t Instance::get(Token name) {
  if (fields.contains(name.lexeme)) {
    return fields[name.lexeme];
  }

  std::shared_ptr<func_t> method = klass->lookup_method(name);
  if (method) {
    return method->bind(shared_from_this()); 
  }

  std::stringstream ss;
  ss << "Undefined property '" << name.lexeme << "'";
  throw Interpreter::RuntimeError(name, ss.str());
}
void Instance::set(Token name, literal_t value) {
  fields[name.lexeme] = value;
}
