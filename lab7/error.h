/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum {
    ERR_END_OF_COMMENT = 1,
    ERR_INVALID_CHAR,
    ERR_INVALID_CONSTANT,
    ERR_INVALID_SYMBOL,
    ERR_IDENT_TOO_LONG,
    
    ERR_INVALID_IDENT = 100,
    ERR_INVALID_PROGRAM,
    ERR_INVALID_STATEMENT,
    ERR_INVALID_EXPRESSION,
    ERR_INVALID_TERM,
    ERR_INVALID_FACTOR,
    ERR_INVALID_BASICTYPE,
    
    ERR_TYPE_INCONSISTENCY = 200,
    ERR_INVALID_ARGUMENT,
    ERR_INVALID_PARAMETER,
    ERR_UNDECLARED_IDENT,
    ERR_DUPLICATE_IDENT,
    ERR_UNDECLARED_TYPE,
    ERR_INVALID_LVALUE,
    ERR_UNDECLARED_FUNCTION,
    ERR_UNDECLARED_PROCEDURE,
    ERR_PARAMETERS_TOO_MANY,
    ERR_PARAMETERS_TOO_FEW,
    ERR_INVALID_ARRAY_ACCESS
} ErrorCode;

void error(ErrorCode err, int lineNo, int colNo);
void missingToken(int tokenType, int lineNo, int colNo);
void assert(char *msg);

#endif
