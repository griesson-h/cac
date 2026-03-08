#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <variant>

enum token_type {
  LEFT_PARENTH, RIGHT_PARENTH,
  LEFT_BRACE, RIGHT_BRACE,
  LEFT_SQR_BR, RIGHT_SQR_BR,

  COMMA, DOT, MINUS, PLUS, SLASH, STAR, SEMI,

  EXCL, EXCL_EQUAL,
  EQUAL, DOUBLE_EQUAL,
  LESS, LESS_EQUAL,
  GREATER, GREATER_EQUAL,

  IDENT, STRING, INT, FLOAT,

  AND, OR,
  FOR, WHILE, IF, ELSE,
  VAR, FUNC, CLASS,
  TRUE, FALSE, THIS, _NULL,
  RETURN, _EOF
};

using literal_t = std::variant<token_type, int, double, std::string>;

struct Token {
    const token_type type;
    const std::string lexeme;
    literal_t literal;
    const int line;

    Token(token_type i_type, std::string i_lexeme, const literal_t* i_literal, int i_line);
    Token operator=(Token tok);
    std::string to_string();
};

extern std::vector<Token> tokens;
extern std::map<std::string, token_type> reserved_keywords;

std::vector<Token> lex_tokens(const std::string source);
