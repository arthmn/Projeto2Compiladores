#include <stdio.h>
#include <stdlib.h>
#include "diag.h"
void diag_error(int linha, const char *esperado, const char *encontrado){
    if(linha>0) printf("Erro linha %d: esperado %s, encontrado %s\n",linha,esperado,encontrado);
    else printf("Erro: %s '%s'\n",esperado,encontrado);
    exit(1);
}
void diag_info(const char *msg){ printf("%s\n",msg); }
