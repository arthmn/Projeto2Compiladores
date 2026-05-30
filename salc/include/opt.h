#ifndef OPT_H
#define OPT_H
typedef struct { char fonte[256]; int log_tokens; int log_symtab; int log_trace; } Options;
void opts_parse(int argc, char **argv); Options *opts_get();
#endif
