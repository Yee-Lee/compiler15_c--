#ifndef __OFFSET_IN_AR_H___
#define __OFFSET_IN_AR_H___

#include "symbolTable.h"

extern int g_offsetInARAux;
extern int g_deepestBlockVariableOffset;
extern int g_offsetInARParam;

void resetOffsetCalculation();
void setOffsetAndUpdateGlobalOffset(SymbolAttribute* attribute);
void setParamOffset(SymbolAttribute* attribute);

#endif
