#pragma once
#include "lexer.h"

#define assertm(exp, msg) assert((void(msg), exp))

extern bool failed;
extern const bool debuginfo;
void report_warning(int line = -1, const char* message = "unknown warning");
void report(int line = -1, const char* message = "unknown error", const char* where = "");
void report(Token token, const char* message = "unknown error");
void report_at_runtime(Token token, std::string msg);
