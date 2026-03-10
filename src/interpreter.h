#pragma once
#include "lexer.h"
void report(int line = -1, const char* message = "unknown error", const char* where = "", const char* file = "IDK");
void report(Token token, const char* message = "unknown error");
void report_at_runtime(Token token, const char* msg, const char* file = "IDK");
