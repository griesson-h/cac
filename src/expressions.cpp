#include "executing.h"
#include "function.h"
#include "expressions.h"
#include "interpreter.h"
#include "statements.h"
#include "lexer.h"
#include "classes.h"
#include <memory>
#include <variant>
#include <iostream>
#include <vector>

Assign::Assign(Token identifier, std::shared_ptr<expr> value) : identifier(identifier) {
  value.swap(this->value);
}
Literal::Literal(literal_t literal) : literal(literal) {}
Group::Group(std::shared_ptr<::expr> exp) {
  exp.swap(this->exp);
} // "unknown type expr, did you mean ::expr?" compiler wake up youre drunk
Binary::Binary(std::shared_ptr<expr> first, std::shared_ptr<expr> second, Token _operator) : _operator(_operator) {
  first.swap(this->first);
  second.swap(this->second);
}
Unary::Unary(Token _operator, std::shared_ptr<expr> postfix) : _operator(_operator) {
  postfix.swap(this->postfix);
}
Variable::Variable(Token name) : name(name) {}
Call::Call(std::shared_ptr<expr> callee, Token tok, std::vector<expr> Args) : tok(tok), Args(Args) {
  callee.swap(this->callee);
}
LogicalBin::LogicalBin(std::shared_ptr<expr> first, std::shared_ptr<expr> second, Token _operator) : _operator(_operator) {
  first.swap(this->first);
  second.swap(this->second);
}
Lambda::Lambda(std::shared_ptr<FunDecl> decl) : decl(decl) {}
Get::Get(std::shared_ptr<expr> object, Token name) : object(object), name(name) {}
Set::Set(std::shared_ptr<expr> object, Token name, std::shared_ptr<expr> value) : object(object), name(name), value(value) {}
This::This(Token tok) : tok(tok) {}
Super::Super(Token tok, Token method) : tok(tok), method(method) {}

bool is_not_null_expr(expr ex) { // that's a BAD way of saying something is null but, uh, here we are
  switch (ex.index()) {          // basicly if it's just 'Literal(literal_t(_NULL))' then true but very verbose because c++
    case LITERAL:
      switch (std::get<Literal>(ex).literal.index()) {
        case RESERV: switch (std::get<token_type>(std::get<Literal>(ex).literal)) {case _NULL: return false; default: break;} default: break;
      }
  }
  return true;
}

std::string LitOp::literal_to_string(literal_t lit) {
  if (auto val = std::get_if<std::string>(&lit)) {
    return *val;
  } else if (auto val = std::get_if<int>(&lit)) {
    return std::to_string(*val);
  } else if (auto val = std::get_if<double>(&lit)) {
    return std::to_string(*val);
  } else if (auto val = std::get_if<std::shared_ptr<func_t>>(&lit)) {
    return (*val)->to_string();
  } else if (auto val = std::get_if<std::shared_ptr<class_t>>(&lit)) {
    return (*val)->to_string();
  } else if (auto val = std::get_if<std::shared_ptr<Instance>>(&lit)) {
    return (*val)->to_string();
  } else {
    switch (std::get<token_type>(lit)) {
      case TRUE: return "true"; break;
      case FALSE: return "false"; break;
      case _NULL: return "null"; break;
      default: return "";
    }
  }
}

literal_t LitOp::negative(literal_t lit) {
  if (auto val = std::get_if<int>(&lit)) {
    literal_t ret = -*val;
    return ret;
  } else if (auto val = std::get_if<double>(&lit)) {
    literal_t ret = -*val;
    return ret;
  } else if (auto val = std::get_if<token_type>(&lit)) {
    if (*val == FALSE) return literal_t(TRUE);
    if (*val == TRUE) return literal_t(FALSE);
  }
  return 0;
}

bool LitOp::if_true(literal_t lit) {
  return std::visit([&](auto&& value){return if_true_over(value);}, lit);
}
bool LitOp::if_true_over(token_type lit) {
    switch (lit) {
      case _NULL: return false;
      case FALSE: return false;
      default:
      break;
    }
  return true;
}
bool LitOp::if_true_over(int lit) {if (lit == 0) return false;return true;}
bool LitOp::if_true_over(double lit) {if (lit == 0.0) return false;return true;}
bool LitOp::if_true_over(std::string lit) {return true;}
bool LitOp::if_true_over(std::shared_ptr<func_t> lit) {return true;}
bool LitOp::if_true_over(std::shared_ptr<Instance> lit) {return true;}

// oh my fucking god that's gonna be awful kill me
// please if someone who knows how to programm this normally tell me because i'm about to go crazy
//
// me the day after: operator overloading exists
//
// fuck it, it works, i don't care
//
// TODO: optimize the fuck out of this shit
literal_t LitOp::add(literal_t lit1, literal_t lit2) {
  switch (lit1.index()) {
    case INT_T:
      switch (lit2.index()) {
        case INT_T: return std::get<int>(lit1) + std::get<int>(lit2);
        case DOUBLE:return std::get<int>(lit1) + std::get<double>(lit2);
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (lit2.index()) {
        case INT_T:return std::get<double>(lit1) + std::get<int>(lit2);
        case DOUBLE:return std::get<double>(lit1) + std::get<double>(lit2);
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  return 0;
}
literal_t LitOp::sub(literal_t lit1, literal_t lit2) {
  switch (lit1.index()) {
    case INT_T:
      switch (lit2.index()) {
        case INT_T: return std::get<int>(lit1) - std::get<int>(lit2);
        case DOUBLE:return std::get<int>(lit1) - std::get<double>(lit2);
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (lit2.index()) {
        case INT_T:return std::get<double>(lit1) - std::get<int>(lit2);
        case DOUBLE:return std::get<double>(lit1) - std::get<double>(lit2);
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  return 0;
}
literal_t LitOp::mul(literal_t lit1, literal_t lit2) {
  switch (lit1.index()) {
    case INT_T:
      switch (lit2.index()) {
        case INT_T: return std::get<int>(lit1) * std::get<int>(lit2);
        case DOUBLE:return std::get<int>(lit1) * std::get<double>(lit2);
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (lit2.index()) {
        case INT_T:return std::get<double>(lit1) * std::get<int>(lit2);
        case DOUBLE:return std::get<double>(lit1) * std::get<double>(lit2);
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  return 0;
}
literal_t LitOp::div(literal_t lit1, literal_t lit2) {
  if (lit2 == literal_t(0) || lit2 == literal_t(0.0)) goto err_div_zero;
  switch (lit1.index()) {
    case INT_T:
      switch (lit2.index()) {
        case INT_T:
          return std::get<int>(lit1) / std::get<int>(lit2);
        case DOUBLE: 
          return std::get<int>(lit1) / std::get<double>(lit2);
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (lit2.index()) {
        case INT_T:
          return std::get<double>(lit1) / std::get<int>(lit2);
        case DOUBLE: 
          return std::get<double>(lit1) / std::get<double>(lit2);
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  return 0;
err_div_zero:
  return _NULL; // then we'll just check for this value and depending on this throw a runtime error
}
literal_t LitOp::if_equal(literal_t lit1, literal_t lit2) {
  switch (lit1.index()) {
    case DOUBLE:
      switch (lit2.index()) {
        case DOUBLE:
          return bool_make_lit(std::get<DOUBLE>(lit1) == std::get<DOUBLE>(lit2));
        case INT_T:
          return bool_make_lit(std::get<DOUBLE>(lit1) == std::get<INT_T>(lit2));
      }
    default:
      if (lit2.index() == DOUBLE) return std::get<INT_T>(lit1) == std::get<DOUBLE>(lit2);
  }
  if (lit1 == lit2) return literal_t(TRUE);
  return literal_t(FALSE);
}
literal_t LitOp::greater(literal_t first, literal_t second) {
  switch (first.index()) {
    case INT_T:
      switch (second.index()) {
        case INT_T: return bool_make_lit(std::get<int>(first) > std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<int>(first) > std::get<double>(second));
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (second.index()) {
        case INT_T:return bool_make_lit(std::get<double>(first) > std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<double>(first) > std::get<double>(second));
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second))
    if (std::get<std::string>(first).length() > std::get<std::string>(second).length()) return literal_t(TRUE);
  return literal_t(FALSE);
}
literal_t LitOp::greater_equal(literal_t first, literal_t second) {
  switch (first.index()) {
    case INT_T:
      switch (second.index()) {
        case INT_T: return bool_make_lit(std::get<int>(first) >= std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<int>(first) >= std::get<double>(second));
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (second.index()) {
        case INT_T:return bool_make_lit(std::get<double>(first) >= std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<double>(first) >= std::get<double>(second));
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second))
    if (std::get<std::string>(first).length() >= std::get<std::string>(second).length()) return literal_t(TRUE);
  return literal_t(FALSE);
}
literal_t LitOp::less(literal_t first, literal_t second) {
  switch (first.index()) {
    case INT_T:
      switch (second.index()) {
        case INT_T: return bool_make_lit(std::get<int>(first) < std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<int>(first) < std::get<double>(second));
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (second.index()) {
        case INT_T:return bool_make_lit(std::get<double>(first) < std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<double>(first) < std::get<double>(second));
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second))
    if (std::get<std::string>(first).length() < std::get<std::string>(second).length()) return literal_t(TRUE);
  return literal_t(FALSE);
}
literal_t LitOp::less_equal(literal_t first, literal_t second) {
  switch (first.index()) {
    case INT_T:
      switch (second.index()) {
        case INT_T: return bool_make_lit(std::get<int>(first) <= std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<int>(first) <= std::get<double>(second));
        case STRING_T: break;
      }
      break;
    case DOUBLE:
      switch (second.index()) {
        case INT_T:return bool_make_lit(std::get<double>(first) <= std::get<int>(second));
        case DOUBLE:return bool_make_lit(std::get<double>(first) <= std::get<double>(second));
        case STRING_T: break;
      }
    case STRING_T: break;
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second))
    if (std::get<std::string>(first).length() <= std::get<std::string>(second).length()) return literal_t(TRUE);

  return literal_t(FALSE);
}

bool LitOp::is_numbers(literal_t lit1, literal_t lit2) {
  return ((LitOp::contains<int>(lit1) || LitOp::contains<double>(lit1)) &&
          (LitOp::contains<int>(lit2) || LitOp::contains<double>(lit2)));
}

std::shared_ptr<func_t> LitOp::get_callable(literal_t lit, Token tok) {
  if (auto val = std::get_if<std::shared_ptr<func_t>>(&lit)) {
    return *val;
  } else if (auto val = std::get_if<std::shared_ptr<class_t>>(&lit)) {
    return std::shared_ptr<func_t>(*val);
  } else {
    throw Interpreter::RuntimeError(tok, "Cannot call a non-callable expression");
  }
}
