#include "parser.h"
#include "expressions.h"
#include "interpreter.h"
#include "lexer.h"
#include "statements.h"
#include <memory>
#include <sstream>
#include <variant>
#include <vector>
#include <iostream>

int Parser::current = 0;

std::vector<Stmt> Parser::parse() {
  current = 0;
  std::vector<Stmt> statements;
  while (!__EOF()) {
    statements.push_back(declaration());
  }
  return statements;
}

void Parser::synchronize() {
  if (!__EOF()) current++;

  while(!__EOF()) {
    if (tokens[current - 1].type == SEMI) return;
    switch(tokens[current].type) {
      case CLASS:
      case FUNC:
      case VAR:
      case FOR:
      case IF:
      case WHILE:
      case RETURN:
        return;
      default:
        break;
    }
    current++;
  }
}

Stmt Parser::declaration() {
  try {
    if (match(VAR)) {current++; return var_decl();}
    return statement();
  } catch (Error::ParseError err) {
    failed = true;
    synchronize();
    return std::monostate();
  }
}
Stmt Parser::var_decl() {
  Token ident = Error::consume(IDENT, "Expected identifier after 'var' token");

  expr initializer = Literal(literal_t(_NULL));
  if (match(EQUAL)) {
    current++;
    initializer = expression();
  }

  Error::consume(SEMI, "Expected ';' after expression");
  return Var(ident, initializer);
}

Stmt Parser::statement() {
  if (match(PRINT)) {current++; return print_statement();}
  if (match(LEFT_BRACE)) {current++; return Block(block_statement());}

  return expr_statement();
}

Stmt Parser::print_statement() {
  expr arg = expression();
  Error::consume(SEMI, "Expected ';' after statement");
  return Stmt(PrintStmt(arg));
}

Stmt Parser::expr_statement() {
  expr arg = expression();
  Error::consume(SEMI, "Expected ';' after statement");
  return Stmt(ExprStmt(arg));
}

std::vector<Stmt> Parser::block_statement() {
  std::vector<Stmt> statements;

  while(!match(RIGHT_BRACE) && !__EOF()) {
    statements.push_back(declaration());
  }

  Error::consume(RIGHT_BRACE, "Expected '}' after block statement");
  return statements;
}


expr Parser::expression() {
  return assignment();
}

expr Parser::assignment() {
  expr ex = equality();

  if (match(EQUAL)) {
    Token token_to_report = tokens[current];
    current++;
    expr value = assignment();

    if (std::holds_alternative<Variable>(ex)) {
      Token identifier = std::get<Variable>(ex).name;
      return Assign(identifier, std::make_shared<expr>(value));
    }

    Error::error(token_to_report, "holy fucking shit i wanna sleep");
  }
  return ex;
}

expr Parser::equality() {
  expr ex = comparision();
  if (match(EXCL_EQUAL) || match(DOUBLE_EQUAL)) {
    Token _operator = tokens[current];
    current++;
    expr right = comparision();
    expr exT = ex;
    // we redefine ex each time because when it tries to assign the previous ex to the new binary expr bad shit happens
    expr ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
    return ex;
  }
  return ex;
}

expr Parser::comparision() {
  expr ex = term();

  while (match(GREATER) || match(GREATER_EQUAL)
        || match(LESS) || match(LESS_EQUAL)) {
    Token _operator = tokens[current];
    current++;
    expr right = term();
    expr exT = ex;
    expr ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
    return ex;
  }
  return ex;
}

// i will never use c-style pointers again
expr Parser::term() {
  expr ex = factor();

  while (match(PLUS) || match(MINUS)) {
    Token _operator = tokens[current];
    current++;
    expr right = factor();
    expr exT = ex;
    expr ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
    return ex;
  }
  return ex;
}

expr Parser::factor() {
  expr ex = unary();
  if (match(SLASH) || match(STAR)) {
    Token _operator = tokens[current];
    current++;
    expr right = unary();
    expr exT = ex;
    expr ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
    return ex;
  }
  return ex;
}

expr Parser::unary() {
  if (match(EXCL) | match(MINUS)) {
    Token _operator = tokens[current];
    current++;
    expr postfix = unary();
    return Unary(_operator, std::make_shared<expr>(postfix));
  }
  return primary();
}

expr Parser::primary() {
  if (match(TRUE)) {current++; return Literal(literal_t(TRUE));}
  if (match(FALSE)) {current++; return Literal(literal_t(FALSE));}
  if (match(_NULL)) {current++; return Literal(literal_t(_NULL));}

  if (match(STRING) || match(INT) || match(FLOAT)) {
    current++;
    return Literal(tokens[current - 1].literal);
  }

  if (match(LEFT_PARENTH)) {
    current++;
    expr ex = expression();
    Error::consume(RIGHT_PARENTH, "Expected ')' after group expression");
    return Group(std::make_shared<expr>(ex));
  }

  if (match(IDENT)) {
    current++;
    return Variable(tokens[current-1]);
  }

  std::stringstream ss;
  ss << "Expected expression, found '" << tokens[current].lexeme << "'";
  throw Error::error(tokens[current], ss.str().c_str());
}


bool Parser::match(token_type type) {
  if (__EOF()) return false;
  if (tokens[current].type == type) {
    return true;
  }
  return false;
}

bool Parser::__EOF() {
  return tokens.at(current).type == _EOF;
}


Parser::Error::ParseError::ParseError(const char* msg) : msg(msg) {}

Parser::Error::ParseError Parser::Error::error(Token token, const char* msg) {
  report(token, msg);
  return ParseError(msg);
}

Token Parser::Error::consume(token_type type, const char* msg) {
  if (match(type)) return tokens[current++];
  throw error(tokens[current], msg);
}

/* ---------------------------------------------------------- */

std::string PreatyPrinter::print(expr expr) {
  return std::visit([&](auto&& value){return print_over(value);}, expr);
}

std::string PreatyPrinter::print_over(Literal expr) {
  if (expr.literal.index() == 0) return "null";
  return LitOp::literal_to_string(expr.literal);
}
std::string PreatyPrinter::print_over(Group expr) {
  std::vector<::expr> vec;
  vec.push_back(*expr.exp);
  return parenthesize("group", vec);
}
std::string PreatyPrinter::print_over(Binary expr) {
  std::vector<::expr> vec;
  vec.push_back(*expr.second);
  vec.push_back(*expr.first);

  return parenthesize(expr._operator.lexeme.c_str(), vec);
}
std::string PreatyPrinter::print_over(Unary expr) {
  std::vector<::expr> vec;
  vec.push_back(*expr.postfix);
  return parenthesize(expr._operator.lexeme.c_str(), vec);
}
std::string PreatyPrinter::print_over(Variable expr) {
  return expr.name.lexeme;
}
std::string PreatyPrinter::print_over(Assign expr) {
  return "Sada";
}

// there's probably more efficient way to do this but i'm just so tired of c++'s variadic functions
std::string PreatyPrinter::parenthesize(const char* name, std::vector<expr> Exprs) {
  std::stringstream ss;
  ss << "(" << name;
  for (int i = 0; i < Exprs.size(); ++i) {
    ss << " " << print(Exprs[i]);
  }
  ss << ")";
  return ss.str();
}
