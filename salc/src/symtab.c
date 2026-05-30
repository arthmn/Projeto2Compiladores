#include <stdio.h>
#include <string.h>
#include "symtab.h"
#include "log.h"
#include "diag.h"

// Define o limite estático de símbolos que o compilador pode armazenar na memória
#define MAX_SYMBOLS 1024
static Symbol table[MAX_SYMBOLS];
static int count = 0;

// Inicializa a tabela de símbolos
void ts_init() { count = 0; }


// Faz a validação semântica para garantir que não existem 
// duas variáveis com o mesmo nome declaradas no mesmo escopo
// Insere um novo identificador na tabela de símbolos
void ts_insert(const char *id, const char *categoria, const char *tipo_str,
               DataType tipo_enum, const char *escopo, int extra, int offset) {
    
	// Verifica duplicidade no mesmo escopo
	for (int i = 0; i < count; i++) {
        if (strcmp(table[i].id, id) == 0 && strcmp(table[i].escopo, escopo) == 0) {
            diag_error(0, "identificador único no escopo", id);
        }
    }
	
	// Tratamento de erro caso o programa fonte seja grande demais para a memória estática
    if (count >= MAX_SYMBOLS) diag_error(0, "tabela de símbolos cheia", "");
	
	// Armazena as propriedades do símbolo
    strcpy(table[count].id, id);
    strcpy(table[count].categoria, categoria);
    strcpy(table[count].tipo_str, tipo_str);
    table[count].tipo_enum = tipo_enum;
    strcpy(table[count].escopo, escopo);
    table[count].extra = extra;
    table[count].offset = offset;
	
	// Registra a inserção no arquivo de log (.ts)
    log_symtab_entry(escopo, id, categoria, tipo_str, extra);
    count++;
}

// Busca um símbolo na tabela pelo seu nome (id)
Symbol *ts_lookup(const char *id) {
    for (int i = count-1; i >= 0; i--) {
        if (strcmp(table[i].id, id) == 0) return &table[i];
    }
    return NULL;
}

// Retorna o endereço (deslocamento na pilha MEPA) de uma variável
// Utilizado por instruções como CRVL e ARMZ
int ts_get_var_offset(const char *id) {
    Symbol *s = ts_lookup(id);
    return s ? s->offset : -1;
}

// Retorna o tipo de dado de uma variável
// Utilizado pelo parser para verificação de tipos em expressões
DataType ts_get_var_type(const char *id) {
    Symbol *s = ts_lookup(id);
    return s ? s->tipo_enum : TYPE_ERROR;
}

// Função de diagnóstico que imprime o estado final da tabela de símbolos no terminal
void ts_print() {
    printf("\nTabela de Simbolos\n");
    for (int i = 0; i < count; i++) {
        printf("%s %s %s %s %d\n", table[i].id, table[i].categoria,
               table[i].tipo_str, table[i].escopo, table[i].extra);
    }
}

// Libera recursos da tabela 
// Como usamos alocação estática, esta função fica vazia
void ts_free() {}