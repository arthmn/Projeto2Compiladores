#ifndef GERADOR_H
#define GERADOR_H

void gerador_init(const char *filename);
void gerador_finalize();
void gerar_instr(const char *mnemonic, const char *arg1, const char *arg2);
void gerar_rotulo(const char *rotulo);
char* novo_rotulo();
void gerar_cabecalho();
void gerar_fim();

#endif
