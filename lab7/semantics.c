/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "semantics.h"
#include "error.h"

int currentLine = 1;
int currentCol = 1;

void checkIntType(Type* type) {
    if (type == NULL || type->typeClass != TP_INT)
        error(ERR_TYPE_INCONSISTENCY, currentLine, currentCol);
}

void checkCharType(Type* type) {
    if (type == NULL || type->typeClass != TP_CHAR)
        error(ERR_TYPE_INCONSISTENCY, currentLine, currentCol);
}

void checkBasicType(Type* type) {
    if (type == NULL || type->typeClass == TP_ARRAY)
        error(ERR_TYPE_INCONSISTENCY, currentLine, currentCol);
}

void checkArrayType(Type* type) {
    if (type == NULL || type->typeClass != TP_ARRAY)
        error(ERR_TYPE_INCONSISTENCY, currentLine, currentCol);
}

void checkTypeEquality(Type* type1, Type* type2) {
    if (type1 == NULL || type2 == NULL) {
        error(ERR_TYPE_INCONSISTENCY, currentLine, currentCol);
        return;
    }
    
    if (type1->typeClass != type2->typeClass) {
        error(ERR_TYPE_INCONSISTENCY, currentLine, currentCol);
        return;
    }
    
    if (type1->typeClass == TP_ARRAY) {
        if (type1->arraySize != type2->arraySize) {
            error(ERR_TYPE_INCONSISTENCY, currentLine, currentCol);
            return;
        }
        checkTypeEquality(type1->elementType, type2->elementType);
    }
}

void checkValidConstant(Type* type) {
    checkIntType(type);
}

void checkValidAssignment(Type* lvalueType, Type* exprType) {
    checkBasicType(lvalueType);
    checkBasicType(exprType);
    checkTypeEquality(lvalueType, exprType);
}

void checkValidForLoop(Type* varType, Type* exp1Type, Type* exp2Type) {
    checkBasicType(varType);
    checkBasicType(exp1Type);
    checkBasicType(exp2Type);
    checkTypeEquality(varType, exp1Type);
    checkTypeEquality(exp1Type, exp2Type);
}

void checkValidCondition(Type* exp1Type, Type* exp2Type) {
    checkBasicType(exp1Type);
    checkBasicType(exp2Type);
    checkTypeEquality(exp1Type, exp2Type);
}

Type* checkValidIndexing(Type* arrayType, Type* indexType) {
    checkArrayType(arrayType);
    checkIntType(indexType);
    
    if (arrayType != NULL && arrayType->typeClass == TP_ARRAY)
        return arrayType->elementType;
    return NULL;
}

void checkValidArithmeticOp(Type* type) {
    checkIntType(type);
}

void checkValidParameter(Object* param, Type* argType, int isLValue) {
    if (param == NULL || param->kind != OBJ_PARAMETER) {
        error(ERR_INVALID_PARAMETER, currentLine, currentCol);
        return;
    }
    
    checkTypeEquality(param->paramAttrs->type, argType);
    
    if (param->paramAttrs->kind == PARAM_REFERENCE && !isLValue)
        error(ERR_INVALID_ARGUMENT, currentLine, currentCol);
}
