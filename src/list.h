#pragma once
#include "lexer.h"
#include <vector>

struct List {
  List(std::vector<literal_t> values);
  std::vector<literal_t> values;
  literal_t get(int index, Token tok);
  void set(int index, literal_t value);
};
