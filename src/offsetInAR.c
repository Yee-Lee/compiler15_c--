#include <stdio.h>
#include "offsetInAR.h"
#include "symbolTable.h"


int g_offsetInARAux = 0;
int g_deepestBlockVariableOffset = 0;

int g_offsetInARParam = 0;

void resetOffsetCalculation()
{
    g_offsetInARAux = 0;
    g_deepestBlockVariableOffset = 0;
    g_offsetInARParam = 0;
}


void setOffsetAndUpdateGlobalOffset(SymbolAttribute* attribute)
{
    int variableSize = getVariableSize(attribute->attr.typeDescriptor);
    g_offsetInARAux = g_offsetInARAux - variableSize;
    attribute->offsetInAR = g_offsetInARAux;
}

void setParamOffset(SymbolAttribute* attribute)
{
	int variableSize = getVariableSize(attribute->attr.typeDescriptor);
    attribute->offsetInAR = g_offsetInARParam + 16;	
    g_offsetInARParam += variableSize;
    
}