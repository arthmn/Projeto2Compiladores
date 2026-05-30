#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "log.h"
#include "lex.h"
#include "diag.h"

// Ponteiro de arquivo ativo contendo o código-fonte a ser compilado
static FILE *fonte = NULL;

// Rastreador da linha atual
static int linha = 1;

// Caractere de lookahead
static int c = ' ';

// Consome o caractere atual e avança o ponteiro de leitura do arquivo
// Caso detecte uma quebra de linha ('\n'), atualiza automaticamente o rastreador de linhas
static void next_char() { 
    c = fgetc(fonte); 
    if (c == '\n') linha++; 
}

// Empacota as informações lidas em uma estrutura de Token unificada
// Instancia o token e invoca o registro no log (.tk), centralizando a exportação.
static Token make_token(TokenCat cat, const char *lex) {
    Token t = {cat, "", linha};
    strcpy(t.lexema, lex);
    log_tokens(t);
    return t;
}

// Verifica se uma string lida corresponde a uma palavra reservada da linguagem SAL
static TokenCat keyword(const char *lex) {
    // Retorna a categoria correta para palavras reservadas
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
	if (strcmp(lex, "v")==0) return sOR;	// Operador condicional alternativo
    if (strcmp(lex, "not")==0) return sNEG;
    return sIDENTIF; // Se não for reservada, é identificador
}

// O primeiro next_char() alimenta o buffer 'c'
// para que o analisador tenha o que avaliar assim que lex_next() for chamado
void lex_init(const char *filename) {
    fonte = fopen(filename, "r");
    if (!fonte) { printf("Erro ao abrir %s\n", filename); exit(1); }
    next_char();
}

// Libera os recursos estáticos de leitura após o término da compilação
void lex_close() { if (fonte) fclose(fonte); }

// Varre o código caractere por caractere para formar tokens
Token lex_next() {
	// garante que múltiplos espaços ou blocos de comentários 
    // em sequência sejam descartados antes de processar um token real
    while (1) {
        while (isspace(c)) next_char();
        
		// Tratamento prioritário para evitar que marcações de comentário sejam lidas como divisão
        if (c == '/') {
            next_char();
            if (c == '/') {
                // Comentário de linha: consome a linha inteira (até o \n) e ignora tudo
                while (c != '\n' && c != EOF) next_char();
            } else if (c == '*') {
                // Comentário de bloco: continua consumindo o arquivo indefinidamente até achar '*/'
                next_char();
                int fechou = 0;
                while (c != EOF) {
                    if (c == '*') {
                        next_char();
                        if (c == '/') { next_char(); fechou = 1; break; }
                    } else {
                        next_char();
                    }
                }
                if (!fechou) diag_error(linha, "fim de bloco de comentario", "EOF");
            } else {
                // Não foi seguido por / ou *, logo é apenas o operador de divisão
                return make_token(sDIV, "/");
            }
        } else {
            // Se não é espaço nem comentário, sai do laço para processar o token
            break; 
        }
    }

    if (c == EOF) return make_token(sEOF, "EOF");

    // Identificadores e Palavras Reservadas
    if (isalpha(c) || c == '_') {
        char buf[256]; int i=0;
        while (isalnum(c) || c=='_') { buf[i++]=c; next_char(); }
        buf[i]='\0';
        return make_token(keyword(buf), buf);
    }
    
    // Constantes Inteiras
    if (isdigit(c)) {
        char buf[256]; int i=0;
        while (isdigit(c)) { buf[i++]=c; next_char(); }
        buf[i]='\0';
        return make_token(sCTEINT, buf);
    }
    
    // Strings Literais
    if (c == '"') {
        char buf[256]; int i=0;
        next_char(); // Pula as aspas iniciais
        while (c != '"' && c != EOF) { buf[i++]=c; next_char(); }
        buf[i]='\0';
        next_char(); // Pula as aspas finais
        return make_token(sSTRING, buf);
    }
    
    // Caracteres Literais
    if (c == '\'') {
        char buf[4];
        buf[0] = '\'';
        next_char();
        buf[1] = c;
        next_char();
		// Tratamento de erro semântico-léxico: caracteres literais devem ter tamanho 1
        if (c != '\'') diag_error(linha, "'", "fim de caractere");
        buf[2] = c;
        buf[3] = '\0';
        next_char();
        return make_token(sCTECHAR, buf);
    }
	// Operadores Simples ou Compostos e Pontuação
    // O comando next_char() é acionado para verificar ambiguidade
    Token t; t.linha = linha;
    switch(c) {
        case '+': t.cat=sSOMA; strcpy(t.lexema,"+"); next_char(); break;
        case '-': t.cat=sSUBRAT; strcpy(t.lexema,"-"); next_char(); break;
        case '*': t.cat=sMULT; strcpy(t.lexema,"*"); next_char(); break;
        // O caso da divisão '/' foi movido para o tratamento de comentários acima
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
            else if (c=='>') { t.cat=sDIFERENTE; strcpy(t.lexema,"<>"); next_char(); }
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