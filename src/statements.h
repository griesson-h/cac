#pragma once
#include "expressions.h"
#include "lexer.h"
#include <memory>
#include <variant>
#include <vector>

struct ExprStmt;
struct PrintStmt;
struct Var;
struct Block;
struct IfStmt;
struct While;
struct ScanStmt;
struct FunDecl;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;
struct ClassDecl;
struct Label;
struct Goto;
struct Include;


using Stmt = std::variant<std::monostate, ExprStmt, PrintStmt, Var, Block, IfStmt, While, ScanStmt, FunDecl, ReturnStmt, BreakStmt, ContinueStmt, ClassDecl, Label, Goto, Include>;


struct ExprStmt {
  ExprStmt(expr exp);
  expr exp;
};
struct PrintStmt {
  PrintStmt(expr exp);
  expr exp;
};
struct Var {
  Var(Token name, expr initializer);
  Token name;
  expr initializer;
};
struct Block {
  Block(std::vector<Stmt> stmts);
  std::vector<Stmt> stmts;
};
struct IfStmt {
  IfStmt(expr condition, std::shared_ptr<Stmt> then, std::shared_ptr<Stmt> elsestmt);
  expr condition;
  std::shared_ptr<Stmt> then;
  std::shared_ptr<Stmt> elsestmt;
};
struct While {
  While(expr condition, std::shared_ptr<Stmt> body);
  expr condition;
  std::shared_ptr<Stmt> body;
};
struct ScanStmt {
  ScanStmt(Variable name);
  Variable name;
};
struct FunDecl {
  FunDecl(Token name, std::vector<Token> param, std::vector<Stmt> body);
  Token name;
  std::vector<Token> param;
  std::vector<Stmt> body;
};
struct ReturnStmt {
  ReturnStmt(Token tok, expr return_value);
  Token tok;
  expr return_value;
};
struct BreakStmt {
  BreakStmt(Token tok);
  Token tok;
};
struct ContinueStmt {
  ContinueStmt(Token tok);
  Token tok;
};
struct ClassDecl {
  ClassDecl(Token name, std::vector<FunDecl> methods, expr base);
  Token name;
  expr base;
  std::vector<FunDecl> methods;
};
struct Label {
  Label(Token name);
  Token name;
};
struct Goto {
  Goto(Token name);
  Token name;
};
struct Include {
  Include(Token tok, std::string path);
  Token tok;
  std::string path;
};

//struct InnerExprStmt {Stmt stmt; InnerExprStmt(Stmt stmt); void operator=(Stmt stmt);};

extern Stmt null_stmt;
