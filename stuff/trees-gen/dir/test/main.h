#include "main.cpp"

struct Fd {
    template<typename T> T func(Visitor<T> visit);
};
