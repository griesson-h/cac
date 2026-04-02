#include "list.h"
#include "executing.h"
#include <iostream>

List::List(std::vector<literal_t> values) : values(values) {}
literal_t List::get(int index, Token tok) {
  if (index >= values.size())
    throw Interpreter::RuntimeError(tok, "Index out of list range");
  return values[index];
}
void List::set(int index, literal_t value) {
  if (index >= values.size()) {
    auto cap = values.size();
    for (int i = 0; i < index - cap; i++) {
      values.push_back(_NULL);
    }
    values.push_back(value);
  }
  values[index] = value;
}
