#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opt.h"
static Options opts;
Options *opts_get() { return &opts; }
void opts_parse(int argc, char **argv) {
    if(argc<2){ printf("Uso: salc <arquivo.sal> [--tokens] [--symtab] [--trace]\n"); exit(1); }
    strcpy(opts.fonte, argv[1]);
    opts.log_tokens=opts.log_symtab=opts.log_trace=0;
    for(int i=2;i<argc;i++) {
        if(strcmp(argv[i],"--tokens")==0) opts.log_tokens=1;
        else if(strcmp(argv[i],"--symtab")==0) opts.log_symtab=1;
        else if(strcmp(argv[i],"--trace")==0) opts.log_trace=1;
    }
}
