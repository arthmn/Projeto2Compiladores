#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opt.h"

static Options opts;

Options *opts_get() { return &opts; }

void opts_parse(int argc, char **argv) {
    // Verifica se o arquivo fonte foi informado
    if(argc < 2){ 
        printf("Uso: salc <arquivo.sal> [--tokens] [--symtab] [--trace]\n"); 
        exit(1); 
    }
    
    strcpy(opts.fonte, argv[1]);
    opts.log_tokens = opts.log_symtab = opts.log_trace = 0;
    
    // Itera sobre os argumentos extras da linha de comando
    for(int i = 2; i < argc; i++) {
        if(strcmp(argv[i], "--tokens") == 0) {
            opts.log_tokens = 1;
        } else if(strcmp(argv[i], "--symtab") == 0) {
            opts.log_symtab = 1;
        } else if(strcmp(argv[i], "--trace") == 0) {
            opts.log_trace = 1;
        } else {
            // Reporta erro caso o parâmetro seja desconhecido
            printf("Erro: Parametro de linha de comando invalido '%s'\n", argv[i]);
            exit(1);
        }
    }
}