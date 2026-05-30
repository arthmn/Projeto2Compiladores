#ifndef SYMTAB_H
#define SYMTAB_H

typedef enum { TYPE_INT, TYPE_BOOL, TYPE_CHAR, TYPE_ERROR } DataType;

typedef struct Symbol {
    char id[64];
    char categoria[32];   // "var", "param", "proc", "func", "program"
    char tipo_str[32];    // "int", "bool", "char", "int[10]" etc.
    DataType tipo_enum;   // TYPE_INT, TYPE_BOOL, TYPE_CHAR
    char escopo[256];
    int extra;            // para vetor: tamanho; para proc/func: nº parâmetros
    int offset;           // deslocamento na pilha (para variáveis)
    struct Symbol *next;
} Symbol;

void ts_init();
void ts_insert(const char *id, const char *categoria, const char *tipo_str,
               DataType tipo_enum, const char *escopo, int extra, int offset);
Symbol *ts_lookup(const char *id);
void ts_print();
void ts_free();
int ts_get_var_offset(const char *id);
DataType ts_get_var_type(const char *id);

#endif