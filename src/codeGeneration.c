#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
#include "util.h"

void CG_processProgramNode(AST_NODE *programNode);
void CG_processDeclarationNode(AST_NODE* declarationNode, int* frameSize);
void CG_declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize, int* frameSize);
void CG_declareFunction(AST_NODE* declarationNode);
void CG_processTypeNode(AST_NODE* idNodeAsType);
void CG_processStmtNode(AST_NODE* stmtNode, int* frameSize);
void CG_processGeneralNode(AST_NODE *node, int* frameSize);
void CG_checkAssignmentStmt(AST_NODE* assignmentNode);
void CG_processExprNode(AST_NODE* exprNode);
void CG_processConstValueNode(AST_NODE* constValueNode);
void CG_processVariableRValue(AST_NODE* idNode);
void CG_checkFunctionCall(AST_NODE* functionCallNode, int asExpr);
void CG_processVariableLValue(AST_NODE* idNode, int* isGlobal, int* addrOrReg);
void CG_processExprRelatedNode(AST_NODE* exprRelatedNode);
void CG_checkWriteFunction(AST_NODE* functionCallNode);
void CG_checkReturnStmt(AST_NODE* returnNode);
void CG_checkWhileStmt(AST_NODE* whileNode, int* frameSize);
void CG_processBlockNode(AST_NODE* blockNode, int* frameSize);
void CG_checkIfStmt(AST_NODE* ifNode, int* frameSize);

FILE* fp;
Poll xpoll;
Poll vpoll;
int constCount = 0;
int ctrlCount = 0;

void codeGeneration(AST_NODE *root)
{
	fp = fopen("output.s", "w");
	initPoll(&xpoll);
	initPoll(&vpoll);
    CG_processProgramNode(root);
	fclose(fp);
    return;
}

void CG_processProgramNode(AST_NODE *programNode)
{
    AST_NODE *traverseDeclaration = programNode->child;
    while(traverseDeclaration) {
        if(traverseDeclaration->nodeType == VARIABLE_DECL_LIST_NODE) {
			int frameSize = 0;
            CG_processGeneralNode(traverseDeclaration, &frameSize);
        }
        else {
            //function declaration
			int frameSize = 0;
            CG_processDeclarationNode(traverseDeclaration, &frameSize);
        }
		traverseDeclaration = traverseDeclaration->rightSibling;
    }
    return;
}

void CG_processGeneralNode(AST_NODE *node, int* frameSize)
{
    AST_NODE *traverseChildren = node->child;
    switch(node->nodeType) {
		case VARIABLE_DECL_LIST_NODE:
			while(traverseChildren) {
				CG_processDeclarationNode(traverseChildren, frameSize);
				traverseChildren = traverseChildren->rightSibling;
			}
			break;
		case STMT_LIST_NODE:
			while(traverseChildren) {
				CG_processStmtNode(traverseChildren, frameSize);
				traverseChildren = traverseChildren->rightSibling;
			}
			break;
		case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
			while(traverseChildren) {
				traverseChildren = traverseChildren->rightSibling;
			}
			break;
		case NONEMPTY_RELOP_EXPR_LIST_NODE:
			while(traverseChildren) {
				CG_processExprRelatedNode(traverseChildren);
				traverseChildren = traverseChildren->rightSibling;
			}
			break;
		case NUL_NODE:
			break;
    }
}

void CG_processDeclarationNode(AST_NODE* declarationNode, int* frameSize)
{
    AST_NODE *typeNode = declarationNode->child;
    CG_processTypeNode(typeNode);
    
    switch(declarationNode->semantic_value.declSemanticValue.kind) {
		case VARIABLE_DECL:
			CG_declareIdList(declarationNode, VARIABLE_ATTRIBUTE, 0, frameSize);
			break;
		case TYPE_DECL:
			CG_declareIdList(declarationNode, TYPE_ATTRIBUTE, 0, NULL);
			break;
		case FUNCTION_DECL:
			CG_declareFunction(declarationNode);
			break;
		case FUNCTION_PARAMETER_DECL:
			CG_declareIdList(declarationNode, VARIABLE_ATTRIBUTE, 1, NULL); // TODO
			break;
    }
    return;
}

void CG_processTypeNode(AST_NODE* idNodeAsType)
{
    SymbolTableEntry* symbolTableEntry = retrieveSymbol(idNodeAsType->semantic_value.identifierSemanticValue.identifierName);
    idNodeAsType->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;
    switch(symbolTableEntry->attribute->attr.typeDescriptor->kind) {
        case SCALAR_TYPE_DESCRIPTOR:
            idNodeAsType->dataType = symbolTableEntry->attribute->attr.typeDescriptor->properties.dataType;
            break;
        case ARRAY_TYPE_DESCRIPTOR:
            idNodeAsType->dataType = symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
            break;
    }
}

void CG_declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize, int* frameSize)
{
    AST_NODE* typeNode = declarationNode->child;
    TypeDescriptor *typeDescriptorOfTypeNode = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
    AST_NODE* traverseIDList = typeNode->rightSibling;
    while(traverseIDList) {
        SymbolAttribute* attribute = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
        attribute->attributeKind = isVariableOrTypeAttribute;
        switch(traverseIDList->semantic_value.identifierSemanticValue.kind) {
            case NORMAL_ID:
                attribute->attr.typeDescriptor = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
                break;
            case ARRAY_ID:
                attribute->attr.typeDescriptor = (TypeDescriptor*)malloc(sizeof(TypeDescriptor));
                processDeclDimList(traverseIDList, attribute->attr.typeDescriptor, ignoreArrayFirstDimSize);
                if(typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR) {
                    attribute->attr.typeDescriptor->properties.arrayProperties.elementType = 
                        typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.dataType;
                }
                else {
                    int typeArrayDimension = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
                    int idArrayDimension = attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
                        
					attribute->attr.typeDescriptor->properties.arrayProperties.elementType = 
                        typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
                    attribute->attr.typeDescriptor->properties.arrayProperties.dimension = 
                        typeArrayDimension + idArrayDimension;
                    int indexType = 0;
                    int indexId = 0;
                    for(indexType = 0, indexId = idArrayDimension; indexId < idArrayDimension + typeArrayDimension; ++indexType, ++indexId) {
                        attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[indexId] = 
                            typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[indexType];
                    }
                }
                break;
            case WITH_INIT_ID:
                attribute->attr.typeDescriptor = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
                break;
		}
        traverseIDList->semantic_value.identifierSemanticValue.symbolTableEntry =
            enterSymbol(traverseIDList->semantic_value.identifierSemanticValue.identifierName, attribute);
		enterAddress(traverseIDList->semantic_value.identifierSemanticValue.symbolTableEntry, frameSize);
		if(traverseIDList->semantic_value.identifierSemanticValue.kind == NORMAL_ID && 
			traverseIDList->semantic_value.identifierSemanticValue.symbolTableEntry->nestingLevel == 0 &&
			attribute->attributeKind == VARIABLE_ATTRIBUTE) {
			if(attribute->attr.typeDescriptor->properties.dataType == INT_TYPE) {
				fprintf(fp, ".data\n");
				char* identifierName = traverseIDList->semantic_value.identifierSemanticValue.identifierName;
				fprintf(fp, "_g_%s: .word %d\n", identifierName, 0);
			}
			else {
				fprintf(fp, ".data\n");
				char* identifierName = traverseIDList->semantic_value.identifierSemanticValue.identifierName;
				float fval = 0.0;
				fprintf(fp, "_g_%s: .word 0x%x\n", identifierName, *(int*)&fval);
			}
		}
		if(traverseIDList->semantic_value.identifierSemanticValue.kind == WITH_INIT_ID) {// generate init
			// TODO:only local variable init is support, global variable init is to be done
			// TODO:we are not support type coercion
			AST_NODE* constNode = traverseIDList->child;
			
			if(attribute->attr.typeDescriptor->properties.dataType == INT_TYPE) {
				if(traverseIDList->semantic_value.identifierSemanticValue.symbolTableEntry->nestingLevel == 0) { // global variable
					fprintf(fp, ".data\n");
					char* identifierName = traverseIDList->semantic_value.identifierSemanticValue.identifierName;
					int intval = constNode->semantic_value.const1->const_u.intval;
					fprintf(fp, "_g_%s: .word %d\n", identifierName, intval);
				}
				else { // local variable
					fprintf(fp, ".data\n");
					fprintf(fp, "_CONSTANT_%d: .word %d\n", constCount, constNode->semantic_value.const1->const_u.intval);
					fprintf(fp, ".text\n");
					int xReg = popPoll(&xpoll);
					fprintf(fp, "\tldr w%d, _CONSTANT_%d\n", xReg, constCount);
					fprintf(fp, "\tstr w%d, [x29, #-%d]\n", xReg, traverseIDList->semantic_value.identifierSemanticValue.symbolTableEntry->addr);
					pushPoll(&xpoll, xReg);
					constCount++;
				}
			}
			else {
				if(traverseIDList->semantic_value.identifierSemanticValue.symbolTableEntry->nestingLevel == 0) { // global variable
					fprintf(fp, ".data\n");
					char* identifierName = traverseIDList->semantic_value.identifierSemanticValue.identifierName;
					float fval = constNode->semantic_value.const1->const_u.fval;
					fprintf(fp, "_g_%s: .word 0x%x\n", identifierName, *(int*)&fval);
				}
				else { // local variable
					fprintf(fp, ".data\n");
					float fval = constNode->semantic_value.const1->const_u.fval;
					fprintf(fp, "_CONSTANT_%d: .word 0x%x\n", constCount, *(int*)&fval);
					fprintf(fp, ".text\n");
					int vReg = popPoll(&vpoll);
					fprintf(fp, "\tldr s%d, _CONSTANT_%d\n", vReg, constCount);
					fprintf(fp, "\tstr s%d, [x29, #-%d]\n", vReg, traverseIDList->semantic_value.identifierSemanticValue.symbolTableEntry->addr);
					pushPoll(&vpoll, vReg);
					constCount++;
				}
			}
		}
        traverseIDList = traverseIDList->rightSibling;
    }
}

void CG_declareFunction(AST_NODE* declarationNode)
{
    AST_NODE* returnTypeNode = declarationNode->child;
    AST_NODE* functionNameID = returnTypeNode->rightSibling;
    
    SymbolAttribute* attribute = NULL;
    attribute = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
    attribute->attributeKind = FUNCTION_SIGNATURE;
    attribute->attr.functionSignature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    attribute->attr.functionSignature->returnType = returnTypeNode->dataType;
    attribute->attr.functionSignature->parameterList = NULL;

	if(strcmp(functionNameID->semantic_value.identifierSemanticValue.identifierName, "main") == 0)
		functionNameID->semantic_value.identifierSemanticValue.identifierName = "MAIN";
    enterSymbol(functionNameID->semantic_value.identifierSemanticValue.identifierName, attribute);

    openScope();

    AST_NODE *parameterListNode = functionNameID->rightSibling;
    AST_NODE *traverseParameter = parameterListNode->child;
    int parametersCount = 0;
    if(traverseParameter) {
        ++parametersCount;
        processDeclarationNode(traverseParameter);
        AST_NODE *parameterID = traverseParameter->child->rightSibling;
        
        Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
        parameter->next = NULL;
        parameter->parameterName = parameterID->semantic_value.identifierSemanticValue.identifierName;
        parameter->type = parameterID->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        attribute->attr.functionSignature->parameterList = parameter;

        traverseParameter = traverseParameter->rightSibling;
    }

    Parameter *parameterListTail = attribute->attr.functionSignature->parameterList;
    
	int useless = 0;
    while(traverseParameter) {
        ++parametersCount;
        CG_processDeclarationNode(traverseParameter, NULL); // TODO
        AST_NODE *parameterID = traverseParameter->child->rightSibling;
        
		Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
        parameter->next = NULL;
        parameter->parameterName = parameterID->semantic_value.identifierSemanticValue.identifierName;
        parameter->type = parameterID->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        parameterListTail->next = parameter;
        parameterListTail = parameter;
        traverseParameter = traverseParameter->rightSibling;
    }
    attribute->attr.functionSignature->parametersCount = parametersCount;

    AST_NODE *blockNode = parameterListNode->rightSibling;
    AST_NODE *traverseListNode = blockNode->child;

	/* code generation : _start_func */
	fprintf(fp, ".text\n");
	fprintf(fp, "_start_%s:\n", functionNameID->semantic_value.identifierSemanticValue.identifierName);
	fprintf(fp, "\tstr x30, [sp, #0]\n"); // save ra
	fprintf(fp, "\tstr x29, [sp, #-8]\n"); // save old fp
	fprintf(fp, "\tadd x29, sp, #-8\n");
	fprintf(fp, "\tadd sp, sp, #-16\n");
	fprintf(fp, "\tldr x30, =_frameSize_%s\n", functionNameID->semantic_value.identifierSemanticValue.identifierName);
	fprintf(fp, "\tldr x30, [x30, #0]\n");
	fprintf(fp, "\tsub sp, sp, w30\n");
	
	
	int frameSize = 0;
	// save x9-x15
	int offset = 0;
	int i;

	for(i = 9; i <= 15; i++) {
		offset += 8;
		fprintf(fp, "\tstr x%d, [sp, #%d]\n", i, offset);
		pushPoll(&xpoll, i);
	}

		
		// save s16-s23
	for(i = 16; i <=23; i++) {
		offset += 4;
		fprintf(fp, "\tstr s%d, [sp, #%d]\n", i, offset);
		pushPoll(&vpoll, i);
	}

    while(traverseListNode) {
        CG_processGeneralNode(traverseListNode, &frameSize);
        traverseListNode = traverseListNode->rightSibling;
    }

	frameSize += offset;
	/* code generation : __end_func */
	fprintf(fp, ".text\n");
	fprintf(fp, "_end_%s:\n", functionNameID->semantic_value.identifierSemanticValue.identifierName);
		// resume x9-x15
	offset = 0;
	for(i = 9; i <= 15; i++) {
		offset += 8;
		fprintf(fp, "\tldr x%d, [sp, #%d]\n", i, offset);
	}
	
	
		// resume s16-s23
	for(i = 16; i <= 23; i++) {	
		offset += 4;
		fprintf(fp, "\tldr s%d, [sp, #%d]\n", i, offset);
	}
	
	fprintf(fp, "\tldr x30, [x29, #8]\n");
	fprintf(fp, "\tmov sp, x29\n");
	fprintf(fp, "\tadd sp, sp, #8\n");
	fprintf(fp, "\tldr x29, [x29, #0]\n");
	fprintf(fp, "\tRET x30\n");
	
	fprintf(fp, ".data\n");
	fprintf(fp, "\t_frameSize_%s: .word %d\n", functionNameID->semantic_value.identifierSemanticValue.identifierName, frameSize);
	//dumpSymbolTable();
    closeScope();

}

void CG_processStmtNode(AST_NODE* stmtNode, int* frameSize)
{
	if(stmtNode->nodeType == NUL_NODE) {
		// do nothing
	}	
    else if(stmtNode->nodeType == BLOCK_NODE) {
        CG_processBlockNode(stmtNode, frameSize);
	}
    else {
        switch(stmtNode->semantic_value.stmtSemanticValue.kind) {
			case WHILE_STMT:
				CG_checkWhileStmt(stmtNode, frameSize);
				break;
			case FOR_STMT:
				//checkForStmt(stmtNode);
				break;
			case ASSIGN_STMT:
				CG_checkAssignmentStmt(stmtNode);
				break;
			case IF_STMT:
				CG_checkIfStmt(stmtNode, frameSize);
				break;
			case FUNCTION_CALL_STMT:
				CG_checkFunctionCall(stmtNode, 0);
				break;
			case RETURN_STMT:
				CG_checkReturnStmt(stmtNode);
				break;
        }
    }
}


void CG_checkAssignmentStmt(AST_NODE* assignmentNode) {
	AST_NODE* leftOp = assignmentNode->child;
    AST_NODE* rightOp = leftOp->rightSibling;
	int addrOrReg;
	int isGlobal;
    CG_processVariableLValue(leftOp, &isGlobal, &addrOrReg);
    CG_processExprRelatedNode(rightOp);
	if(assignmentNode->dataType == INT_TYPE) {
		if(isGlobal) {
			fprintf(fp, "\tstr w%d, [x%d, #0]\n", rightOp->reg, addrOrReg);
			pushPoll(&xpoll, rightOp->reg);
			pushPoll(&xpoll, addrOrReg);
		}
		else {
			fprintf(fp, "\tstr w%d, [x29, #-%d]\n", rightOp->reg, addrOrReg);
			pushPoll(&xpoll, rightOp->reg);
		}
	}
	else {
		if(isGlobal) {
			fprintf(fp, "\tstr s%d, [x%d, #0]\n", rightOp->reg, addrOrReg);
			pushPoll(&vpoll, rightOp->reg);
			pushPoll(&xpoll, addrOrReg);
		}
		else {
			fprintf(fp, "\tstr s%d, [x29, #-%d]\n", rightOp->reg, addrOrReg);
			pushPoll(&vpoll, rightOp->reg);
		}
	}

	// we don't handle implicit type conversion in hw5
    //assignmentNode->dataType = getBiggerType(leftOp->dataType, rightOp->dataType); 

}

void CG_processExprRelatedNode(AST_NODE* exprRelatedNode)
{
    switch(exprRelatedNode->nodeType) {
		case EXPR_NODE:
			CG_processExprNode(exprRelatedNode);
			break;
		case STMT_NODE:
			//function call
			CG_checkFunctionCall(exprRelatedNode, 1);
			break;
		case IDENTIFIER_NODE:
			CG_processVariableRValue(exprRelatedNode);
			break;
		case CONST_VALUE_NODE:
			CG_processConstValueNode(exprRelatedNode);
			break;
    }
}

void CG_processExprNode(AST_NODE* exprNode)
{
	if(exprNode->semantic_value.exprSemanticValue.isConstEval) { // view as leaf node
		if(exprNode->dataType == INT_TYPE) {
			fprintf(fp, ".data\n");
			fprintf(fp, "_CONSTANT_%d: .word %d\n", constCount, exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
			fprintf(fp, ".text\n");
			int src = popPoll(&xpoll);
			fprintf(fp, "\tldr w%d, _CONSTANT_%d\n", src, constCount);
			exprNode->reg = src;
			constCount++;
		}
		else if(exprNode->dataType == FLOAT_TYPE) {
			fprintf(fp, ".data\n");
			float fval = exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue;
			fprintf(fp, "_CONSTANT_%d: .word 0x%x\n", constCount, *(int*)&fval);
			fprintf(fp, ".text\n");
			int src = popPoll(&vpoll);
			fprintf(fp, "\tldr s%d, _CONSTANT_%d\n", src, constCount);
			exprNode->reg = src;
			constCount++;
		} 
		else {
			printf("error in line %d\n", __LINE__);
			exit(1);
		}
	}
    else if(exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION) {
        AST_NODE* leftOp = exprNode->child;
        AST_NODE* rightOp = leftOp->rightSibling;
        CG_processExprRelatedNode(leftOp);
        CG_processExprRelatedNode(rightOp);
		// we don't support type coercion in hw5
		if(exprNode->dataType == INT_TYPE) {
			int dst = popPoll(&xpoll);
			switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
				case BINARY_OP_ADD:
					fprintf(fp, "\tadd w%d, w%d, w%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_SUB:
					fprintf(fp, "\tsub w%d, w%d, w%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_MUL:
					fprintf(fp, "\tmul w%d, w%d, w%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_DIV:
					fprintf(fp, "\tsdiv w%d, w%d, w%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_EQ:
					fprintf(fp, "\tcmp w%d, w%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, eq\n", dst);
					break;
				case BINARY_OP_GE:
					fprintf(fp, "\tcmp w%d, w%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, ge\n", dst);
					break;
				case BINARY_OP_LE:
					fprintf(fp, "\tcmp w%d, w%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, le\n", dst);
					break;
				case BINARY_OP_NE:
					fprintf(fp, "\tcmp w%d, w%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, ne\n", dst);
					break;
				case BINARY_OP_GT:
					fprintf(fp, "\tcmp w%d, w%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, gt\n", dst);
					break;
				case BINARY_OP_LT:
					fprintf(fp, "\tcmp w%d, w%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, lt\n", dst);
					break;
				case BINARY_OP_AND:
					fprintf(fp, "\tand w%d, w%d, w%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_OR:	
					fprintf(fp, "\torr w%d, w%d, w%d\n", dst, leftOp->reg, rightOp->reg);
					break;
            }
			exprNode->reg = dst;
			pushPoll(&xpoll, leftOp->reg);
			pushPoll(&xpoll, rightOp->reg);
		} 
		else if(exprNode->dataType == FLOAT_TYPE) {
			int tmp = popPoll(&xpoll);
			int tmp2 = popPoll(&xpoll);
			int dst = popPoll(&vpoll);
			switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
				case BINARY_OP_ADD:
					fprintf(fp, "\tfadd s%d, s%d, s%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_SUB:
					fprintf(fp, "\tfsub s%d, s%d, s%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_MUL:
					fprintf(fp, "\tfmul s%d, s%d, s%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_DIV:
					fprintf(fp, "\tfdiv s%d, s%d, s%d\n", dst, leftOp->reg, rightOp->reg);
					break;
				case BINARY_OP_EQ:
					fprintf(fp, "\tfcmp s%d, s%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, eq\n", tmp);
					fprintf(fp, "\tscvtf s%d, w%d\n", dst, tmp);
					break;
				case BINARY_OP_GE:
					fprintf(fp, "\tfcmp s%d, s%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, ge\n", tmp);
					fprintf(fp, "\tscvtf s%d, w%d\n", dst, tmp);
					break;
				case BINARY_OP_LE:
					fprintf(fp, "\tfcmp s%d, s%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, le\n", tmp);
					fprintf(fp, "\tscvtf s%d, w%d\n", dst, tmp);
					break;
				case BINARY_OP_NE:
					fprintf(fp, "\tfcmp s%d, s%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, ne\n", tmp);
					fprintf(fp, "\tscvtf s%d, w%d\n", dst, tmp);
					break;
				case BINARY_OP_GT:
					fprintf(fp, "\tfcmp s%d, s%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, gt\n", tmp);
					fprintf(fp, "\tscvtf s%d, w%d\n", dst, tmp);
					break;
				case BINARY_OP_LT:
					fprintf(fp, "\tfcmp s%d, s%d\n", leftOp->reg, rightOp->reg);
					fprintf(fp, "\tcset w%d, lt\n", tmp);
					fprintf(fp, "\tscvtf s%d, w%d\n", dst, tmp);
					break;
				case BINARY_OP_AND:
					fprintf(fp, "\tfmov w%d, s%d\n", tmp, leftOp->reg);
					fprintf(fp, "\tfmov w%d, s%d\n", tmp2, rightOp->reg);
					fprintf(fp, "\tand w%d, w%d, w%d\n", tmp, tmp, tmp2);
					fprintf(fp, "\tfmov s%d, w%d\n", dst, tmp);
					break;
				case BINARY_OP_OR:	
					fprintf(fp, "\tfmov w%d, s%d\n", tmp, leftOp->reg);
					fprintf(fp, "\tfmov w%d, s%d\n", tmp2, rightOp->reg);
					fprintf(fp, "\torr w%d, w%d, w%d\n", tmp, tmp, tmp2);
					fprintf(fp, "\tfmov s%d, w%d\n", dst, tmp);
					break;
            }
			exprNode->reg = dst;
			pushPoll(&vpoll, leftOp->reg);
			pushPoll(&vpoll, rightOp->reg);
			pushPoll(&xpoll, tmp);
			pushPoll(&xpoll, tmp2);
		}
		else {
			printf("error in line %d\n", __LINE__);
			exit(1);
		}
    }
    else {
        AST_NODE* operand = exprNode->child;
        CG_processExprRelatedNode(operand);
		if(exprNode->dataType == INT_TYPE) {
			int dst = popPoll(&xpoll);
			switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp) {
				case UNARY_OP_POSITIVE:
					fprintf(fp, "\tmov w%d, w%d\n", dst, operand->reg);
					break;
				case UNARY_OP_NEGATIVE:
					fprintf(fp, "\tsub w%d, wzr, w%d\n", dst, operand->reg);
					break;
				case UNARY_OP_LOGICAL_NEGATION:
					fprintf(fp, "\tcmp w%d, #0\n", operand->reg);
					fprintf(fp, "\tcset w%d, eq\n", dst);
					break;
            }
			exprNode->reg = dst;
			pushPoll(&xpoll, operand->reg);
		}
		else if(exprNode->dataType == FLOAT_TYPE) {
			int dst = popPoll(&vpoll);
			int tmp = popPoll(&xpoll);
			switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp) {
				case UNARY_OP_POSITIVE:
					fprintf(fp, "\tfmov s%d, s%d\n", dst, operand->reg);
					break;
				case UNARY_OP_NEGATIVE:
					fprintf(fp, "\tfneg s%d, s%d\n", dst, operand->reg);
					break;
				case UNARY_OP_LOGICAL_NEGATION:
					fprintf(fp, "\tfcmp s%d, #0.0\n", operand->reg);
					fprintf(fp, "\tcset w%d, eq\n", tmp);
					fprintf(fp, "\tscvtf s%d, w%d\n", dst, tmp);
					break;
            }
			exprNode->reg = dst;
			pushPoll(&vpoll, operand->reg);
			pushPoll(&xpoll, tmp);
		}
		else {
			printf("error in line %d\n", __LINE__);
			exit(1);
		}
    }
}

void CG_processConstValueNode(AST_NODE* constValueNode)
{
    switch(constValueNode->semantic_value.const1->const_type) {
		case INTEGERC: {
			fprintf(fp, ".data\n");
			fprintf(fp, "_CONSTANT_%d: .word %d\n", constCount, constValueNode->semantic_value.const1->const_u.intval);
			fprintf(fp, ".text\n");
			int xReg = popPoll(&xpoll);
			fprintf(fp, "\tldr w%d, _CONSTANT_%d\n", xReg, constCount);
			constValueNode->reg = xReg;
			constCount++;
			break;
		}
		case FLOATC: {
			fprintf(fp, ".data\n");
			float fval = constValueNode->semantic_value.const1->const_u.fval;
			fprintf(fp, "_CONSTANT_%d: .word 0x%x\n", constCount, *(int*)&fval);
			fprintf(fp, ".text\n");
			int vReg = popPoll(&vpoll);
			fprintf(fp, "\tldr s%d, _CONSTANT_%d\n", vReg, constCount);
			constValueNode->reg = vReg;
			constCount++;
			break;
		}
		case STRINGC: {
			fprintf(fp, ".data\n");
			char buf[256];
			memset(buf, 0, 256);
			int len = strlen(constValueNode->semantic_value.const1->const_u.sc);
			char* ptr = constValueNode->semantic_value.const1->const_u.sc;
			ptr++;
			strncpy(buf, ptr, len - 2);
			fprintf(fp, "_CONSTANT_%d: .ascii \"%s\\000\"\n", constCount, buf);
			fprintf(fp, ".align 3\n");
			fprintf(fp, ".text\n");
			int xReg = popPoll(&xpoll);
			fprintf(fp, "\tldr x%d, =_CONSTANT_%d\n", xReg, constCount);
			constValueNode->reg = xReg;
			constCount++;
			break;
		}
    }

}

void CG_processVariableRValue(AST_NODE* idNode)
{
    SymbolTableEntry *symbolTableEntry = retrieveSymbol(idNode->semantic_value.identifierSemanticValue.identifierName);
    
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;
     
    TypeDescriptor *typeDescriptor = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        
    if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID)
    {
        if(typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
            /* we don't handle array reference in hw5 */
        }
        else {
            if(idNode->dataType == INT_TYPE) {
				if(symbolTableEntry->nestingLevel == 0) { // global variable reference
					int xReg = popPoll(&xpoll);
					int xReg2 = popPoll(&xpoll);
					fprintf(fp, "\tldr x%d, =_g_%s\n", xReg, idNode->semantic_value.identifierSemanticValue.identifierName);
					fprintf(fp, "\tldr w%d, [x%d, #0]\n", xReg2, xReg);
					pushPoll(&xpoll, xReg);
					idNode->reg = xReg2;
				}
				else { // local variable reference
					int xReg = popPoll(&xpoll);
					fprintf(fp, "\tldr w%d, [x29, #-%d]\n", xReg, symbolTableEntry->addr);
					idNode->reg = xReg;
				}
			}
			else {
				if(symbolTableEntry->nestingLevel == 0) { // global variable reference
					int xReg = popPoll(&xpoll);
					int vReg = popPoll(&xpoll);
					fprintf(fp, "\tldr x%d, =_g_%s\n", xReg, idNode->semantic_value.identifierSemanticValue.identifierName);
					fprintf(fp, "\tldr s%d, [x%d, #0]\n", vReg, xReg);
					pushPoll(&xpoll, xReg);
					idNode->reg = vReg;
				}
				else { // local variable reference
					int vReg = popPoll(&vpoll);
					fprintf(fp, "\tldr s%d, [x29, #-%d]\n", vReg, symbolTableEntry->addr);
					idNode->reg = vReg;
				}
			}
        }
    }
    else if(idNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID) {
		/* we don't handle array reference in hw5 */
    }
}

void CG_checkFunctionCall(AST_NODE* functionCallNode, int asExpr)
{
    AST_NODE* functionIDNode = functionCallNode->child;

    //special case
	//TODO: lib call
    if(strcmp(functionIDNode->semantic_value.identifierSemanticValue.identifierName, "write") == 0) {
        CG_checkWriteFunction(functionCallNode);
        return;
    }
	if(strcmp(functionIDNode->semantic_value.identifierSemanticValue.identifierName, "read") == 0) {
		fprintf(fp, "\tbl _read_int\n");
		if(asExpr) {
			int xReg = popPoll(&xpoll);
			fprintf(fp, "\tmov w%d, w0\n", xReg);
			functionCallNode->reg = xReg;
		}
		return;
	}
	if(strcmp(functionIDNode->semantic_value.identifierSemanticValue.identifierName, "fread") == 0) {
		fprintf(fp, "\tbl _read_float\n");
		if(asExpr) {
			int vReg = popPoll(&vpoll);
			fprintf(fp, "\tfmov s%d, s0\n", vReg);
			functionCallNode->reg = vReg;
		}
		return;
	}

    SymbolTableEntry* symbolTableEntry = retrieveSymbol(functionIDNode->semantic_value.identifierSemanticValue.identifierName);
    functionIDNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry;

	/* we don't support parameter passing in hw5 */
	fprintf(fp, "\tbl _start_%s\n", symbolTableEntry->name);
	if(asExpr) {
		if(functionCallNode->dataType == INT_TYPE) {
			int xReg = popPoll(&xpoll);
			fprintf(fp, "\tmov w%d, w0\n", xReg);
			functionCallNode->reg = xReg;
		}
		else {
			int vReg = popPoll(&vpoll);
			fprintf(fp, "\tfmov s%d, s0\n", vReg);
			functionCallNode->reg = vReg;
		}
	}
}

void CG_processVariableLValue(AST_NODE* idNode, int* isGlobal, int* addrOrReg)
{
    SymbolTableEntry *symbolTableEntry = retrieveSymbol(idNode->semantic_value.identifierSemanticValue.identifierName);
    
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = symbolTableEntry; 
    TypeDescriptor *typeDescriptor = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        
    if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID) {
         if(symbolTableEntry->nestingLevel == 0) { // global variable
			*isGlobal = 1;
			*addrOrReg = popPoll(&xpoll);
			fprintf(fp, "\tldr x%d, =_g_%s\n", *addrOrReg, idNode->semantic_value.identifierSemanticValue.identifierName);
		 }
		 else { // local variable
			*isGlobal = 0;
			*addrOrReg = symbolTableEntry->addr;
		}
    }
    else if(idNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID) {
        /* we don't support */
    }
}

void CG_checkWriteFunction(AST_NODE* functionCallNode)
{
    AST_NODE* functionIDNode = functionCallNode->child;

    AST_NODE* actualParameterList = functionIDNode->rightSibling;
	int useless = 0;
    CG_processGeneralNode(actualParameterList, &useless);

    AST_NODE* actualParameter = actualParameterList->child;
    
    if(actualParameter->dataType == INT_TYPE) {
		fprintf(fp, "\tmov w0, w%d\n", actualParameter->reg);
		fprintf(fp, "\tbl _write_int\n");
		pushPoll(&xpoll, actualParameter->reg);
	}
	else if(actualParameter->dataType == FLOAT_TYPE) {
		fprintf(fp, "\tfmov s0, s%d\n", actualParameter->reg);
		fprintf(fp, "\tbl _write_float\n");
		pushPoll(&vpoll, actualParameter->reg);
	}
	else {
		fprintf(fp, "\tmov x0, x%d\n", actualParameter->reg);
		fprintf(fp, "\tbl _write_str\n");
		pushPoll(&xpoll, actualParameter->reg);
	}
}

void CG_checkReturnStmt(AST_NODE* returnNode)
{
	/* search function idNode */
	AST_NODE* parentNode = returnNode->parent;
	AST_NODE* idNode;
    while(parentNode) {
        if(parentNode->nodeType == DECLARATION_NODE) {
            if(parentNode->semantic_value.declSemanticValue.kind == FUNCTION_DECL) {
                idNode = parentNode->child->rightSibling;
            }
            break;
        }
        parentNode = parentNode->parent;
    }

    if(returnNode->child->nodeType == NUL_NODE) {
        fprintf(fp, "\tb _end_%s\n", idNode->semantic_value.identifierSemanticValue.identifierName);
    }
    else {
		AST_NODE* exprNode = returnNode->child;
        CG_processExprRelatedNode(exprNode);
        if(returnNode->dataType == INT_TYPE) {
			fprintf(fp, "\tmov w0, w%d\n", exprNode->reg);
			fprintf(fp, "\tb _end_%s\n", idNode->semantic_value.identifierSemanticValue.identifierName);
			pushPoll(&xpoll, exprNode->reg);
		}
		else {
			fprintf(fp, "\tfmov s0, s%d\n", exprNode->reg);
			fprintf(fp, "\tb _end_%s\n", idNode->semantic_value.identifierSemanticValue.identifierName);
			pushPoll(&vpoll, exprNode->reg);
		}
    }
 
}

void CG_checkWhileStmt(AST_NODE* whileNode, int* frameSize)
{
    AST_NODE* boolExpression = whileNode->child;
    AST_NODE* bodyNode = boolExpression->rightSibling;
	int labno = ctrlCount;
	ctrlCount++;
	fprintf(fp, "_Test%d:", labno);
	CG_processExprRelatedNode(boolExpression);
	/* we don't support short-circuit boolean expression in hw5 */
	if(boolExpression->dataType == INT_TYPE) {
		fprintf(fp, "\tcmp w%d, #0\n", boolExpression->reg);
		fprintf(fp, "\tbeq _Lexit%d\n", labno);
		pushPoll(&xpoll, boolExpression->reg);
	}
	else {
		fprintf(fp, "\tfcmp s%d, #0.0\n", boolExpression->reg);
		fprintf(fp, "\tbeq _Lexit%d\n", labno);
		pushPoll(&vpoll, boolExpression->reg);
	}
    CG_processStmtNode(bodyNode, frameSize);
	fprintf(fp, "\tb _Test%d\n", labno);
	fprintf(fp, "_Lexit%d:\n", labno);
}

void CG_processBlockNode(AST_NODE* blockNode, int* frameSize)
{
    openScope();

    AST_NODE *traverseListNode = blockNode->child;
    while(traverseListNode)
    {
        CG_processGeneralNode(traverseListNode, frameSize);
        traverseListNode = traverseListNode->rightSibling;
    }
	//dumpSymbolTable();
    closeScope();
}

void CG_checkIfStmt(AST_NODE* ifNode, int* frameSize)
{
    AST_NODE* boolExpression = ifNode->child;
	AST_NODE* ifBodyNode = boolExpression->rightSibling;
    AST_NODE* elsePartNode = ifBodyNode->rightSibling;
    int labno = ctrlCount;
	ctrlCount++;
	CG_processExprRelatedNode(boolExpression);
	if(boolExpression->dataType == INT_TYPE) {
		fprintf(fp, "\tcmp w%d, #0\n", boolExpression->reg);
		fprintf(fp, "\tbeq _Lelse%d\n", labno);
		pushPoll(&xpoll, boolExpression->reg);
	}
	else {
		fprintf(fp, "\tfcmp s%d, #0.0\n", boolExpression->reg);
		fprintf(fp, "\tbeq _Lelse%d\n", labno);
		pushPoll(&vpoll, boolExpression->reg);
	}
	CG_processStmtNode(ifBodyNode, frameSize);
	fprintf(fp, "\tb _Lexit%d\n", labno);
	fprintf(fp, "_Lelse%d:\n", labno);
    CG_processStmtNode(elsePartNode, frameSize);
	fprintf(fp, "_Lexit%d:\n", labno);
}
