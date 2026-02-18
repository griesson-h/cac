// Thank https://craftinginterpreters.com for the beautiful book

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <csignal>
#include "interpreter.h"
#include "lexer.h"

bool failed = false;

void SIGINT_handler(int s) {
  std::cout << '\n';
  exit(0);
}

void report(int line, const char* message, const char* file) {
  std::cout << "<file " << file << ", line " << line << "> " << message;
  failed = true;
}

void run(std::string bytes) {
  //const char* errorm = "ur too stoopid lmao lol\n";
  //report(69, errorm);
//  if (failed) exit(10);
//  std::stringstream stream(bytes);
//  std::vector<std::string> tokens;
//  std::string token;
//  while (getline(stream, token, ' ')) {
//    if (token.empty()) continue;
//    tokens.push_back(token);
//  }
//  for (int i = 0; i < tokens.size(); ++i) std::cout << tokens[i] << std::endl;
  std::cout << "fsdf" << std::endl;
  lex_tokens(bytes);
  int i = 0;
  for (auto token : tokens) {
    std::cout << std::endl << token.to_string() << '\t' << i << std::endl;
    i++;
  }
}

void run_script(char* source) {
  std::ifstream bytestream(source);
  if (!bytestream.is_open()) {
    throw std::runtime_error("ERROR: passed file does not exist");
  }
  std::string temp;
  std::stringstream bytes;
  while (std::getline(bytestream, temp)) {
    if (temp.empty()) {
      break;
    }
    bytes << temp << '\n';
  }
  bytestream.close();
  run(bytes.str());
}

void run_cmd() {
  bool cmd_should_close = false;
  while (!cmd_should_close) {
    std::cout << "cac-cmd > ";
    std::string line;
    std::getline(std::cin, line);
    run(line);
    failed = false;
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
