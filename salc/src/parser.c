#include <stdio.h>
#include <string.h>
#include "log.h"
#include "parser.h"
#include "lex.h"
#include "diag.h"
#include "symtab.h"
#include "gerador.h"

static Token token;
static char nome_arquivo_saida[256] = "";
static int var_count_global = 0;
static int var_count_local = 0;

void parser_set_output_name(const char *name) {
    strcpy(nome_arquivo_saida, name);
}

static void next() { token = lex_next(); }
static void match(TokenCat cat) {
    if (token.cat == cat) next();
    else diag_error(token.linha, "token esperado", token.lexema);
}

// Protótipos das funções de expressão (retornam tipo)
static DataType expr();
static DataType exlog();
static DataType exrel();
static DataType exari();
static DataType exarp();
static DataType fator();
static DataType elem();

// Tipos e declarações
static DataType token_to_type(TokenCat cat) {
    if (cat == sINT) return TYPE_INT;
    if (cat == sBOOL) return TYPE_BOOL;
    if (cat == sCHAR) return TYPE_CHAR;
    return TYPE_ERROR;
}

static void parse_tipo(char *tipo_str, DataType *tipo_enum) {
    if (token.cat == sINT) { strcpy(tipo_str, "int"); *tipo_enum = TYPE_INT; next(); }
    else if (token.cat == sBOOL) { strcpy(tipo_str, "bool"); *tipo_enum = TYPE_BOOL; next(); }
    else if (token.cat == sCHAR) { strcpy(tipo_str, "char"); *tipo_enum = TYPE_CHAR; next(); }
    else diag_error(token.linha, "tipo", token.lexema);
}

static void parse_declaracoes(const char *escopo, int *var_count) {
    while (token.cat == sIDENTIF) {
        char ids[10][64];
        int n = 0;
        do {
            strcpy(ids[n++], token.lexema);
            next();
            if (token.cat == sVIRG) next();
            else break;
        } while (1);
        match(sDOISPONTOS);
        char tipo_str[32];
        DataType tipo_enum;
        parse_tipo(tipo_str, &tipo_enum);
        // verifica se é vetor
        int tamanho_vetor = 0;
        if (token.cat == sABRECOL) {
            next();
            if (token.cat == sCTEINT) {
                tamanho_vetor = atoi(token.lexema);
                sprintf(tipo_str, "%s[%d]", tipo_str, tamanho_vetor);
                next();
            } else diag_error(token.linha, "constante inteira", token.lexema);
            match(sFECHACOL);
        }
        match(sPONTOEVIRG);
        for (int i = 0; i < n; i++) {
            ts_insert(ids[i], "var", tipo_str, tipo_enum, escopo, tamanho_vetor, (*var_count)++);
        }
    }
}

// Expressões com retorno de tipo
static DataType expr() {
    DataType t = exlog();
    while (token.cat == sOR) {
        next();
        DataType t2 = exlog();
        if (t != TYPE_BOOL || t2 != TYPE_BOOL)
            diag_error(token.linha, "operandos booleanos", "tipos incompatíveis");
        gerar_instr("DISJ", NULL, NULL);
        t = TYPE_BOOL;
    }
    return t;
}

static DataType exlog() {
    DataType t = exrel();
    while (token.cat == sAND) {
        next();
        DataType t2 = exrel();
        if (t != TYPE_BOOL || t2 != TYPE_BOOL)
            diag_error(token.linha, "operandos booleanos", "tipos incompatíveis");
        gerar_instr("CONJ", NULL, NULL);
        t = TYPE_BOOL;
    }
    return t;
}

static DataType exrel() {
    DataType t = exari();
    if (token.cat == sMAIOR || token.cat == sMAIORIG ||
        token.cat == sMENOR || token.cat == sMENORIG ||
        token.cat == sIGUAL || token.cat == sDIFERENTE) {
        TokenCat op = token.cat;
        next();
        DataType t2 = exari();
        if (t != TYPE_INT || t2 != TYPE_INT)
            diag_error(token.linha, "operandos inteiros", "tipos incompatíveis");
        switch (op) {
            case sMAIOR: gerar_instr("CMMA", NULL, NULL); break;
            case sMAIORIG: gerar_instr("CMAG", NULL, NULL); break;
            case sMENOR: gerar_instr("CMME", NULL, NULL); break;
            case sMENORIG: gerar_instr("CMEG", NULL, NULL); break;
            case sIGUAL: gerar_instr("CMIG", NULL, NULL); break;
            case sDIFERENTE: gerar_instr("CMDG", NULL, NULL); break;
            default: break;
        }
        t = TYPE_BOOL;
    }
    return t;
}

static DataType exari() {
    DataType t = exarp();
    while (token.cat == sSOMA || token.cat == sSUBRAT) {
        TokenCat op = token.cat;
        next();
        DataType t2 = exarp();
        if (t != TYPE_INT || t2 != TYPE_INT)
            diag_error(token.linha, "operandos inteiros", "tipos incompatíveis");
        if (op == sSOMA) gerar_instr("SOMA", NULL, NULL);
        else gerar_instr("SUBT", NULL, NULL);
        t = TYPE_INT;
    }
    return t;
}

static DataType exarp() {
    DataType t = fator();
    while (token.cat == sMULT || token.cat == sDIV) {
        TokenCat op = token.cat;
        next();
        DataType t2 = fator();
        if (t != TYPE_INT || t2 != TYPE_INT)
            diag_error(token.linha, "operandos inteiros", "tipos incompatíveis");
        if (op == sMULT) gerar_instr("MULT", NULL, NULL);
        else gerar_instr("DIVI", NULL, NULL);
        t = TYPE_INT;
    }
    return t;
}

static DataType fator() {
    if (token.cat == sNEG || token.cat == sSUBRAT) {
        int is_neg = (token.cat == sSUBRAT);
        next();
        DataType t = fator();
        if (t != TYPE_INT)
            diag_error(token.linha, "operando inteiro", "tipo incompatível");
        if (is_neg) gerar_instr("INVR", NULL, NULL);
        else gerar_instr("NEGA", NULL, NULL);
        return TYPE_INT;
    } else if (token.cat == sABREPAR) {
        next();
        DataType t = expr();
        match(sFECHAPAR);
        return t;
    } else {
        return elem();
    }
}

static DataType elem() {
    if (token.cat == sIDENTIF) {
        char nome[64];
        strcpy(nome, token.lexema);
        next();
        // verifica se é chamada de função
        if (token.cat == sABREPAR) {
            // Chamada de função (deve retornar tipo)
            Symbol *s = ts_lookup(nome);
            if (!s || (strcmp(s->categoria, "func") != 0 && strcmp(s->categoria, "proc") != 0))
                diag_error(token.linha, "procedimento/função", nome);
            match(sABREPAR);
            // avaliar argumentos
            if (token.cat != sFECHAPAR) {
                do {
                    DataType arg_tipo = expr();
                    // validação de tipo com parâmetro (simplificada)
                    if (arg_tipo != TYPE_INT) // apenas exemplo
                        diag_error(token.linha, "tipo do argumento", "int esperado");
                    if (token.cat == sVIRG) next();
                    else break;
                } while (1);
            }
            match(sFECHAPAR);
            // gerar chamada (CHPR para procedimento, CHPP para função)
            char rotulo[32];
            sprintf(rotulo, "F_%s", nome); // rótulo da função (precisa ser definido)
            if (strcmp(s->categoria, "func") == 0)
                gerar_instr("CHPP", rotulo, NULL);
            else
                gerar_instr("CHPR", rotulo, NULL);
            return s->tipo_enum;
        }
        // variável ou vetor
        int offset = ts_get_var_offset(nome);
        DataType tipo = ts_get_var_type(nome);
        if (offset < 0) diag_error(token.linha, "variável declarada", nome);
        char arg[16];
        sprintf(arg, "0,%d", offset);
        if (token.cat == sABRECOL) { // vetor: a[expr]
            match(sABRECOL);
            DataType idx_tipo = expr();
            if (idx_tipo != TYPE_INT)
                diag_error(token.linha, "índice inteiro", "tipo incompatível");
            match(sFECHACOL);
            // cálculo do endereço: base + (indice-1)*tamanho_elemento
            gerar_instr("CRVL", arg, NULL);        // endereço base do vetor
            gerar_instr("CRCT", "1", NULL);        // pular o tamanho
            gerar_instr("SOMA", NULL, NULL);       // addr do elemento 0
            // o índice já está na pilha? Na nossa expr, o índice foi empilhado. Precisamos reordenar.
            // Para simplificar, vamos gerar o código de acesso diretamente.
            // O código ideal seria: 
            //   CRVL base
            //   CRCT 1
            //   SOMA
            //   (código do índice)
            //   CRCT 1 (tamanho elemento)
            //   INDX
            //   CRVI / ARVI
            // Mas como a expressão do índice já empilhou seu valor, precisamos trocar a ordem.
            // Uma alternativa: gerar o código de carga do endereço, depois do índice, depois INDX.
            // Vamos reorganizar manualmente aqui (simplificação).
            // Na verdade, a função expr() avaliará o índice e deixará no topo. Precisamos gerar INDX após.
            // O código abaixo assume que o índice é avaliado ANTES de empilharmos o endereço.
            // Para não complicar, deixamos apenas a marcação de acesso.
            // O tratamento completo de vetores exigiria reestruturação. Por ora, geramos erro e não implementamos.
            diag_error(token.linha, "vetores ainda não implementados", "use variável simples");
        }
        // se não for vetor, carrega o valor
        gerar_instr("CRVL", arg, NULL);
        return tipo;
    } else if (token.cat == sCTEINT) {
        char val[16];
        strcpy(val, token.lexema);
        next();
        gerar_instr("CRCT", val, NULL);
        return TYPE_INT;
    } else if (token.cat == sSTRING) {
        next();
        // print de string trata no comando, não em expressão
        return TYPE_ERROR;
    } else if (token.cat == sCTECHAR) {
        char val[16];
        sprintf(val, "%d", token.lexema[1]);
        next();
        gerar_instr("CRCT", val, NULL);
        return TYPE_CHAR;
    } else if (token.cat == sTRUE) {
        next();
        gerar_instr("CRCT", "1", NULL);
        return TYPE_BOOL;
    } else if (token.cat == sFALSE) {
        next();
        gerar_instr("CRCT", "0", NULL);
        return TYPE_BOOL;
    } else {
        diag_error(token.linha, "elemento", token.lexema);
        return TYPE_ERROR;
    }
}

// Comandos
static void comando() {
    if (token.cat == sPRINT) {
        match(sPRINT);
        match(sABREPAR);
        do {
            DataType t = elem();   // empilha valor
            if (t != TYPE_INT && t != TYPE_BOOL && t != TYPE_CHAR)
                diag_error(token.linha, "valor imprimível", "tipo inválido");
            gerar_instr("IMPR", NULL, NULL);
            if (token.cat == sVIRG) next();
            else break;
        } while (1);
        match(sFECHAPAR);
        match(sPONTOEVIRG);
    } else if (token.cat == sSCAN) {
        match(sSCAN);
        match(sABREPAR);
        if (token.cat == sIDENTIF) {
            char nome[64];
            strcpy(nome, token.lexema);
            next();
            // verificar se é vetor (não tratado)
            if (token.cat == sABRECOL)
                diag_error(token.linha, "vetor não suportado em scan", nome);
            int offset = ts_get_var_offset(nome);
            if (offset < 0) diag_error(token.linha, "variável declarada", nome);
            char arg[16];
            sprintf(arg, "0,%d", offset);
            gerar_instr("LEIT", NULL, NULL);
            gerar_instr("ARMZ", arg, NULL);
        } else diag_error(token.linha, "identificador", token.lexema);
        match(sFECHAPAR);
        match(sPONTOEVIRG);
    } else if (token.cat == sIDENTIF) {
        // atribuição ou chamada de procedimento
        char nome[64];
        strcpy(nome, token.lexema);
        next();
        if (token.cat == sABREPAR) {
            // chamada de procedimento (sem retorno)
            Symbol *s = ts_lookup(nome);
            if (!s || strcmp(s->categoria, "proc") != 0)
                diag_error(token.linha, "procedimento", nome);
            match(sABREPAR);
            if (token.cat != sFECHAPAR) {
                do {
                    DataType arg_tipo = expr();
                    // validação simplificada
                    if (arg_tipo != TYPE_INT)
                        diag_error(token.linha, "parâmetro inteiro", "tipo incorreto");
                    if (token.cat == sVIRG) next();
                    else break;
                } while (1);
            }
            match(sFECHAPAR);
            char rotulo[32];
            sprintf(rotulo, "P_%s", nome);
            gerar_instr("CHPR", rotulo, NULL);
            match(sPONTOEVIRG);
        } else {
            // atribuição
            int offset = ts_get_var_offset(nome);
            DataType var_tipo = ts_get_var_type(nome);
            if (offset < 0) diag_error(token.linha, "variável declarada", nome);
            if (token.cat == sABRECOL) {
                // vetor no lado esquerdo: atribuição a[i] := expr
                diag_error(token.linha, "vetor em atribuição ainda não implementado", nome);
            }
            match(sATRIB);
            DataType expr_tipo = expr();
            if (var_tipo != expr_tipo)
                diag_error(token.linha, "tipos incompatíveis na atribuição", nome);
            char arg[16];
            sprintf(arg, "0,%d", offset);
            gerar_instr("ARMZ", arg, NULL);
            match(sPONTOEVIRG);
        }
    } else if (token.cat == sIF) {
        char *rot_falso = novo_rotulo();
        char *rot_fim = novo_rotulo();
        match(sIF);
        match(sABREPAR);
        DataType cond = expr();
        if (cond != TYPE_BOOL)
            diag_error(token.linha, "condição booleana", "tipo inválido");
        match(sFECHAPAR);
        gerar_instr("DSVF", rot_falso, NULL);
        comando();
        gerar_instr("DSVS", rot_fim, NULL);
        gerar_rotulo(rot_falso);
        if (token.cat == sELSE) {
            next();
            comando();
        }
        gerar_rotulo(rot_fim);
    } else if (token.cat == sLOOP) {
        char *rot_loop = novo_rotulo();
        char *rot_saida = novo_rotulo();
        next();
        if (token.cat == sWHILE) {
            match(sWHILE);
            match(sABREPAR);
            gerar_rotulo(rot_loop);
            DataType cond = expr();
            if (cond != TYPE_BOOL)
                diag_error(token.linha, "condição booleana", "tipo inválido");
            match(sFECHAPAR);
            gerar_instr("DSVF", rot_saida, NULL);
            comando();
            gerar_instr("DSVS", rot_loop, NULL);
            gerar_rotulo(rot_saida);
        } else {
            // loop ... until
            gerar_rotulo(rot_loop);
            comando();
            match(sUNTIL);
            match(sABREPAR);
            DataType cond = expr();
            if (cond != TYPE_BOOL)
                diag_error(token.linha, "condição booleana", "tipo inválido");
            match(sFECHAPAR);
            match(sPONTOEVIRG);
            gerar_instr("DSVF", rot_loop, NULL);
        }
    } else if (token.cat == sFOR) {
        match(sFOR);
        // variável de controle
        if (token.cat != sIDENTIF) diag_error(token.linha, "identificador", token.lexema);
        char var[64];
        strcpy(var, token.lexema);
        int offset = ts_get_var_offset(var);
        if (offset < 0) diag_error(token.linha, "variável declarada", var);
        char arg[16];
        sprintf(arg, "0,%d", offset);
        next();
        match(sATRIB);
        DataType t_ini = expr();        // valor inicial
        if (t_ini != TYPE_INT) diag_error(token.linha, "expressão inteira", "tipo incompatível");
        gerar_instr("ARMZ", arg, NULL);
        match(sTO);
        char *rot_loop = novo_rotulo();
        char *rot_saida = novo_rotulo();
        gerar_rotulo(rot_loop);
        // carrega variável
        gerar_instr("CRVL", arg, NULL);
        DataType t_lim = expr();        // limite
        if (t_lim != TYPE_INT) diag_error(token.linha, "expressão inteira", "tipo incompatível");
        gerar_instr("CMME", NULL, NULL); // <=
        gerar_instr("DSVF", rot_saida, NULL);
        if (token.cat == sSTEP) {
            match(sSTEP);
            // expr step (pode ser constante ou variável)
            DataType t_step = expr();
            if (t_step != TYPE_INT) diag_error(token.linha, "passo inteiro", "tipo incompatível");
        } else {
            gerar_instr("CRCT", "1", NULL); // passo padrão 1
        }
        match(sDO);
        comando();   // corpo
        // incremento: var := var + passo
        gerar_instr("CRVL", arg, NULL);
        if (token.cat == sSTEP) {
            // já temos o passo empilhado? na verdade consumimos antes do do, teríamos que reavaliar.
            // Simplificação: assumimos que o passo está no topo (precisa ser guardado)
            diag_error(token.linha, "step não implementado totalmente", "use passo constante");
        } else {
            gerar_instr("CRCT", "1", NULL);
        }
        gerar_instr("SOMA", NULL, NULL);
        gerar_instr("ARMZ", arg, NULL);
        gerar_instr("DSVS", rot_loop, NULL);
        gerar_rotulo(rot_saida);
    } else if (token.cat == sRET) {
        match(sRET);
        DataType t = expr();
        if (t != TYPE_INT && t != TYPE_BOOL && t != TYPE_CHAR)
            diag_error(token.linha, "valor de retorno", "tipo inválido");
        gerar_instr("RTPR", NULL, NULL);
        match(sPONTOEVIRG);
    } else if (token.cat == sSTART) {
        match(sSTART);
        while (token.cat != sEND && token.cat != sEOF) {
            comando();
        }
        match(sEND);
    } else {
        diag_error(token.linha, "comando", token.lexema);
    }
}

// Programa principal
void parse_program() {
    log_trace("enter parse_program");
    next();
    match(sMODULE);
    if (token.cat == sIDENTIF) {
        ts_insert(token.lexema, "program", "", TYPE_ERROR, "global", 0, 0);
        next();
    } else diag_error(token.linha, "identificador", token.lexema);
    match(sPONTOEVIRG);

    gerador_init(nome_arquivo_saida[0] ? nome_arquivo_saida : "a");
    gerar_cabecalho();

    if (token.cat == sGLOBALS) {
        match(sGLOBALS);
        parse_declaracoes("global", &var_count_global);
        char arg[16];
        sprintf(arg, "%d", var_count_global);
        gerar_instr("AMEM", arg, NULL);
    }

    // sub-rotinas: apenas main (simplificado)
    match(sPROC);
    match(sMAIN);
    match(sABREPAR);
    match(sFECHAPAR);

    if (token.cat == sLOCALS) {
        match(sLOCALS);
        parse_declaracoes("local", &var_count_local);
        char arg[16];
        sprintf(arg, "%d", var_count_local);
        gerar_instr("AMEM", arg, NULL);
    }

    match(sSTART);
    while (token.cat != sEND && token.cat != sEOF) {
        comando();
    }
    match(sEND);

    if (token.cat != sEOF) diag_error(token.linha, "EOF", token.lexema);
    log_trace("exit parse_program");

    if (var_count_local > 0) {
        char arg[16];
        sprintf(arg, "%d", var_count_local);
        gerar_instr("DMEM", arg, NULL);
    }
    if (var_count_global > 0) {
        char arg[16];
        sprintf(arg, "%d", var_count_global);
        gerar_instr("DMEM", arg, NULL);
    }
    gerar_fim();
    gerador_finalize();

    printf("Programa valido. Codigo MEPA gerado em %s.mepa\n",
           nome_arquivo_saida[0] ? nome_arquivo_saida : "a");
}