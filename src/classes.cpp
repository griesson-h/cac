#include "classes.h"
#include "executing.h"
#include "expressions.h"
#include "function.h"
#include "lexer.h"
#include <memory>
#include <sstream>
#include <iostream>
#include <variant>

std::string class_t::to_string() {return "";}
literal_t class_t::call(std::vector<literal_t> args, Token) {return _NULL;}
int class_t::arity() {return 0;}
std::shared_ptr<func_t> class_t::lookup_method(Token) {return nullptr;}
Token class_t::get_token() {return null_token;}

Classs::Classs(Token name) : name(name) {}
Classs::Classs(Token name, std::unordered_map<std::string, std::shared_ptr<func_t>> methods, std::shared_ptr<class_t> base) : name(name), methods(methods), base(base) {}
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

  if (base)
    return base->lookup_method(name);

  return nullptr;
}
Token Classs::get_token() {
  return name;
}

Instance::Instance(std::shared_ptr<class_t> klass) : klass(klass) {}
std::string Instance::to_string() {
  std::stringstream ss;
  ss << "<why whould you even print an instance of class '" << klass->get_token().lexeme << "'? Uh, people are so weird ):>";
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

namespace Foreigns {

// -------------------------------------------------------------------- 
// CacFile (FILE) class
// -------------------------------------------------------------------- 

CacFile::CacFile() {
  methods["read"] = std::shared_ptr<func_t>(new CacFileMethodGet(nullptr));
  methods["write"] = std::shared_ptr<func_t>(new CacFileMethodWrite(nullptr));
}
int CacFile::arity() {return 1;}
literal_t CacFile::call(std::vector<literal_t> args, Token tok) {
  CacFile thiss = *this;
  std::shared_ptr<Instance> instance (new Instance(std::make_shared<CacFile>(thiss)));
  instance->set(Token(IDENT, "path", nullptr, 0), literal_t(args[0]));
  return instance;
}
std::string CacFile::to_string() {
  return "<why whould you even print class 'FILE'? Uh, people are so weird ):>";
}
std::shared_ptr<func_t> CacFile::lookup_method(Token name) {
  if (methods.contains(name.lexeme)) {
    return methods[name.lexeme];
  }

  return nullptr;
}
Token CacFile::get_token() {return name;}

// -------------------------------CacFileMethodGet(read()) method------------------------------------- 

CacFileMethodGet::CacFileMethodGet(std::shared_ptr<Instance> instance) : instance(instance) {}
int CacFileMethodGet::arity() {return 2;}
literal_t CacFileMethodGet::call(std::vector<literal_t> args, Token tok) {
  // TODO: error handling
  auto pathlit = instance->get(Token(IDENT, "path", nullptr, 0));
  if (is_not_null_expr(pathlit) && LitOp::contains<std::string>(pathlit)) {
    try {
    std::string path = std::get<std::string>(pathlit);
    std::fstream file(path);
    if (!file)
        throw Interpreter::RuntimeError(tok, "FILE: 'FILE.read': Reading file failed");
    std::stringstream ss;

    if (std::get<int>(args[0]) == 0 && std::get<int>(args[1]) == 0) {
      std::string temp;
      while (std::getline(file, temp)) {
        ss << temp << '\n';
      }
      return ss.str();
    }

    file.seekg(std::abs(std::get<int>(args[0])));

    for (int i = 0; i < std::get<int>(args[1]); ++i) {
      char c = static_cast<char>(file.get());
      if (file.eof()) break;
      ss << c;
    }
    return ss.str();
    } catch (std::bad_variant_access) {
      throw Interpreter::RuntimeError(tok, "FILE: Either of arguement for 'FILE.read' should be integers");
    }
  } else {
    throw Interpreter::RuntimeError(tok, "FILE: 'path' field has incorrect type");
  }
}
std::string CacFileMethodGet::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'read' function, that's...weird>";
  return ss.str();
}
std::shared_ptr<func_t> CacFileMethodGet::bind(std::shared_ptr<Instance> instance) {
  return std::shared_ptr<func_t>(new CacFileMethodGet(instance));
}

// -------------------------------CacFileMethodWrite(write()) method------------------------------------- 

CacFileMethodWrite::CacFileMethodWrite(std::shared_ptr<Instance> instance) : instance(instance) {}
int CacFileMethodWrite::arity() {return 2;}
literal_t CacFileMethodWrite::call(std::vector<literal_t> args, Token tok)  {
  auto pathlit = instance->get(Token(IDENT, "path", nullptr, 0));
  if (LitOp::contains<std::string>(pathlit)) {
    try {
      std::string path = std::get<std::string>(pathlit);
      std::fstream file(path);
      if (!file)
        throw Interpreter::RuntimeError(tok, "FILE: 'FILE.read': Reading file failed");

      file.seekg(std::abs(std::get<int>(args[0])));
      file << std::get<std::string>(args[1]);
    } catch(std::bad_variant_access) {
      throw Interpreter::RuntimeError(tok, "FILE: Invalid arguement type for 'FILE.write'; should be: integer, string");
    }
  } else {
    throw Interpreter::RuntimeError(tok, "FILE: 'path' field has incorrect type");
  }
  return _NULL;
}
std::string CacFileMethodWrite::to_string() {
  std::stringstream ss;
  ss << "<see you tried to print 'write' function, that's...weird>";
  return ss.str();
}
std::shared_ptr<func_t> CacFileMethodWrite::bind(std::shared_ptr<Instance> instance) {
  return std::shared_ptr<func_t>(new CacFileMethodWrite(instance));
}
}
