#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gerador.h"
#include "diag.h"

static FILE *mepa_file = NULL;
static int rotulo_count = 0; // Rastreador global para garantir rótulos únicos

// Cria o arquivo final .mepa em modo de escrita
void gerador_init(const char *filename) {
    char mepa_name[256];
    sprintf(mepa_name, "%s.mepa", filename);
    mepa_file = fopen(mepa_name, "w");
    if (!mepa_file) {
        diag_error(0, "criar arquivo .mepa", mepa_name);
    }
}

// Salva as alterações e fecha o arquivo .mepa gerado
void gerador_finalize() {
    if (mepa_file) fclose(mepa_file);
}

// Formata e escreve uma instrução genérica no arquivo MEPA
// Instruções sem argumento, com 1 argumento ou com 2 argumentos
void gerar_instr(const char *mnemonic, const char *arg1, const char *arg2) {
    if (!mepa_file) return;
    if (arg1 && arg2) {
        fprintf(mepa_file, "    %s %s,%s\n", mnemonic, arg1, arg2);
    } else if (arg1) {
        fprintf(mepa_file, "    %s %s\n", mnemonic, arg1);
    } else {
        fprintf(mepa_file, "    %s\n", mnemonic);
    }
}

// Escreve um marcador de desvio de fluxo no arquivo
void gerar_rotulo(const char *rotulo) {
    if (!mepa_file) return;
    fprintf(mepa_file, "%s: NADA\n", rotulo);
}

// Retorna uma string sequencial única usada para criar 
// rótulos de desvio em IFs e laços de repetição.
// Retorna um ponteiro para um buffer interno
char* novo_rotulo() {
    static char buffer[16];
    rotulo_count++;
    sprintf(buffer, "L%d", rotulo_count);
    return buffer;
}

void gerar_cabecalho() {
    gerar_instr("INPP", NULL, NULL);
}

void gerar_fim() {
    gerar_instr("PARA", NULL, NULL);
    gerar_instr("FIM", NULL, NULL);
}
