#pragma once
#include "lexer.h"

#define assertm(exp, msg) assert((void(msg), exp))

extern bool failed;
void report(int line = -1, const char* message = "unknown error", const char* where = "", const char* file = "IDK");
void report(Token token, const char* message = "unknown error");
void report_at_runtime(Token token, std::string msg, const char* file = "IDK");
