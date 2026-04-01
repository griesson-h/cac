// Thank https://craftinginterpreters.com for the beautiful book

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <csignal>
#include <unordered_map>
#include "executing.h"
#include "parser.h"
#include "interpreter.h"
#include "lexer.h"
#include "resolver.h"

bool failed = false;
bool failed_at_runtime = false;
const bool debuginfo = false;

std::unordered_map<std::string, std::vector<Stmt>> ASTs;

void SIGINT_handler(int s) {
  std::cout << '\n';
  exit(0);
}

void report(int line, const char* message, const char* where, const char* file) {
  std::cout << "<file " << file << ", line " << line << "> " << where << ": " << message << std::endl;
  failed = true;
}
void report(Token token, const char* message) {
  if (token.type == _EOF) {
    report(token.line, message, "at end");
  } else {
    std::stringstream ss;
    ss << "at '" << token.lexeme << "'";
    report(token.line, message, ss.str().c_str());
  }
}
void report_at_runtime(Token token, std::string msg, const char* file) {
  std::cout << "<file " << file << ", line " << token.line << "> "
    << "\nRUNTIME ERROR: " << msg << std::endl;
  failed_at_runtime = true;
}
void report_warning(int line, const char* message, const char* file) {
  std::cout << "<file " << file << ", line " << line << "> " << ": WARNING: " << message << std::endl;
}

void run(std::string bytes, std::string file) {
  if (failed) return;

  if (debuginfo)
    std::cout << "------------debug------------" << std::endl;

  lex_tokens(bytes);

  if (debuginfo) {
    int i = 0;
    for (auto token : tokens) {
      std::cout << std::endl << token.to_string() << '\t' << "id: " << i << std::endl;
      i++;
    }
  }

  if (failed) { // later i'm gonna make a bit more convienient error handling rather than doing a bunch of if statments (hopefully)
    std::cout << "Lexer failed, aborting\n";
    return;
  }

  // TODO: make it work for multipule files
  std::vector<Stmt> program = Parser::parse();
  /*TEMP*/ASTs[file] = program;

  if (failed) {
    std::cout << "Parser failed, aborting\n";
    return;
  }
  Resolver::define_foreigns();
  Resolver::resolve(ASTs[file]);
  if (failed) {
    std::cout << "Resolver failed, aborting\n";
    return;
  }

  if (debuginfo)
    std::cout << "------------output------------" << std::endl;

  Interpreter::interpret(ASTs[file]);
  if (failed_at_runtime) return;

  //std::cout << std::get<Binary>(ex).first->index() << std::endl;
  
  //std::cout << PreatyPrinter::print(ex) << " was printed "<< std::endl;

//  (+ (- 23 43) (* 23 (group 23.43)))
//
//  literal_t lit1 = 23, lit2 = 43, lit3 = 23, lit4 = 23.43;
//  expr Lit1 = Literal(lit1), Lit2 = Literal(43), Lit3 = Literal(lit3), Lit4 = Literal(lit4);
//  expr group = Group(Lit4);
//  expr otricanie = Binary(Lit1, Lit2, Token(MINUS, "-", nullptr, -1));
//  expr umnozhenie = Binary(Lit3, group, Token(STAR, "*", nullptr, -1));
//  expr dobavlenie = Binary(otricanie, umnozhenie, Token(PLUS, "+", nullptr, -1));
//
//  std::cout << PreatyPrinter::print(dobavlenie) << std::endl;
}

void run_script(char* source) {
  std::ifstream bytestream(source);
  if (!bytestream.is_open()) {
    throw std::runtime_error("ERROR: passed file does not exist");
  }
  std::string temp;
  std::stringstream bytes;
  while (std::getline(bytestream, temp)) {
//    if (temp.empty()) {
//      break;
//    }
    bytes << temp << '\n';
  }
  bytestream.close();
  run(bytes.str(), "IDK");
}

void run_cmd() {
  bool cmd_should_close = false;
  int cmd_id = 0;
  while (!cmd_should_close) {
    std::cout << "cac-cmd > ";
    std::string line;
    std::getline(std::cin, line);
    if (line == "clear") {std::cout << "\033[2J\033[1;1H"; continue;} // clear and move the cursor to the top left
    if (line == "exit") break;
    run(line, std::string("CMD_PROMT").append(std::to_string(cmd_id)));
    /*(maybe)TEMP*/tokens.clear();
    failed = false;
    failed_at_runtime = false;
    cmd_id++;
  }
}

int main(int argc, char* argv[]) {
  signal(SIGINT, SIGINT_handler);
  if (argc > 2) {
    std::cerr << "ERROR: too many arguments passed\n";
    return -1;
  } else if (argc == 2) {
    try {run_script(argv[1]);} catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return -1;
    }
  } else {
    try {run_cmd();} catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return -1;
    }
  }
  return 0;
}
