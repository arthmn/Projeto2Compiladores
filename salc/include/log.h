#ifndef LOG_H
#define LOG_H
#include "token.h"
void log_tokens_open(const char *base); void log_tokens(Token t); void log_tokens_close();
void log_trace_open(const char *base); void log_trace(const char *msg); void log_trace_close();
void log_symtab_open(const char *base); void log_symtab_entry(const char *esc, const char *id, const char *cat, const char *tipo, int extra); void log_symtab_close();
#endif
