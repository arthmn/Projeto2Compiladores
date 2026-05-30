#ifndef TOKEN_H
#define TOKEN_H
#define MAX_LEXEMA 256
typedef enum {
    sEOF=0, sIDENTIF, sCTEINT, sCTECHAR, sSTRING,
    sMODULE, sGLOBALS, sLOCALS, sPROC, sFN, sMAIN,
    sINT, sBOOL, sCHAR, sIF, sELSE, sMATCH, sWHEN, sOTHERWISE,
    sFOR, sTO, sSTEP, sDO, sLOOP, sWHILE, sUNTIL,
    sPRINT, sSCAN, sRET, sTRUE, sFALSE, sSTART, sEND,
    sATRIB, sSOMA, sSUBRAT, sMULT, sDIV,
    sMAIOR, sMENOR, sMAIORIG, sMENORIG, sIGUAL, sDIFERENTE,
    sAND, sOR, sNEG, sABREPAR, sFECHAPAR, sABRECOL, sFECHACOL,
    sVIRG, sPONTOEVIRG, sDOISPONTOS
} TokenCat;
typedef struct { TokenCat cat; char lexema[MAX_LEXEMA]; int linha; } Token;
#endif
