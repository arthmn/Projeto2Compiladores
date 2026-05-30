#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "lex.h"
#include "parser.h"
#include "symtab.h"
#include "log.h"

static void extract_basename(const char *path, char *base) {
    const char *last_slash = strrchr(path, '/');
    const char *start = (last_slash ? last_slash + 1 : path);
    char *dot = strrchr((char *)start, '.');
    if (dot) {
        size_t len = dot - start;
        strncpy(base, start, len);
        base[len] = '\0';
    } else {
        strcpy(base, start);
    }
}

int main(int argc, char **argv) {
    opts_parse(argc, argv);
    Options *opts = opts_get();
    char basename[256];
    extract_basename(opts->fonte, basename);

    log_tokens_open(basename);
    log_trace_open(basename);
    log_symtab_open(basename);

    lex_init(opts->fonte);
    ts_init();
    parser_set_output_name(basename);

    parse_program();

    ts_print();
    ts_free();
    lex_close();

    log_tokens_close();
    log_trace_close();
    log_symtab_close();

    printf("Compilacao finalizada.\n");
    return 0;
}