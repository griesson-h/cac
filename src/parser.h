#pragma once
#include "expressions.h"
#include "lexer.h"
#include "statements.h"
#include <concepts>
#include <string>
#include <vector>

/*abstract*/ class Parser {
public:
  static std::vector<Stmt> parse();
private:
  static int current;

  static void synchronize();

  static Stmt statement();
  static Stmt print_statement();
  static Stmt expr_statement();

  static expr expression();
  static expr equality();
  static expr comparision();
  static expr term();
  static expr factor();
  static expr unary();
  static expr primary();

  static bool match(token_type type);
  static bool __EOF();
  static expr peek();

  /*abstract*/ struct Error {
    class ParseError {
    private:
      const char* msg;
    public:
      ParseError(const char* msg);
    };

    static ParseError error(Token token, const char* msg);
    static Token consume(token_type type, const char* msg);
  };
};

/*abstract*/ class PreatyPrinter {
public:
  static std::string print(expr expr);
private:
  static std::string print_over(Literal expr);
  static std::string print_over(Group expr);
  static std::string print_over(Binary expr);
  static std::string print_over(Unary expr);
  static std::string parenthesize(const char* name, std::vector<expr> Exprs);
};
