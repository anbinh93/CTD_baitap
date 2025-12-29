/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void error(ErrorCode err, int lineNo, int colNo) {
    switch (err) {
    case ERR_END_OF_COMMENT:
        printf("%d-%d: End of comment expected\n", lineNo, colNo);
        break;
    case ERR_INVALID_CHAR:
        printf("%d-%d: Invalid character\n", lineNo, colNo);
        break;
    case ERR_INVALID_CONSTANT:
        printf("%d-%d: Invalid constant\n", lineNo, colNo);
        break;
    case ERR_INVALID_SYMBOL:
        printf("%d-%d: Invalid symbol\n", lineNo, colNo);
        break;
    case ERR_IDENT_TOO_LONG:
        printf("%d-%d: Identifier too long\n", lineNo, colNo);
        break;
    case ERR_INVALID_IDENT:
        printf("%d-%d: Invalid identifier\n", lineNo, colNo);
        break;
    case ERR_INVALID_PROGRAM:
        printf("%d-%d: Invalid program\n", lineNo, colNo);
        break;
    case ERR_INVALID_STATEMENT:
        printf("%d-%d: Invalid statement\n", lineNo, colNo);
        break;
    case ERR_INVALID_EXPRESSION:
        printf("%d-%d: Invalid expression\n", lineNo, colNo);
        break;
    case ERR_INVALID_TERM:
        printf("%d-%d: Invalid term\n", lineNo, colNo);
        break;
    case ERR_INVALID_FACTOR:
        printf("%d-%d: Invalid factor\n", lineNo, colNo);
        break;
    case ERR_INVALID_BASICTYPE:
        printf("%d-%d: Invalid basic type\n", lineNo, colNo);
        break;
    case ERR_TYPE_INCONSISTENCY:
        printf("%d-%d: Type inconsistency\n", lineNo, colNo);
        break;
    case ERR_INVALID_ARGUMENT:
        printf("%d-%d: Invalid argument\n", lineNo, colNo);
        break;
    case ERR_INVALID_PARAMETER:
        printf("%d-%d: Invalid parameter\n", lineNo, colNo);
        break;
    case ERR_UNDECLARED_IDENT:
        printf("%d-%d: Undeclared identifier\n", lineNo, colNo);
        break;
    case ERR_DUPLICATE_IDENT:
        printf("%d-%d: Duplicate identifier\n", lineNo, colNo);
        break;
    case ERR_UNDECLARED_TYPE:
        printf("%d-%d: Undeclared type\n", lineNo, colNo);
        break;
    case ERR_INVALID_LVALUE:
        printf("%d-%d: Invalid LValue\n", lineNo, colNo);
        break;
    case ERR_UNDECLARED_FUNCTION:
        printf("%d-%d: Undeclared function\n", lineNo, colNo);
        break;
    case ERR_UNDECLARED_PROCEDURE:
        printf("%d-%d: Undeclared procedure\n", lineNo, colNo);
        break;
    case ERR_PARAMETERS_TOO_MANY:
        printf("%d-%d: Too many arguments\n", lineNo, colNo);
        break;
    case ERR_PARAMETERS_TOO_FEW:
        printf("%d-%d: Too few arguments\n", lineNo, colNo);
        break;
    case ERR_INVALID_ARRAY_ACCESS:
        printf("%d-%d: Invalid array access\n", lineNo, colNo);
        break;
    }
    exit(1);
}

void missingToken(int tokenType, int lineNo, int colNo) {
    printf("%d-%d: Missing token %d\n", lineNo, colNo, tokenType);
    exit(1);
}

void assert(char *msg) {
    printf("Error: %s\n", msg);
    exit(1);
}
