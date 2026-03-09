#include "expressions.h"
#include "lexer.h"
#include <memory>
#include <variant>
#include <iostream>

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

std::string LitOp::literal_to_string(literal_t lit) {
  if (auto val = std::get_if<std::string>(&lit)) {
    return *val;
  } else if (auto val = std::get_if<int>(&lit)) {
    return std::to_string(*val);
  } else if (auto val = std::get_if<double>(&lit)) {
    return std::to_string(*val);
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

literal_t LitOp::if_true(literal_t lit) {
  return std::visit([&](auto&& value){return if_true_over(value);}, lit);
}
literal_t LitOp::if_true_over(token_type lit) {
    switch (lit) {
      case _NULL: return literal_t(FALSE);
      case FALSE: return literal_t(FALSE);
      default:
      break;
    }
  return literal_t(TRUE);
}
literal_t LitOp::if_true_over(int lit) {if (lit == 0) return literal_t(FALSE);return literal_t(TRUE);}
literal_t LitOp::if_true_over(double lit) {if (lit == 0.0) return literal_t(FALSE);return literal_t(TRUE);}
literal_t LitOp::if_true_over(std::string lit) {return literal_t(TRUE);}

// oh my fucking god that's gonna be awful kill me
// please if someone who knows how to programm this normally tell me because i'm about to go crazy
literal_t LitOp::add(literal_t lit1, literal_t lit2) {
  if (auto val1 = std::get_if<int>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t((*val1) + (*val2));
    } else if (auto val2 = std::get_if<double>(&lit1)) {
      return literal_t(*val1 + *val2);
    }
  } else if (auto val1 = std::get_if<double>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t(*val1 + *val2);
    } else if (auto val2 = std::get_if<double>(&lit2)) {
      return literal_t(*val1 + *val2);
    }
  }
  return 0;
}
literal_t LitOp::sub(literal_t lit1, literal_t lit2) {
  if (auto val1 = std::get_if<int>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t(*val1 - *val2);
    } else if (auto val2 = std::get_if<double>(&lit2)) {
      return literal_t(*val1 - *val2);
    }
  } else if (auto val1 = std::get_if<double>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t(*val1 - *val2);
    } else if (auto val2 = std::get_if<double>(&lit2)) {
      return literal_t(*val1 - *val2);
    }
  }
  return 0;
}
literal_t LitOp::mul(literal_t lit1, literal_t lit2) {
  if (auto val1 = std::get_if<int>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t(*val1 * *val2);
    } else if (auto val2 = std::get_if<double>(&lit2)) {
      return literal_t(*val1 * *val2);
    }
  } else if (auto val1 = std::get_if<double>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t(*val1 * *val2);
    } else if (auto val2 = std::get_if<double>(&lit2)) {
      return literal_t(*val1 * *val2);
    }
  }
  return 0;
}
literal_t LitOp::div(literal_t lit1, literal_t lit2) {
  if (auto val1 = std::get_if<int>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t(*val1 / *val2);
    } else if (auto val2 = std::get_if<double>(&lit2)) {
      return literal_t(*val1 / *val2);
    }
  } else if (auto val1 = std::get_if<double>(&lit1)) {
    if (auto val2 = std::get_if<int>(&lit2)) {
      return literal_t(*val1 / *val2);
    } else if (auto val2 = std::get_if<double>(&lit2)) {
      return literal_t(*val1 / *val2);
    }
  }
  return 0;
}
literal_t LitOp::if_equal(literal_t lit1, literal_t lit2) {
  if (lit1 == lit2) return literal_t(TRUE);
  return literal_t(FALSE);
}
literal_t LitOp::greater(literal_t first, literal_t second) {
  if (std::holds_alternative<int>(first) && std::holds_alternative<int>(second)) {
    if (std::get<int>(first) >= std::get<int>(second)) return literal_t(TRUE);
  }
  if (std::holds_alternative<double>(first) && std::holds_alternative<double>(second)) {
    if (std::get<double>(first) >= std::get<double>(second)) return literal_t(TRUE) ;
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second)) {
    if (std::get<std::string>(first).length() > std::get<std::string>(second).length()) return literal_t(TRUE);
  }
  return literal_t(FALSE);
}
literal_t LitOp::greater_equal(literal_t first, literal_t second) {
  if (std::holds_alternative<int>(first) && std::holds_alternative<int>(second)) {
    if (std::get<int>(first) >= std::get<int>(second)) return literal_t(TRUE);
  }
  if (std::holds_alternative<double>(first) && std::holds_alternative<double>(second)) {
    if (std::get<double>(first) >= std::get<double>(second)) return literal_t(TRUE);
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second)) {
    if (std::get<std::string>(first).length() >= std::get<std::string>(second).length()) return literal_t(TRUE);
  }
  return literal_t(FALSE);
}
literal_t LitOp::less(literal_t first, literal_t second) {
  if (std::holds_alternative<int>(first) && std::holds_alternative<int>(second)) {
    if (std::get<int>(first) < std::get<int>(second)) return literal_t(TRUE);
  }
  if (std::holds_alternative<double>(first) && std::holds_alternative<double>(second)) {
    if (std::get<double>(first) < std::get<double>(second)) return literal_t(TRUE);
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second)) {
    if (std::get<std::string>(first).length() < std::get<std::string>(second).length()) return literal_t(TRUE);
  }
  return literal_t(FALSE);
}
literal_t LitOp::less_equal(literal_t first, literal_t second) {
  if (std::holds_alternative<int>(first) && std::holds_alternative<int>(second)) {
    if (std::get<int>(first) <= std::get<int>(second)) return literal_t(TRUE);
  }
  if (std::holds_alternative<double>(first) && std::holds_alternative<double>(second)) {
    if (std::get<double>(first) <= std::get<double>(second)) return literal_t(TRUE);
  }
  if (std::holds_alternative<std::string>(first) && std::holds_alternative<std::string>(second)) {
    if (std::get<std::string>(first).length() <= std::get<std::string>(second).length()) return literal_t(TRUE);
  }
  return literal_t(FALSE);
}
