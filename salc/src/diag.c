#include <stdio.h>
#include <stdlib.h>
#include "diag.h"


// Interrompe imediatamente a execução do compilador com código de erro 1
// linha: A linha no arquivo-fonte onde o erro ocorreu
// esperado: O que a gramática exigia naquele momento
// encontrado: O token/caractere que quebrou a regra
void diag_error(int linha, const char *esperado, const char *encontrado){
    if(linha>0) printf("Erro linha %d: esperado %s, encontrado %s\n",linha,esperado,encontrado);
    else printf("Erro: %s '%s'\n",esperado,encontrado);
    exit(1);
}
void diag_info(const char *msg){ printf("%s\n",msg); }
