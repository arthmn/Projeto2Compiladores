#include <stdio.h>
#include <string.h>
#include "log.h"
#include "opt.h"
static FILE *tk=NULL, *trc=NULL, *ts=NULL;
static const char* name(TokenCat c){
    switch(c){
        case sEOF: return "sEOF"; case sIDENTIF: return "sIDENTIF";
        case sCTEINT: return "sCTEINT"; case sSTRING: return "sSTRING";
        case sMODULE: return "sMODULE"; case sGLOBALS: return "sGLOBALS";
        case sLOCALS: return "sLOCALS"; case sPROC: return "sPROC";
        case sMAIN: return "sMAIN"; case sINT: return "sINT";
        case sPRINT: return "sPRINT"; case sSTART: return "sSTART";
        case sEND: return "sEND"; case sATRIB: return "sATRIB";
        case sPONTOEVIRG: return "sPONTOEVIRG"; case sDOISPONTOS: return "sDOISPONTOS";
        case sVIRG: return "sVIRG"; case sABREPAR: return "sABREPAR";
        case sFECHAPAR: return "sFECHAPAR"; case sIGUAL: return "sIGUAL";
        default: return "sUNKNOWN";
    }
}
void log_tokens_open(const char *b){ Options *o=opts_get(); if(!o->log_tokens) return; char n[256]; sprintf(n,"%s.tk",b); tk=fopen(n,"w"); }
void log_tokens(Token t){ if(tk) fprintf(tk,"%d %s \"%s\"\n",t.linha,name(t.cat),t.lexema); }
void log_tokens_close(){ if(tk) fclose(tk); }
void log_trace_open(const char *b){ Options *o=opts_get(); if(!o->log_trace) return; char n[256]; sprintf(n,"%s.trc",b); trc=fopen(n,"w"); }
void log_trace(const char *m){ if(trc) fprintf(trc,"%s\n",m); }
void log_trace_close(){ if(trc) fclose(trc); }
void log_symtab_open(const char *b){ Options *o=opts_get(); if(!o->log_symtab) return; char n[256]; sprintf(n,"%s.ts",b); ts=fopen(n,"w"); }
void log_symtab_entry(const char *e,const char *id,const char *c,const char *t,int extra){ if(ts) fprintf(ts,"SCOPE=%s id=\"%s\" cat=%s tipo=%s extra=%d\n",e,id,c,t,extra); }
void log_symtab_close(){ if(ts) fclose(ts); }
