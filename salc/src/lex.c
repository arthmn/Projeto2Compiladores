#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "log.h"
#include "lex.h"
#include "diag.h"

static FILE *fonte = NULL;
static int linha = 1;
static int c = ' ';

static void next_char() { c = fgetc(fonte); if (c == '\n') linha++; }

static Token make_token(TokenCat cat, const char *lex) {
    Token t = {cat, "", linha};
    strcpy(t.lexema, lex);
    log_tokens(t);
    return t;
}

static TokenCat keyword(const char *lex) {
    if (strcmp(lex, "module")==0) return sMODULE;
    if (strcmp(lex, "globals")==0) return sGLOBALS;
    if (strcmp(lex, "locals")==0) return sLOCALS;
    if (strcmp(lex, "proc")==0) return sPROC;
    if (strcmp(lex, "fn")==0) return sFN;
    if (strcmp(lex, "main")==0) return sMAIN;
    if (strcmp(lex, "int")==0) return sINT;
    if (strcmp(lex, "bool")==0) return sBOOL;
    if (strcmp(lex, "char")==0) return sCHAR;
    if (strcmp(lex, "if")==0) return sIF;
    if (strcmp(lex, "else")==0) return sELSE;
    if (strcmp(lex, "match")==0) return sMATCH;
    if (strcmp(lex, "when")==0) return sWHEN;
    if (strcmp(lex, "otherwise")==0) return sOTHERWISE;
    if (strcmp(lex, "for")==0) return sFOR;
    if (strcmp(lex, "to")==0) return sTO;
    if (strcmp(lex, "step")==0) return sSTEP;
    if (strcmp(lex, "do")==0) return sDO;
    if (strcmp(lex, "loop")==0) return sLOOP;
    if (strcmp(lex, "while")==0) return sWHILE;
    if (strcmp(lex, "until")==0) return sUNTIL;
    if (strcmp(lex, "print")==0) return sPRINT;
    if (strcmp(lex, "scan")==0) return sSCAN;
    if (strcmp(lex, "ret")==0) return sRET;
    if (strcmp(lex, "true")==0) return sTRUE;
    if (strcmp(lex, "false")==0) return sFALSE;
    if (strcmp(lex, "start")==0) return sSTART;
    if (strcmp(lex, "end")==0) return sEND;
    if (strcmp(lex, "and")==0) return sAND;
    if (strcmp(lex, "or")==0) return sOR;
    if (strcmp(lex, "not")==0) return sNEG;
    return sIDENTIF;
}

void lex_init(const char *filename) {
    fonte = fopen(filename, "r");
    if (!fonte) { printf("Erro ao abrir %s\n", filename); exit(1); }
    next_char();
}

void lex_close() { if (fonte) fclose(fonte); }

Token lex_next() {
    while (isspace(c)) next_char();
    if (c == EOF) return make_token(sEOF, "EOF");
    if (isalpha(c) || c == '_') {
        char buf[256]; int i=0;
        while (isalnum(c) || c=='_') { buf[i++]=c; next_char(); }
        buf[i]='\0';
        return make_token(keyword(buf), buf);
    }
    if (isdigit(c)) {
        char buf[256]; int i=0;
        while (isdigit(c)) { buf[i++]=c; next_char(); }
        buf[i]='\0';
        return make_token(sCTEINT, buf);
    }
    if (c == '"') {
        char buf[256]; int i=0;
        next_char();
        while (c != '"' && c != EOF) { buf[i++]=c; next_char(); }
        buf[i]='\0';
        next_char();
        return make_token(sSTRING, buf);
    }
    if (c == '\'') {
        char buf[4];
        buf[0] = '\'';
        next_char();
        buf[1] = c;
        next_char();
        if (c != '\'') diag_error(linha, "'", "fim de caractere");
        buf[2] = c;
        buf[3] = '\0';
        next_char();
        return make_token(sCTECHAR, buf);
    }
    Token t; t.linha = linha;
    switch(c) {
        case '+': t.cat=sSOMA; strcpy(t.lexema,"+"); next_char(); break;
        case '-': t.cat=sSUBRAT; strcpy(t.lexema,"-"); next_char(); break;
        case '*': t.cat=sMULT; strcpy(t.lexema,"*"); next_char(); break;
        case '/': t.cat=sDIV; strcpy(t.lexema,"/"); next_char(); break;
        case '=': t.cat=sIGUAL; strcpy(t.lexema,"="); next_char(); break;
        case '!':
            next_char();
            if (c=='=') { t.cat=sDIFERENTE; strcpy(t.lexema,"!="); next_char(); }
            else diag_error(linha, "!=", "!");
            break;
        case '>':
            next_char();
            if (c=='=') { t.cat=sMAIORIG; strcpy(t.lexema,">="); next_char(); }
            else { t.cat=sMAIOR; strcpy(t.lexema,">"); }
            break;
        case '<':
            next_char();
            if (c=='=') { t.cat=sMENORIG; strcpy(t.lexema,"<="); next_char(); }
            else { t.cat=sMENOR; strcpy(t.lexema,"<"); }
            break;
        case ':':
            next_char();
            if (c=='=') { t.cat=sATRIB; strcpy(t.lexema,":="); next_char(); }
            else { t.cat=sDOISPONTOS; strcpy(t.lexema,":"); }
            break;
        case '^': t.cat=sAND; strcpy(t.lexema,"^"); next_char(); break;
        case 'v': t.cat=sOR; strcpy(t.lexema,"v"); next_char(); break;
        case '~': t.cat=sNEG; strcpy(t.lexema,"~"); next_char(); break;
        case '(': t.cat=sABREPAR; strcpy(t.lexema,"("); next_char(); break;
        case ')': t.cat=sFECHAPAR; strcpy(t.lexema,")"); next_char(); break;
        case '[': t.cat=sABRECOL; strcpy(t.lexema,"["); next_char(); break;
        case ']': t.cat=sFECHACOL; strcpy(t.lexema,"]"); next_char(); break;
        case ',': t.cat=sVIRG; strcpy(t.lexema,","); next_char(); break;
        case ';': t.cat=sPONTOEVIRG; strcpy(t.lexema,";"); next_char(); break;
        default: diag_error(linha, "token valido", (char[]){c,0});
    }
    log_tokens(t);
    return t;
}
