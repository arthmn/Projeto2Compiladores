#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "parser.h"
#include "lex.h"
#include "diag.h"
#include "symtab.h"
#include "gerador.h"

static Token token;
static char nome_arquivo_saida[256] = "";

// Contadores para alocação de memória na máquina MEPA (instruções AMEM e DMEM)
static int var_count_global = 0;
static int var_count_local = 0;

// Define o nome do arquivo MEPA que será gerado
void parser_set_output_name(const char *name) {
    strcpy(nome_arquivo_saida, name);
}

// Avança para o próximo token
static void next() { token = lex_next(); }

// Verifica se o token atual é o esperado pela gramática
// Se for, chama next
// Se não for, dispara um erro
static void match(TokenCat cat) {
    if (token.cat == cat) next();
    else diag_error(token.linha, "token esperado", token.lexema);
}

// Protótipos das funções de expressão
static DataType expr();
static DataType exlog();
static DataType exrel();
static DataType exari();
static DataType exarp();
static DataType fator();
static DataType elem();

// Lê o tipo da variável e preenche as estruturas 
// necessárias para salvar na Tabela de Símbolos
static void parse_tipo(char *tipo_str, DataType *tipo_enum) {
    if (token.cat == sINT) { strcpy(tipo_str, "int"); *tipo_enum = TYPE_INT; next(); }
    else if (token.cat == sBOOL) { strcpy(tipo_str, "bool"); *tipo_enum = TYPE_BOOL; next(); }
    else if (token.cat == sCHAR) { strcpy(tipo_str, "char"); *tipo_enum = TYPE_CHAR; next(); }
    else diag_error(token.linha, "tipo (int, bool, char)", token.lexema);
}

// Processa blocos de variáveis (globals ou locals)
// Calcula os deslocamentos (offsets) na pilha da MEPA para cada variável
static void parse_declaracoes(const char *escopo, int *var_count) {
    while (token.cat == sIDENTIF) {
        char ids[100][256];
        int id_count = 0;
        
        // Lê um ou mais identificadores separados por vírgula (ex: a, b, c : int;)
        strcpy(ids[id_count++], token.lexema);
        next();
        while (token.cat == sVIRG) {
            next();
            if (token.cat == sIDENTIF) { strcpy(ids[id_count++], token.lexema); next(); }
            else diag_error(token.linha, "identificador", token.lexema);
        }
        
        match(sDOISPONTOS);
        
        char tipo_str[32];
        DataType tipo_enum;
        parse_tipo(tipo_str, &tipo_enum);
        
        // Tratamento de Vetores (Arrays)
        int tamanho_vetor = 0;
        if (token.cat == sABRECOL) {
            next();
            if (token.cat == sCTEINT) {
                tamanho_vetor = atoi(token.lexema);
                strcat(tipo_str, "[]");
                next();
            } else diag_error(token.linha, "constante inteira", token.lexema);
            match(sFECHACOL);
        }
        
        match(sPONTOEVIRG);
        
        // Insere as variáveis lidas na tabela de símbolos com seus offsets
        for (int i = 0; i < id_count; i++) {
            int offset = *var_count;
            int alocacao = (tamanho_vetor > 0) ? tamanho_vetor : 1;
            ts_insert(ids[i], "var", tipo_str, tipo_enum, escopo, tamanho_vetor, offset);
            *var_count += alocacao;
        }
    }
}

// As funções abaixo avaliam expressões matemáticas e lógicas respeitando a 
// precedência de operadores e emitindo instruções MEPA baseadas em pilha
static DataType elem() {
    DataType t = TYPE_ERROR;
    if (token.cat == sIDENTIF) {
        Symbol *s = ts_lookup(token.lexema);
        if (!s) diag_error(token.linha, "variavel declarada", token.lexema);
        t = s->tipo_enum;
        char arg[16];
        sprintf(arg, "0,%d", s->offset);
        gerar_instr("CRVL", arg, NULL); // Carrega o Valor da variável na pilha MEPA
        next();
    } else if (token.cat == sCTEINT) {
        t = TYPE_INT;
        gerar_instr("CRCT", token.lexema, NULL); // Carrega Constante Inteira
        next();
    } else if (token.cat == sTRUE) {
        t = TYPE_BOOL;
        gerar_instr("CRCT", "1", NULL);
        next();
    } else if (token.cat == sFALSE) {
        t = TYPE_BOOL;
        gerar_instr("CRCT", "0", NULL);
        next();
    } else if (token.cat == sABREPAR) {
        next();
        t = expr();
        match(sFECHAPAR);
    } else {
        diag_error(token.linha, "fator valido", token.lexema);
    }
    return t;
}

static DataType fator() {
    if (token.cat == sNEG) { // Operador Lógico NOT
        next();
        DataType t = fator();
        if (t != TYPE_BOOL) diag_error(token.linha, "expressao booleana para 'not'", "");
        gerar_instr("NEGA", NULL, NULL);
        return TYPE_BOOL;
    } else if (token.cat == sSUBRAT) { // Operador Matemático Inversor (Negativo)
        next();
        DataType t = fator();
        if (t != TYPE_INT) diag_error(token.linha, "expressao inteira para '-'", "");
        gerar_instr("INVR", NULL, NULL);
        return TYPE_INT;
    }
    return elem();
}

static DataType exarp() {
    DataType t1 = fator();
    while (token.cat == sMULT || token.cat == sDIV) {
        TokenCat op = token.cat;
        next();
        DataType t2 = fator();
        if (t1 != TYPE_INT || t2 != TYPE_INT) diag_error(token.linha, "tipos incompativeis (*, /)", "");
        if (op == sMULT) gerar_instr("MULT", NULL, NULL);
        else gerar_instr("DIVI", NULL, NULL);
    }
    return t1;
}

static DataType exari() {
    DataType t1 = exarp();
    while (token.cat == sSOMA || token.cat == sSUBRAT) {
        TokenCat op = token.cat;
        next();
        DataType t2 = exarp();
        if (t1 != TYPE_INT || t2 != TYPE_INT) diag_error(token.linha, "tipos incompativeis (+, -)", "");
        if (op == sSOMA) gerar_instr("SOMA", NULL, NULL);
        else gerar_instr("SUBT", NULL, NULL);
    }
    return t1;
}

static DataType exrel() {
    DataType t1 = exari();
    if (token.cat == sIGUAL || token.cat == sDIFERENTE || token.cat == sMENOR ||
        token.cat == sMAIOR || token.cat == sMENORIG || token.cat == sMAIORIG) {
        TokenCat op = token.cat;
        next();
        DataType t2 = exari();
        if (t1 != t2) diag_error(token.linha, "tipos incompativeis em relacional", "");
        
        if (op == sIGUAL) gerar_instr("CMIG", NULL, NULL);
        else if (op == sDIFERENTE) gerar_instr("CMDG", NULL, NULL);
        else if (op == sMENOR) gerar_instr("CMME", NULL, NULL);
        else if (op == sMAIOR) gerar_instr("CMMA", NULL, NULL);
        else if (op == sMENORIG) gerar_instr("CMEG", NULL, NULL);
        else if (op == sMAIORIG) gerar_instr("CMAG", NULL, NULL);
        return TYPE_BOOL;
    }
    return t1;
}

static DataType exlog() {
    DataType t1 = exrel();
    while (token.cat == sAND) {
        next();
        DataType t2 = exrel();
        if (t1 != TYPE_BOOL || t2 != TYPE_BOOL) diag_error(token.linha, "tipos incompativeis (and)", "");
        gerar_instr("CONJ", NULL, NULL); // Conjunção lógica (AND)
    }
    return t1;
}

static DataType expr() {
    DataType t1 = exlog();
    while (token.cat == sOR) {
        next();
        DataType t2 = exlog();
        if (t1 != TYPE_BOOL || t2 != TYPE_BOOL) diag_error(token.linha, "tipos incompativeis (or)", "");
        gerar_instr("DISJ", NULL, NULL); // Disjunção lógica (OR)
    }
    return t1;
}

// Realiza o parsing recursivo dos comandos do programa.
// Responsável por gerenciar os laços e desvios criando rótulos estáticos.
static void comando() {
    if (token.cat == sSTART) {
        // Bloco de Comandos
        match(sSTART);
        while (token.cat != sEND && token.cat != sEOF) {
            comando();
        }
        match(sEND);
    } else if (token.cat == sPRINT) {
        match(sPRINT);
        match(sABREPAR);
        expr();
        gerar_instr("IMPR", NULL, NULL);
        match(sFECHAPAR);
        match(sPONTOEVIRG);
    } else if (token.cat == sSCAN) {
        match(sSCAN);
        match(sABREPAR);
        if (token.cat == sIDENTIF) {
            Symbol *s = ts_lookup(token.lexema);
            if (!s) diag_error(token.linha, "variavel declarada", token.lexema);
            gerar_instr("LEIT", NULL, NULL);
            char arg[16];
            sprintf(arg, "0,%d", s->offset);
            gerar_instr("ARMZ", arg, NULL); // Armazena a leitura na variável
            next();
        } else {
            diag_error(token.linha, "identificador", token.lexema);
        }
        match(sFECHAPAR);
        match(sPONTOEVIRG);
    } else if (token.cat == sIF) {
        char rot_falso[16], rot_fim[16];
        strcpy(rot_falso, novo_rotulo());
        strcpy(rot_fim, novo_rotulo());

        match(sIF);
        match(sABREPAR);
        DataType t = expr();
        if (t != TYPE_BOOL) diag_error(token.linha, "expressao booleana no if", "");
        match(sFECHAPAR);
        
        gerar_instr("DSVF", rot_falso, NULL); // Desvia se falso para o ELSE
        comando();
        
        if (token.cat == sELSE) {
            gerar_instr("DSVS", rot_fim, NULL); // Desvia incondicionalmente pro fim
            gerar_rotulo(rot_falso);
            match(sELSE);
            comando();
            gerar_rotulo(rot_fim);
        } else {
            gerar_rotulo(rot_falso); // Se não tiver ELSE, o salto falso é o próprio fim
        }
    } else if (token.cat == sLOOP) {
        char rot_loop[16], rot_saida[16];
        strcpy(rot_loop, novo_rotulo());
        strcpy(rot_saida, novo_rotulo());

        match(sLOOP);
        gerar_rotulo(rot_loop);
        
        if (token.cat == sWHILE) {
            match(sWHILE);
            match(sABREPAR);
            DataType t = expr();
            if (t != TYPE_BOOL) diag_error(token.linha, "expressao booleana no while", "");
            match(sFECHAPAR);
            gerar_instr("DSVF", rot_saida, NULL);
            comando();
            gerar_instr("DSVS", rot_loop, NULL);
            gerar_rotulo(rot_saida);
        } else {
            diag_error(token.linha, "while", token.lexema);
        }
    } else if (token.cat == sFOR) {
        char rot_loop[16], rot_saida[16];
        strcpy(rot_loop, novo_rotulo());
        strcpy(rot_saida, novo_rotulo());
        
        match(sFOR);
        if (token.cat == sIDENTIF) {
            Symbol *s = ts_lookup(token.lexema);
            if (!s) diag_error(token.linha, "variavel declarada", token.lexema);
            char arg[16];
            sprintf(arg, "0,%d", s->offset);
            next();
            match(sATRIB);
            expr();
            gerar_instr("ARMZ", arg, NULL); // Inicializa a variável do laço
            
            match(sTO);
            gerar_rotulo(rot_loop);
            gerar_instr("CRVL", arg, NULL); // Compara a variável com o limite
            expr();
            gerar_instr("CMEG", NULL, NULL); // Condição: var <= limite
            gerar_instr("DSVF", rot_saida, NULL);
            
            match(sDO);
            comando(); // Executa o corpo do laço
            
            // Incremento automático do for
            gerar_instr("CRVL", arg, NULL);
            gerar_instr("CRCT", "1", NULL);
            gerar_instr("SOMA", NULL, NULL);
            gerar_instr("ARMZ", arg, NULL);
            gerar_instr("DSVS", rot_loop, NULL);
            gerar_rotulo(rot_saida);
        } else {
            diag_error(token.linha, "identificador", token.lexema);
        }
    } else if (token.cat == sIDENTIF) {
        Symbol *s = ts_lookup(token.lexema);
        if (!s) diag_error(token.linha, "variavel declarada", token.lexema);
        next();
        if (token.cat == sATRIB) {
            match(sATRIB);
            DataType t = expr();
            if (s->tipo_enum != t) diag_error(token.linha, "tipos incompativeis na atribuicao", "");
            char arg[16];
            sprintf(arg, "0,%d", s->offset);
            gerar_instr("ARMZ", arg, NULL); // Salva o topo da pilha na variável
        } else {
            diag_error(token.linha, ":=", token.lexema);
        }
        match(sPONTOEVIRG);
    } else {
        diag_error(token.linha, "comando", token.lexema);
    }
}

// Ponto de entrada do Analisador Sintático
// Cria a estrutura inicial do arquivo .mepa (INPP), aloca variáveis (AMEM),
// processa o programa principal e finaliza a execução desalocando pilha (DMEM)
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
    gerar_instr("INPP", NULL, NULL); // Inicializa Programa Principal

    if (token.cat == sGLOBALS) {
        match(sGLOBALS);
        parse_declaracoes("global", &var_count_global);
        if (var_count_global > 0) {
            char arg[16];
            sprintf(arg, "%d", var_count_global);
            gerar_instr("AMEM", arg, NULL); // Aloca Memória Global
        }
    }

    match(sPROC);
    match(sMAIN);
    match(sABREPAR);
    match(sFECHAPAR);

    if (token.cat == sLOCALS) {
        match(sLOCALS);
        parse_declaracoes("local", &var_count_local);
        if (var_count_local > 0) {
            char arg[16];
            sprintf(arg, "%d", var_count_local);
            gerar_instr("AMEM", arg, NULL); // Aloca Memória Local
        }
    }

    match(sSTART);
    while (token.cat != sEND && token.cat != sEOF) {
        comando();
    }
    match(sEND);

    if (token.cat != sEOF) diag_error(token.linha, "EOF", token.lexema);
    log_trace("exit parse_program");

    // Desaloca a memória reservada e finaliza o programa virtual
    int total_vars = var_count_global + var_count_local;
    if (total_vars > 0) {
        char arg[16];
        sprintf(arg, "%d", total_vars);
        gerar_instr("DMEM", arg, NULL); 
    }
    gerar_instr("PARA", NULL, NULL);
    gerar_instr("FIM", NULL, NULL);

    gerador_finalize();
}