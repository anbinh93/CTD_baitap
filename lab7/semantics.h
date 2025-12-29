/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __SEMANTICS_H__
#define __SEMANTICS_H__

#include "symtab.h"

void checkIntType(Type* type);
void checkCharType(Type* type);
void checkBasicType(Type* type);
void checkArrayType(Type* type);
void checkTypeEquality(Type* type1, Type* type2);

void checkValidConstant(Type* type);
void checkValidAssignment(Type* lvalueType, Type* exprType);
void checkValidForLoop(Type* varType, Type* exp1Type, Type* exp2Type);
void checkValidCondition(Type* exp1Type, Type* exp2Type);
Type* checkValidIndexing(Type* arrayType, Type* indexType);
void checkValidArithmeticOp(Type* type);
void checkValidParameter(Object* param, Type* argType, int isLValue);

#endif
