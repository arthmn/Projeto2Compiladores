#!/bin/bash

# Criar os arquivos .h
cat > include/token.h << 'EOF'
#ifndef TOKEN_H
#define TOKEN_H
#define MAX_LEXEMA 256
typedef enum { sEOF=0, sIDENTIF, sCTEINT, sSTRING, sMODULE, sGLOBALS, sLOCALS,
    sPROC, sMAIN, sINT, sPRINT, sSTART, sEND, sATRIB, sPONTOEVIRG, sDOISPONTOS,
    sVIRG, sABREPAR, sFECHAPAR, sIGUAL, sMAIOR, sMENOR, sAND, sOR, sNEG,
    sTRUE, sFALSE, sSCAN, sRET, sIF, sELSE, sWHILE, sLOOP, sFOR, sTO, sDO
} TokenCat;
typedef struct { TokenCat cat; char lexema[MAX_LEXEMA]; int linha; } Token;
#endif
EOF

cat > include/lex.h << 'EOF'
#ifndef LEX_H
#define LEX_H
#include "token.h"
void lex_init(const char *filename);
Token lex_next();
void lex_close();
#endif
EOF

cat > include/parser.h << 'EOF'
#ifndef PARSER_H
#define PARSER_H
void parse_program();
#endif
EOF

cat > include/symtab.h << 'EOF'
#ifndef SYMTAB_H
#define SYMTAB_H
typedef struct { char id[64]; char cat[32]; char tipo[32]; char escopo[256]; int extra; } Symbol;
void ts_init(); void ts_insert(const char *id, const char *cat, const char *tipo, const char *escopo, int extra);
Symbol *ts_lookup(const char *id); void ts_print(); void ts_free();
#endif
EOF

cat > include/opt.h << 'EOF'
#ifndef OPT_H
#define OPT_H
typedef struct { char fonte[256]; int log_tokens; int log_symtab; int log_trace; } Options;
void opts_parse(int argc, char **argv); Options *opts_get();
#endif
EOF

cat > include/log.h << 'EOF'
#ifndef LOG_H
#define LOG_H
#include "token.h"
void log_tokens_open(const char *base); void log_tokens(Token t); void log_tokens_close();
void log_trace_open(const char *base); void log_trace(const char *msg); void log_trace_close();
void log_symtab_open(const char *base); void log_symtab_entry(const char *esc, const char *id, const char *cat, const char *tipo, int extra); void log_symtab_close();
#endif
EOF

cat > include/diag.h << 'EOF'
#ifndef DIAG_H
#define DIAG_H
void diag_error(int linha, const char *esperado, const char *encontrado);
void diag_info(const char *msg);
#endif
EOF

echo "Headers criados em include/"

# Criar um programa mínimo em C (compilador simplificado)
cat > src/mini_compiler.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static FILE *input;
static int linha = 1;
static char token_lex[256];
static int token_cat;
static int log_tokens_flag = 0;

#define T_EOF 0
#define T_ID 1
#define T_NUM 2
#define T_STRING 3
#define T_MODULE 4
#define T_PROC 5
#define T_MAIN 6
#define T_START 7
#define T_END 8
#define T_PRINT 9
#define T_INT 10
#define T_GLOBALS 11
#define T_LOCALS 12
#define T_PONTOVIRG 13
#define T_ATRIB 14
#define T_ABREPAR 15
#define T_FECHAPAR 16
#define T_IGUAL 17
#define T_DOISPONTOS 18
#define T_VIRG 19
#define T_MAIOR 20
#define T_MENOR 21

static const char *token_names[] = {
    "sEOF", "sIDENTIF", "sCTEINT", "sSTRING", "sMODULE", "sPROC", "sMAIN",
    "sSTART", "sEND", "sPRINT", "sINT", "sGLOBALS", "sLOCALS", "sPONTOEVIRG",
    "sATRIB", "sABREPAR", "sFECHAPAR", "sIGUAL", "sDOISPONTOS", "sVIRG",
    "sMAIOR", "sMENOR"
};

static int is_keyword(const char *s) {
    if (strcmp(s, "module")==0) return T_MODULE;
    if (strcmp(s, "proc")==0) return T_PROC;
    if (strcmp(s, "main")==0) return T_MAIN;
    if (strcmp(s, "start")==0) return T_START;
    if (strcmp(s, "end")==0) return T_END;
    if (strcmp(s, "print")==0) return T_PRINT;
    if (strcmp(s, "int")==0) return T_INT;
    if (strcmp(s, "globals")==0) return T_GLOBALS;
    if (strcmp(s, "locals")==0) return T_LOCALS;
    return T_ID;
}

static void log_token() {
    if (log_tokens_flag) {
        printf("%d %s \"%s\"\n", linha, token_names[token_cat], token_lex);
    }
}

static void next_token() {
    int c = fgetc(input);
    while (isspace(c)) {
        if (c == '\n') linha++;
        c = fgetc(input);
    }
    if (c == EOF) {
        token_cat = T_EOF;
        strcpy(token_lex, "EOF");
        log_token();
        return;
    }
    if (isalpha(c)) {
        int i = 0;
        while (isalnum(c)) {
            token_lex[i++] = c;
            c = fgetc(input);
        }
        token_lex[i] = '\0';
        ungetc(c, input);
        token_cat = is_keyword(token_lex);
        log_token();
        return;
    }
    if (isdigit(c)) {
        int i = 0;
        while (isdigit(c)) {
            token_lex[i++] = c;
            c = fgetc(input);
        }
        token_lex[i] = '\0';
        ungetc(c, input);
        token_cat = T_NUM;
        log_token();
        return;
    }
    if (c == '"') {
        int i = 0;
        c = fgetc(input);
        while (c != '"' && c != EOF) {
            token_lex[i++] = c;
            c = fgetc(input);
        }
        token_lex[i] = '\0';
        token_cat = T_STRING;
        log_token();
        return;
    }
    token_lex[0] = c;
    token_lex[1] = '\0';
    switch(c) {
        case ';': token_cat = T_PONTOVIRG; break;
        case ':': token_cat = T_DOISPONTOS; break;
        case '=': token_cat = T_ATRIB; break;
        case '(': token_cat = T_ABREPAR; break;
        case ')': token_cat = T_FECHAPAR; break;
        case ',': token_cat = T_VIRG; break;
        case '>': token_cat = T_MAIOR; break;
        case '<': token_cat = T_MENOR; break;
        default: token_cat = T_EOF;
    }
    log_token();
}

static void match(int expected) {
    if (token_cat == expected) {
        next_token();
    } else {
        printf("Erro na linha %d: esperado %s, encontrado %s\n", 
               linha, token_names[expected], token_lex);
        exit(1);
    }
}

static void parse_program() {
    next_token();
    match(T_MODULE);
    match(T_ID);
    match(T_PONTOVIRG);
    if (token_cat == T_GLOBALS) {
        match(T_GLOBALS);
        while (token_cat == T_ID) {
            match(T_ID);
            if (token_cat == T_VIRG) match(T_VIRG);
            match(T_DOISPONTOS);
            if (token_cat == T_INT) match(T_INT);
            match(T_PONTOVIRG);
        }
    }
    match(T_PROC);
    match(T_MAIN);
    match(T_ABREPAR);
    match(T_FECHAPAR);
    if (token_cat == T_LOCALS) {
        match(T_LOCALS);
        while (token_cat == T_ID) {
            match(T_ID);
            if (token_cat == T_VIRG) match(T_VIRG);
            match(T_DOISPONTOS);
            if (token_cat == T_INT) match(T_INT);
            match(T_PONTOVIRG);
        }
    }
    match(T_START);
    while (token_cat != T_END && token_cat != T_EOF) {
        if (token_cat == T_PRINT) {
            match(T_PRINT);
            match(T_ABREPAR);
            if (token_cat == T_STRING || token_cat == T_ID || token_cat == T_NUM) {
                match(token_cat);
            }
            match(T_FECHAPAR);
            match(T_PONTOVIRG);
        } else if (token_cat == T_ID) {
            match(T_ID);
            match(T_ATRIB);
            if (token_cat == T_NUM || token_cat == T_ID) {
                match(token_cat);
            }
            match(T_PONTOVIRG);
        } else {
            printf("Erro na linha %d: comando invalido\n", linha);
            exit(1);
        }
    }
    match(T_END);
    printf("Programa valido.\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: mini_salc <arquivo.sal> [--tokens]\n");
        return 1;
    }
    if (argc >= 3 && strcmp(argv[2], "--tokens") == 0) {
        log_tokens_flag = 1;
    }
    input = fopen(argv[1], "r");
    if (!input) {
        printf("Erro ao abrir %s\n", argv[1]);
        return 1;
    }
    parse_program();
    fclose(input);
    return 0;
}
EOF

echo "Compilador criado em src/mini_compiler.c"

# Compilar
gcc -Wall -Wextra -std=c99 src/mini_compiler.c -o salc
chmod +x salc

echo "Compilacao concluida!"
echo
echo "Para testar:"
echo "  echo 'module Teste; proc main() start print(\"Oi\"); end' > teste.sal"
echo "  ./salc teste.sal --tokens"
