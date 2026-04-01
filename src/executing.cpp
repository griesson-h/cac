#include "executing.h"
#include "classes.h"
#include "environment.h"
#include "expressions.h"
#include "function.h"
#include "interpreter.h"
#include "lexer.h"
#include "statements.h"
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>
#include <sstream>

std::shared_ptr<Environment> Interpreter::env(new Environment);
std::shared_ptr<Environment> Interpreter::backup_env(nullptr);
std::unordered_map<expr*, int> Interpreter::locals;
std::unordered_map<std::string, long> Interpreter::labels;
bool Interpreter::passed_first_init = false;
long Interpreter::current = 0;

bool Interpreter::in_loop = false;

void Interpreter::interpret(std::vector<Stmt> &statements) {
  if (!passed_first_init) {
    backup_env = env;
    init_foreigns();
  }
  passed_first_init = true;
  try {
    for (; current < statements.capacity(); current++) {
      execute(statements[current]);
    }
  } catch(RuntimeError e) {
    if (!backup_env) env = backup_env;
    report_at_runtime(e.token, e.what());
  }
}

void Interpreter::resolve(expr &ex, int depth) {
  locals[&ex] = depth;
}

void Interpreter::init_foreigns() {
  std::shared_ptr<func_t> time(new Foreigns::Time);
  env->define(Token(IDENT, "time", nullptr, 0), time);
  std::shared_ptr<func_t> to_string(new Foreigns::ToString);
  env->define(Token(IDENT, "str", nullptr, 0), to_string);
  std::shared_ptr<func_t> to_int(new Foreigns::ToInteger);
  env->define(Token(IDENT, "int", nullptr, 0), to_int);
}

void Interpreter::execute(Stmt &stmt) {
  std::visit([&](auto&& value){execute_over(value);}, stmt);
}

void Interpreter::execute_over(std::monostate&) {
  throw RuntimeError(null_token, "Somehow parser didn't throw a ParseError when failed");
}
void Interpreter::execute_over(PrintStmt &stmt) {
  literal_t arg = evaluate(stmt.exp);
  
  std::cout << LitOp::literal_to_string(arg) << std::flush;
  ///*TEMP*/ std::cout << std::endl;
}
void Interpreter::execute_over(ScanStmt &stmt) {
  std::string input;
  std::cin >> input;
  try {
    double x = std::stod(input);
    env->assign(stmt.name.name, x);
  } catch(std::invalid_argument) {
    env->assign(stmt.name.name, input);
  }
}
void Interpreter::execute_over(ExprStmt &stmt) {
  evaluate(stmt.exp);
}
void Interpreter::execute_over(Var &stmt) {
  literal_t value = _NULL;
  if (is_not_null_expr(stmt.initializer)) {
    value = evaluate(stmt.initializer);
  }

  env->define(stmt.name, value);
}
void Interpreter::execute_over(Block &stmt) {
  auto new_env = Environment(env);
  execute_block(stmt.stmts, new_env);
}

void Interpreter::execute_block(std::vector<Stmt> &stmts, Environment &new_env) {
  auto previous = env;
  env = std::make_shared<Environment>(new_env);
  for (auto& statement : stmts) {
    execute(statement);
  }
  env = previous;
}

void Interpreter::execute_over(IfStmt &stmt) {
  if (LitOp::if_true(evaluate(stmt.condition))) {
    execute(*stmt.then);
  } else if (stmt.elsestmt) {
    execute(*stmt.elsestmt);
  }
}
void Interpreter::execute_over(While &stmt) {
  in_loop = true;
  while (LitOp::if_true(evaluate(stmt.condition))) {
    try {
      execute(*stmt.body);
    } catch (Break) {
      break;
    }
  }
  in_loop = false;
}
void Interpreter::execute_over(BreakStmt &stmt) {
  if (!in_loop)
    throw RuntimeError(stmt.tok, "Attempted to break out of non-loop scope");
  throw Break();
}
void Interpreter::execute_over(ContinueStmt &stmt) {
  if (!in_loop)
    throw RuntimeError(stmt.tok, "'continue' must be called inside a loop");
  throw RuntimeError(stmt.tok, "Continue statements are bad practice and\ntherefore are not supported (totally not because they break for loops)");
  //throw Continue();
}
void Interpreter::execute_over(FunDecl &stmt) {
  std::shared_ptr<func_t> function(new Function(stmt, env, false));
  env->define(stmt.name, function);
}
void Interpreter::execute_over(ReturnStmt &stmt) {
  literal_t return_value = evaluate(stmt.return_value);
  throw Return(return_value);
}
Interpreter::Return::Return(literal_t value) : value(value) {}
void Interpreter::execute_over(ClassDecl &stmt) {
  env->define(stmt.name, _NULL);

  std::unordered_map<std::string, std::shared_ptr<func_t>> methods;
  for (auto& method: stmt.methods) {
    std::shared_ptr<func_t> function(new Function(method, env, (method.name.lexeme == stmt.name.lexeme)));
    methods[method.name.lexeme] = function;
  }

  std::shared_ptr<func_t> klass (new Classs(stmt.name, methods));
  env->assign(stmt.name, klass);
}

void Interpreter::execute_over(Label &stmt) {}
void Interpreter::execute_over(Goto &stmt) {
  try {
    current = labels.at(stmt.name.lexeme);
  } catch (std::out_of_range) {
    std::stringstream ss;
    ss << "Undefined label '" << stmt.name.lexeme << "'";
    throw RuntimeError(stmt.name, ss.str());
  }
}


literal_t Interpreter::evaluate(expr &ex) {
  // evil c++ visitor (never let em in...)
  return std::visit([&](auto&& value){return evaluate_over(value);}, ex);
}

literal_t Interpreter::evaluate_over(Literal &ex) {
  return ex.literal;
}

literal_t Interpreter::evaluate_over(Group &ex) {
  return evaluate(*ex.exp);
}

literal_t Interpreter::evaluate_over(Unary &ex) {
  literal_t right = evaluate(*(ex.postfix));

  switch (ex._operator.type) {
    case MINUS:
      return LitOp::negative(right);
      break;
    case EXCL:
      return LitOp::negative(LitOp::bool_make_lit(LitOp::if_true(right)));
    default:
      break;
  }
  
  std::cout << "how the fuck" << std::endl;
  return literal_t(_NULL);
}

literal_t Interpreter::evaluate_over(Binary &ex) {
  literal_t first = evaluate(*(ex.first));
  literal_t second = evaluate(*(ex.second));

  switch (ex._operator.type) {
    case EXCL_EQUAL:
      return LitOp::negative(LitOp::if_equal(first, second));
    case DOUBLE_EQUAL:
      return LitOp::if_equal(first, second);
    case GREATER:
      return LitOp::greater(first, second);
    case GREATER_EQUAL:
      return LitOp::greater_equal(first, second);
    case LESS:
      return LitOp::less(first, second);
    case LESS_EQUAL:
      return LitOp::less_equal(first, second);
    case PLUS:
      if (LitOp::is_numbers(first, second)) {
        return LitOp::add(first, second);
      } else if (LitOp::contains<std::string>(first) && LitOp::contains<std::string>(second)) {
        return literal_t(std::get<std::string>(first).append(std::get<std::string>(second)));
      } else {
        throw RuntimeError(ex._operator, "Operants must be either numbers or strings");
      }
      break;
    case MINUS:
      if (!(LitOp::is_numbers(first, second)))
        goto err_not_number;
      return LitOp::sub(first, second);
    case SLASH: {
      if (!(LitOp::is_numbers(first, second)))
        goto err_not_number;
      literal_t ret = LitOp::div(first, second);
      if (LitOp::contains<token_type>(ret)) throw RuntimeError(ex._operator, "Division by zero is undefined");
      return ret;
    } // yes to initialize a variable in a switch statement i gotta use this weird braces on case statement idk why c++'s weird ig (also it's 1:15 pm i needa go to bed)
      // it's needed to initialize scope you idiot
    case STAR:
      if (!(LitOp::is_numbers(first, second)))
        goto err_not_number;
      return LitOp::mul(first, second);
    default:
      break;
  }

  std::cout << "how the fuck" << std::endl;
  return literal_t(_NULL);
err_not_number:
  throw RuntimeError(ex._operator, "Operants must be numbers");
}
Interpreter::RuntimeError::RuntimeError(Token token, std::string msg) : token(token), msg(msg) {}
const char* Interpreter::RuntimeError::what() const noexcept {
  return msg.c_str();
}

literal_t Interpreter::lookup_variables(Token name, expr &ex) {
  try {
    int distance = locals.at(reinterpret_cast<expr*>(&ex));
    return env->get_at(distance, name);
  } catch(std::out_of_range) {
    return backup_env->get(name);
  }
}
literal_t Interpreter::evaluate_over(Variable &ex) {
  return lookup_variables(ex.name, reinterpret_cast<expr&>(ex));
}
literal_t Interpreter::evaluate_over(This &ex) {
  return lookup_variables(ex.tok, reinterpret_cast<expr&>(ex));
}

literal_t Interpreter::evaluate_over(Assign &ex) {
  literal_t value = evaluate(*ex.value);
  try {
    int distance = locals.at(reinterpret_cast<expr*>(&ex));
    env->assign_at(distance, ex.identifier, value);
  } catch(std::out_of_range) {
    backup_env->assign(ex.identifier, value);
  }
  return value;
}
literal_t Interpreter::evaluate_over(LogicalBin &ex) {
  literal_t first = evaluate(*ex.first);

  if (ex._operator.type == OR) {
    if (LitOp::if_true(first)) return first;
  } else {
    if (!LitOp::if_true(first)) return first;
  }

  return evaluate(*ex.second);
}
literal_t Interpreter::evaluate_over(Call &ex) {
  literal_t callee = evaluate(*ex.callee);
  
  if (!LitOp::contains<std::shared_ptr<func_t>>(callee)) {
    throw RuntimeError(ex.tok, "Cannot call a non-callable expression");
  }
  auto &fun = std::get<std::shared_ptr<func_t>>(callee);

  std::vector<literal_t> args;
  for (auto& arg : ex.Args) {
    args.push_back(evaluate(arg));
  }

  if (args.capacity() != fun->arity()) {
    std::stringstream ss;
    ss << "Expected " << fun->arity() << " arguments to be passed, but found " << args.capacity();
    throw RuntimeError(ex.tok, ss.str());
  }

  return fun->call(args, ex.tok);
}
literal_t Interpreter::evaluate_over(Get &ex) {
  literal_t object = evaluate(*ex.object);
  if (auto obj = std::get_if<std::shared_ptr<Instance>>(&object)) {
    return (*obj)->get(ex.name);
  }

  throw RuntimeError(ex.name, "Cannot use '.' (get property) operation on a non-instance value");
}
literal_t Interpreter::evaluate_over(Set &ex) {
  literal_t object = evaluate(*ex.object);

  if (!LitOp::contains<std::shared_ptr<Instance>>(object)) {
    throw RuntimeError(ex.name, "Cannot use '.' (set property) operation on a non-instance value");
  }

  literal_t value = evaluate(*ex.value);
  std::get<std::shared_ptr<Instance>>(object)->set(ex.name, value);
  return value;
}
literal_t Interpreter::evaluate_over(Lambda &ex) {
  std::stringstream ss;
  std::shared_ptr<func_t> lambda(new Function(*ex.decl, env, false));
  return lambda;
}
