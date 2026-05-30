#include <stdio.h>
#include <string.h>
#include "log.h"
#include "opt.h"

// Ponteiros para os três tipos de arquivos de log do compilador
static FILE *tk=NULL, *trc=NULL, *ts=NULL;

// Converte o enumerador interno do Token para uma string legível
static const char* name(TokenCat c){
    switch(c){
        case sEOF: return "sEOF"; case sIDENTIF: return "sIDENTIF";
        case sCTEINT: return "sCTEINT"; case sSTRING: return "sSTRING";
        case sCTECHAR: return "sCTECHAR"; case sMODULE: return "sMODULE"; 
        case sGLOBALS: return "sGLOBALS"; case sLOCALS: return "sLOCALS"; 
        case sPROC: return "sPROC"; case sMAIN: return "sMAIN"; 
        case sINT: return "sINT"; case sBOOL: return "sBOOL"; 
        case sCHAR: return "sCHAR"; case sIF: return "sIF"; 
        case sELSE: return "sELSE"; case sMATCH: return "sMATCH"; 
        case sWHEN: return "sWHEN"; case sOTHERWISE: return "sOTHERWISE"; 
        case sFOR: return "sFOR"; case sTO: return "sTO"; 
        case sSTEP: return "sSTEP"; case sDO: return "sDO"; 
        case sLOOP: return "sLOOP"; case sWHILE: return "sWHILE"; 
        case sUNTIL: return "sUNTIL"; case sPRINT: return "sPRINT"; 
        case sSCAN: return "sSCAN"; case sRET: return "sRET"; 
        case sTRUE: return "sTRUE"; case sFALSE: return "sFALSE"; 
        case sSTART: return "sSTART"; case sEND: return "sEND"; 
        case sAND: return "sAND"; case sOR: return "sOR"; 
        case sNEG: return "sNEG"; case sSOMA: return "sSOMA"; 
        case sSUBRAT: return "sSUBRAT"; case sMULT: return "sMULT"; 
        case sDIV: return "sDIV"; case sIGUAL: return "sIGUAL"; 
        case sDIFERENTE: return "sDIFERENTE"; case sMAIOR: return "sMAIOR"; 
        case sMAIORIG: return "sMAIORIG"; case sMENOR: return "sMENOR"; 
        case sMENORIG: return "sMENORIG"; case sATRIB: return "sATRIB"; 
        case sPONTOEVIRG: return "sPONTOEVIRG"; case sDOISPONTOS: return "sDOISPONTOS"; 
        case sVIRG: return "sVIRG"; case sABREPAR: return "sABREPAR"; 
        case sFECHAPAR: return "sFECHAPAR"; case sABRECOL: return "sABRECOL"; 
        case sFECHACOL: return "sFECHACOL";
        default: return "sUNKNOWN";
    }
}

// Funções de log de tokens (--tokens)
void log_tokens_open(const char *b){ Options *o=opts_get(); if(!o->log_tokens) return; char n[256]; sprintf(n,"%s.tk",b); tk=fopen(n,"w"); }
void log_tokens(Token t){ if(tk) fprintf(tk,"%d %s \"%s\"\n",t.linha,name(t.cat),t.lexema); }
void log_tokens_close(){ if(tk) fclose(tk); }

// Funções de log de trace (--trace)
void log_trace_open(const char *b){ Options *o=opts_get(); if(!o->log_trace) return; char n[256]; sprintf(n,"%s.trc",b); trc=fopen(n,"w"); }
void log_trace(const char *m){ if(trc) fprintf(trc,"%s\n",m); }
void log_trace_close(){ if(trc) fclose(trc); }

// Funções de log da tabela de simbolos (--symtab)
void log_symtab_open(const char *b){ Options *o=opts_get(); if(!o->log_symtab) return; char n[256]; sprintf(n,"%s.ts",b); ts=fopen(n,"w"); }
void log_symtab_entry(const char *e,const char *id,const char *c,const char *t,int extra){ if(ts) fprintf(ts,"SCOPE=%s id=\"%s\" cat=%s tipo=%s extra=%d\n",e,id,c,t,extra); }
void log_symtab_close(){ if(ts) fclose(ts); }
