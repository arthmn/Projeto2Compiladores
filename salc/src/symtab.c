#include <stdio.h>
#include <string.h>
#include "symtab.h"
#include "log.h"
#include "diag.h"

#define MAX_SYMBOLS 1024
static Symbol table[MAX_SYMBOLS];
static int count = 0;

void ts_init() { count = 0; }

void ts_insert(const char *id, const char *categoria, const char *tipo_str,
               DataType tipo_enum, const char *escopo, int extra, int offset) {
    // verificar duplicata no mesmo escopo
    for (int i = 0; i < count; i++) {
        if (strcmp(table[i].id, id) == 0 && strcmp(table[i].escopo, escopo) == 0) {
            diag_error(0, "identificador único no escopo", id);
        }
    }
    if (count >= MAX_SYMBOLS) diag_error(0, "tabela de símbolos cheia", "");
    strcpy(table[count].id, id);
    strcpy(table[count].categoria, categoria);
    strcpy(table[count].tipo_str, tipo_str);
    table[count].tipo_enum = tipo_enum;
    strcpy(table[count].escopo, escopo);
    table[count].extra = extra;
    table[count].offset = offset;
    log_symtab_entry(escopo, id, categoria, tipo_str, extra);
    count++;
}

Symbol *ts_lookup(const char *id) {
    for (int i = count-1; i >= 0; i--) {
        if (strcmp(table[i].id, id) == 0) return &table[i];
    }
    return NULL;
}

int ts_get_var_offset(const char *id) {
    Symbol *s = ts_lookup(id);
    return s ? s->offset : -1;
}

DataType ts_get_var_type(const char *id) {
    Symbol *s = ts_lookup(id);
    return s ? s->tipo_enum : TYPE_ERROR;
}

void ts_print() {
    printf("\nTabela de Símbolos\n");
    for (int i = 0; i < count; i++) {
        printf("%s %s %s %s %d\n", table[i].id, table[i].categoria,
               table[i].tipo_str, table[i].escopo, table[i].extra);
    }
}

void ts_free() {}