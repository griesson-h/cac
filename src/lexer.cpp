#include "lexer.h"
#include <sstream>
#include <string>
#include "interpreter.h"
#include <iostream>

std::vector<Token> tokens;
std::map<std::string, token_type> reserved_keywords = {
  {"and", AND},
  {"or", OR},
  {"var", VAR},
  {"func", FUNC},
  {"class", CLASS},
  {"if", IF},
  {"else", ELSE},
  {"for", FOR},
  {"while", WHILE},
  {"true", TRUE},
  {"false", FALSE},
  {"this", THIS},
  {"null", _NULL}
};

Token::Token(token_type i_type, std::string i_lexeme, void* i_literal, int i_line): type(i_type), lexeme(i_lexeme), line(i_line) {
  literal = i_literal;
}

std::string Token::to_string() {
  std::stringstream ss;
  ss << this->type << '\t' << this->lexeme << '\t' << this->literal;
  return ss.str();
}

std::vector<Token> lex_tokens(const std::string source) {
  std::cout << "source length: " << source.length() << " " << std::endl;
  int begining = 0;
  int current = 0;
  int current_line = 1;

  const auto __EOF = [&current, source](){
    return current >= source.length();
  };
  const auto add_token = [begining, current, current_line, source](token_type type, void* literal){
    const std::string text = source.substr(begining, current);
    tokens.push_back(Token(type, text, literal, current_line));
    Token(type, text, literal, current_line).to_string();
  };
  const auto check = [&current, source](char expected){
    if (current <= source.length()) return false;
    if (source.at(current) != expected) return false;

    current++;
    return true;
  };
  const auto peek_safely = [&current, source](int how_far = 0){
    // for some reason it's important for language hackers to know
    // how far my lexer peeks so yeah by far it's 2
    if (current + how_far >= source.length()) return '\0';
    return source.at(current + how_far);
  };
  const auto lex_string = [&](){
    const auto report_line = current_line;
    while (peek_safely() != '"' && !__EOF()) {
      if (peek_safely() == '\n') current_line++;
      current++;
    }

    if (__EOF()) {
      report(report_line, "Unterminated string");
    }

    current++;

    const auto literal = source.substr(begining + 1, current - 1);
    add_token(STRING, (void*)&literal);
  };
  const auto is_digit = [&](char c){
    return c >= '0' && c <= '9';
  };
  const auto is_alphabetic = [&](char c){
    return (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || c == '_';
  };
  const auto lex_numtype = [&](){
    while (is_digit(peek_safely())) current++;

    if (peek_safely() == '.' && is_digit(peek_safely(1))) {
      std::cout << "FLOAT" << std::endl;
      current++;
      while (is_digit(peek_safely())) current++;
      const auto num_string = source.substr(begining, current - begining);
      const int num = std::stod(num_string);
      add_token(NUM, (void*)&num);
    } else {
      std::cout << "INT" << std::endl;
      const auto num_string = source.substr(begining, current - begining);
      const int num = std::stoi(num_string);
      add_token(NUM, (void*)&num); // i fucking hate void pointers, i'm gonna have to write those stupid references and dereferences each
    }                                             // time i wanna access them over and over again, and also i can already see the segfaults annoying the hell out of me
  };
  const auto lex_identifier = [&](){
    while (is_alphabetic(peek_safely())) {
      std::cout << peek_safely() << std::endl;
      current++; 
    }

    std::cout << peek_safely() << std::endl;
    const auto ident_string = source.substr(begining, current - begining);
    std::cout << ident_string << std::endl;
    if (reserved_keywords.contains(ident_string)) {
      std::cout << "RESERVED" << std::endl;
      add_token(reserved_keywords.at(ident_string), nullptr); 
    }
    else
      add_token(IDENT, nullptr);
  };

  while (!__EOF()) {
    begining = current;

    const char ch = source.at(current++);
    std::cout << ch << std::endl;
    switch(ch) {
      case '(': add_token(LEFT_PARENTH, nullptr); break;
      case ')': add_token(RIGHT_PARENTH, nullptr); break;
      case '{': add_token(LEFT_BRACE, nullptr); break;
      case '}': add_token(RIGHT_BRACE, nullptr); break;
      case '[': add_token(LEFT_SQR_BR, nullptr); break;
      case ']': add_token(RIGHT_SQR_BR, nullptr); break;
      case ',': add_token(COMMA, nullptr); break;
      case '.': add_token(DOT, nullptr); break;
      case '-': add_token(MINUS, nullptr); break;
      case '+': add_token(PLUS, nullptr); break;
      case '*': add_token(STAR, nullptr); break;
      case ';': add_token(SEMI, nullptr); break;

      case '!':
        add_token(check('=') ? EXCL_EQUAL : EXCL, nullptr);
        break;
      case '=':
        add_token(check('=') ? DOUBLE_EQUAL : EQUAL, nullptr);
        break;
      case '<':
        add_token(check('=') ? LESS_EQUAL : LESS, nullptr);
        break;
      case '>':
        add_token(check('=') ? GREATER_EQUAL : GREATER, nullptr);
        break;
      case '/':
        if (check('/')) {
          while (peek_safely() != '\n' && !__EOF()) current++;
        } else {
          add_token(SLASH, nullptr);
        }
      case '"':
        std::cout << "STRING" << std::endl;
        lex_string();
        break;
      case ' ':
      case '\r':
      case '\t':
        break;
      case '\n':
        current_line++;
        break;
      default:
        std::cout << "defuault" << std::endl;
        if (ch >= '0' && ch <= '9') {
          lex_numtype();
        } else if (is_alphabetic(ch)) {
          std::cout << "IDENT" << std::endl;
          lex_identifier();
        } else {
          report(current_line, "Unexpected character");
          break;
        }
    }
  }

  add_token(_EOF, nullptr);
  return tokens;
}

