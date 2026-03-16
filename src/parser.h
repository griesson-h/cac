#pragma once
#include "expressions.h"
#include "lexer.h"
#include "statements.h"
#include <concepts>
#include <string>
#include <vector>

const int MAX_ARGS = 255;

/*abstract*/ class Parser {
public:
  static std::vector<Stmt> parse();
private:
  static int current;

  static void synchronize();

  static Stmt declaration();
  static Stmt var_decl();
  static Stmt func_decl();

  static Stmt statement();
  static Stmt print_statement();
  static Stmt scan_statement();
  static Stmt expr_statement();
  static std::vector<Stmt> block_statement();
  static Stmt if_statement();
  static Stmt while_statement();
  static Stmt for_statement();
  static Stmt return_statement();
  static Stmt break_statement();
  static Stmt continue_statement();

  static expr assignment();
  static expr lambda();
  static expr expression();
  static expr or_expr();
  static expr and_expr();
  static expr equality();
  static expr comparision();
  static expr term();
  static expr factor();
  static expr unary();
  static expr call();
  static expr primary();

  static bool match(token_type type);
  static bool match_consume(token_type type);
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
  static std::string print_over(Variable expr);
  static std::string print_over(Assign expr);
  static std::string print_over(LogicalBin expr);
  static std::string print_over(Call expr);
  static std::string print_over(Lambda expr);
  static std::string parenthesize(const char* name, std::vector<expr> Exprs);
};
