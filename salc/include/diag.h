#ifndef DIAG_H
#define DIAG_H
void diag_error(int linha, const char *esperado, const char *encontrado);
void diag_info(const char *msg);
#endif
