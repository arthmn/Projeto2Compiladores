#ifndef LEX_H
#define LEX_H
#include "token.h"
void lex_init(const char *filename);
Token lex_next();
void lex_close();
#endif
