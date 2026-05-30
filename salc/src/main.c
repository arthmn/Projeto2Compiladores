#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "lex.h"
#include "parser.h"
#include "symtab.h"
#include "log.h"

// Extrai apenas o nome principal do arquivo passado, ignorando o 
// caminho de pastas e a extensão ".sal".
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

// Ponto de entrada do compilador
// Inicializa a esteira de compilação: Logs -> Léxico -> Tabela de Símbolos -> Sintático
int main(int argc, char **argv) {
	// 1. Processa a linha de comando e valida parâmetros
    opts_parse(argc, argv);
    Options *opts = opts_get();
    
	// 2. Prepara o nome base para os arquivos de saída
	char basename[256];
    extract_basename(opts->fonte, basename);

	// 3. Abre os arquivos de log
    log_tokens_open(basename);
    log_trace_open(basename);
    log_symtab_open(basename);

	// 4. Inicializa os módulos principais
    lex_init(opts->fonte);
    ts_init();
    parser_set_output_name(basename);

	// 5. Inicia a Compilação
    parse_program();

	// 6. Imprime a Tabela de Símbolos no console para validação
    ts_print();
    
	// 7. Libera a memória e fecha todos os arquivos abertos
	ts_free();
    lex_close();
    log_tokens_close();
    log_trace_close();
    log_symtab_close();

    printf("Compilacao finalizada.\n");
    return 0;
}