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
    if (match(FUNC)) {current++; return func_decl();}
    if (match(VAR)) {current++; return var_decl();}
    if (match(CLASS)) {current++; return class_decl();}
    return statement();
  } catch (Error::ParseError err) {
    failed = true;
    synchronize();
    return std::monostate();
  }
}
Stmt Parser::func_decl() {
  Token ident = Error::consume(IDENT, "Expected identifier after 'func' token");

  Error::consume(LEFT_PARENTH, "Expected '(' after function identifier");

  std::vector<Token> param;
  if (!match(RIGHT_PARENTH)) {
    do {
      if (param.capacity() >= MAX_ARGS)
        Error::error(tokens[current], "Parameter overflow");
      param.push_back(Error::consume(IDENT, "Expected identifier"));
    } while (match_consume(COMMA));
  }
  Error::consume(RIGHT_PARENTH, "Unterminated parameter list");

  Error::consume(LEFT_BRACE, "Expected '{' after function declaration");
  std::vector<Stmt> body = block_statement();
  return FunDecl(ident, param, body);
}
Stmt Parser::var_decl() {
  Token ident = Error::consume(IDENT, "Expected identifier after 'var' token");

  expr initializer = Literal(literal_t(_NULL));
  if (match(EQUAL)) {
    current++;
    initializer = expression();
  }

  Error::consume(SEMI, "Expected ';' after statement");
  return Var(ident, initializer);
}
Stmt Parser::class_decl() {
  Token ident = Error::consume(IDENT, "Expected identifier after 'class' token");
  // handle inheretence
  Error::consume(LEFT_BRACE, "Expected '{' after class declaration");
  std::vector<FunDecl> methods;
  while (!match(RIGHT_BRACE) && !__EOF()) {
    FunDecl decl = std::get<FunDecl>(declaration());
    methods.push_back(decl);
  }
  Error::consume(RIGHT_BRACE, "Expected '}' after class declaration");
  Error::consume(SEMI, "Expected ';' after statement");
  return ClassDecl(ident, methods);
}

Stmt Parser::statement() {
  if (match(PRINT)) {current++; return print_statement();}
  if (match(LEFT_BRACE)) {current++; return Block(block_statement());}
  if (match(IF)) {current++; return if_statement();}
  if (match(WHILE)) {current++; return while_statement();}
  if (match(FOR)) {current++; return for_statement();}
  if (match(SCAN)) {current++; return scan_statement();}
  if (match(BREAK)) {current++; return break_statement();}
  if (match(CONTINUE)) {current++; return continue_statement();}
  if (match(RETURN)) {current++; return return_statement();}
  if (match(LABEL)) {current++; return label_statement();}
  if (match(GOTO)) {current++; return goto_statement();}

  return expr_statement();
}

Stmt Parser::print_statement() {
  expr arg = expression();
  Error::consume(SEMI, "Expected ';' after statement");
  return Stmt(PrintStmt(arg));
}

Stmt Parser::scan_statement() {
  expr arg = primary();

  Error::consume(SEMI, "Expected ';' after statement");

  if (std::holds_alternative<Variable>(arg)) {
    return ScanStmt(std::get<Variable>(arg).name);
  }

  return null_stmt;
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

Stmt Parser::if_statement() {
  bool parenthesized = false;
  if (match(LEFT_PARENTH)) {
    current++;
    parenthesized = true;
  }

  expr condition = expression();
  if (parenthesized) 
    Error::consume(RIGHT_PARENTH, "Expected ')' after parenthesized if condition");

  Stmt then = statement();
  std::shared_ptr<Stmt> elsestmt = nullptr;
  if (match(ELSE)) {
    current++;
    elsestmt = std::make_shared<Stmt>(statement());
  }
  return IfStmt(condition, std::make_shared<Stmt>(then),elsestmt);
}

Stmt Parser::while_statement() {
  bool parenthesized = false;
  if (match(LEFT_PARENTH)) {
    current++;
    parenthesized = true;
  }

  expr condition = expression();
  if (parenthesized) 
    Error::consume(RIGHT_PARENTH, "Expected ')' after parenthesized while condition");

  Stmt body = statement();
  return While(condition, std::make_shared<Stmt>(body));
}

Stmt Parser::for_statement() {
  bool parenthesized = false;
  if (match(LEFT_PARENTH)) {
    current++;
    parenthesized = true;
  }

  std::shared_ptr<Stmt> initializer;
  if (match(VAR)) {
    current++;
    initializer = std::make_shared<Stmt>(var_decl());
  } else if (match(SEMI)) {
    current++;
    initializer = nullptr;
  } else {
    initializer = std::make_shared<Stmt>(expr_statement());
  }

  expr condition = Literal(literal_t(TRUE));
  if (!match(SEMI)) {
    condition = expression();
  }
  Error::consume(SEMI, "Expected ';' after condition");

  expr increment = Literal(literal_t(_NULL));
  if (!match(RIGHT_PARENTH)) {
    increment = expression();
  }
  if (parenthesized)
    Error::consume(RIGHT_PARENTH, "Expected ')' after parenthesized for clauses");

  Stmt body = statement();

  if (is_not_null_expr(increment)) {
    std::vector<Stmt> stmts;
    stmts.push_back(body); stmts.push_back(ExprStmt(increment));
    body = null_stmt;
    body = Block(stmts);
  }
  Stmt body_t = body;
  body = While(condition, std::make_shared<Stmt>(body_t));
  if (initializer) {
    std::vector<Stmt> stmts;
    stmts.push_back(*initializer); stmts.push_back(body);
    body = null_stmt;
    body = Block(stmts);
  }

  return body;
}
Stmt Parser::break_statement() {
  Error::consume(SEMI, "Expected ';' after statement");

  return BreakStmt(tokens[current - 1]);
}

Stmt Parser::continue_statement() {
  Error::consume(SEMI, "Expected ';' after statement");

  return ContinueStmt(tokens[current - 1]);
}

Stmt Parser::return_statement() {
  Token tok = tokens[current - 1];
  expr return_value = Literal(literal_t(_NULL));
  if (!match(SEMI)) {
    return_value = expression();
  }
  Error::consume(SEMI, "Expected ';' after statement");
  return ReturnStmt(tok, return_value);
}

Stmt Parser::label_statement() {
  Token name = Error::consume(IDENT, "Expected identifier after 'label' statement");
  Error::consume(COLON, "Expected ':' after label declaration");
  return Label(name);
}

Stmt Parser::goto_statement() {
  Token name = Error::consume(IDENT, "Expected identifier after 'goto' statement");
  Error::consume(SEMI, "Expected ';' after statement");
  return Goto(name);
}


expr Parser::expression() {
  return assignment();
}

expr Parser::assignment() {
  expr ex = lambda();

  if (match(EQUAL)) {
    Token token_to_report = tokens[current];
    current++;
    expr value = assignment();

    if (std::holds_alternative<Variable>(ex)) {
      Token identifier = std::get<Variable>(ex).name;
      return Assign(identifier, std::make_shared<expr>(value));
    } else if (std::holds_alternative<Get>(ex)) {
      Get get = std::get<Get>(ex);
      return Set(get.object, get.name, std::make_shared<expr>(value));
    }

    Error::error(token_to_report, "holy fucking shit i wanna sleep");
  }
  return ex;
}

expr Parser::lambda() {
  if (match(FUNC)) {
    Token tok = tokens[current];
    current++;
    Error::consume(LEFT_PARENTH, "Expected parameter list after lambda declaration");

    std::vector<Token> param;
    if (!match(RIGHT_PARENTH)) {
      do {
        if (param.capacity() >= MAX_ARGS)
          Error::error(tokens[current], "Parameter overflow");
        param.push_back(Error::consume(IDENT, "Expected identifier"));
      } while(match_consume(COMMA));
    }
    Error::consume(RIGHT_PARENTH, "Unterminated parameter list");

    Error::consume(LEFT_BRACE, "Expected '{' after lambda paramater list");
    Block block = Block(block_statement());
    
    std::stringstream ss;
    ss << "lambda." << tok.line << "." << param.capacity();
    Token name = Token(IDENT, ss.str().c_str(), nullptr, tok.line);
    std::shared_ptr<FunDecl> decl(new FunDecl(name, param, block.stmts));
    return Lambda(decl);
  }
  expr ex = or_expr();
  return ex;
}

expr Parser::or_expr() {
  expr ex = and_expr();
  while (match(OR)) {
    Token _operator = tokens[current];
    current++;
    expr right = and_expr();
    expr exT = ex;
    ex = Literal(literal_t(_NULL));
    ex = LogicalBin(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
  }
  return ex;
}
expr Parser::and_expr() {
  expr ex = equality();
  while (match(AND)) {
    Token _operator = tokens[current];
    current++;
    expr right = equality();
    expr exT = ex;
    ex = Literal(literal_t(_NULL));
    ex = LogicalBin(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
  }
  return ex;
}

expr Parser::equality() {
  expr ex = comparision();
  while (match(EXCL_EQUAL) || match(DOUBLE_EQUAL)) {
    Token _operator = tokens[current];
    current++;
    expr right = comparision();
    expr exT = ex;
    // we should assign ex to null expr because variants were made by not very smart people
    ex = Literal(literal_t(_NULL));
    ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
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
    ex = Literal(literal_t(_NULL));
    ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
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
    ex = Literal(literal_t(_NULL));
    ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
  }
  return ex;
}

expr Parser::factor() {
  expr ex = unary();
  while (match(SLASH) || match(STAR)) {
    Token _operator = tokens[current];
    current++;
    expr right = unary();
    expr exT = ex;
    ex = Literal(literal_t(_NULL));
    ex = Binary(std::make_shared<expr>(exT), std::make_shared<expr>(right), _operator);
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
  return call();
}

expr Parser::call() {
  expr ex = primary();
  
  while (true) {
    if (match(LEFT_PARENTH)) {
      current++;
      std::vector<expr> args;
      if (!match(RIGHT_PARENTH)) {
        do {
          if (args.capacity() >= MAX_ARGS) Error::error(tokens[current], "Argument overflow");
          args.push_back(expression());
        } while (match_consume(COMMA));
      }
      Token paren = Error::consume(RIGHT_PARENTH, "Unterminated function call");
      expr exT = ex;
      ex = Literal(literal_t(_NULL));
      ex = Call(std::make_shared<expr>(exT), paren, args);
    } else if (match(DOT)) {
      current++;
      Token prop = Error::consume(IDENT, "Expected propety name after '.'");
      expr exT = ex;
      ex = Literal(literal_t(_NULL));
      ex = Get(std::make_shared<expr>(exT), prop);
    } else break;
  }

  return ex;
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

  if (match(THIS)) {current++; return This(tokens[current - 1]);}

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
  if (tokens[current].type == type)
    return true;
  return false;
}

bool Parser::match_consume(token_type type) {
  if (__EOF()) return false;
  if (tokens[current].type == type) {
    current++;
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
  return "nah";
}
std::string PreatyPrinter::print_over(LogicalBin expr) {
  return "nah";
}
std::string PreatyPrinter::print_over(Call expr) {
  return "nah";
}
std::string PreatyPrinter::print_over(Lambda expr) {
  return "nah";
}
std::string PreatyPrinter::print_over(Get expr) {
  return "nah";
}
std::string PreatyPrinter::print_over(Set expr) {
  return "nah";
}
std::string PreatyPrinter::print_over(This expr) {
  return "nah";
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
