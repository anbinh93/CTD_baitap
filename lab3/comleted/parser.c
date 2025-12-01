/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

Token *currentToken;
Token *lookAhead;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  assert("Program parsed!");
}

void compileBlock(void) {
  assert("Parsing a Block ....");
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
    compileBlock2();
  } 
  else compileBlock2();
  assert("Block parsed!");
}

void compileBlock2(void) {
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileConstDecls(void) {
  // ConstDecls -> ConstDecl ConstDecls | ε
  // FIRST(ConstDecl) = {IDENT}, FOLLOW(ConstDecls) = {TYPE, VAR, FUNCTION, PROCEDURE, BEGIN}
  while (lookAhead->tokenType == TK_IDENT) {
    compileConstDecl();
  }
}

void compileConstDecl(void) {
  // ConstDecl -> IDENT = Constant ;
  eat(TK_IDENT);
  eat(SB_EQ);
  compileConstant();
  eat(SB_SEMICOLON);
}

void compileTypeDecls(void) {
  // TypeDecls -> TypeDecl TypeDecls | ε
  // FIRST(TypeDecl) = {IDENT}, FOLLOW(TypeDecls) = {VAR, FUNCTION, PROCEDURE, BEGIN}
  while (lookAhead->tokenType == TK_IDENT) {
    compileTypeDecl();
  }
}

void compileTypeDecl(void) {
  // TypeDecl -> IDENT = Type ;
  eat(TK_IDENT);
  eat(SB_EQ);
  compileType();
  eat(SB_SEMICOLON);
}

void compileVarDecls(void) {
  // VarDecls -> VarDecl VarDecls | ε
  // FIRST(VarDecl) = {IDENT}, FOLLOW(VarDecls) = {FUNCTION, PROCEDURE, BEGIN}
  while (lookAhead->tokenType == TK_IDENT) {
    compileVarDecl();
  }
}

void compileVarDecl(void) {
  // VarDecl -> IDENT : Type ;
  eat(TK_IDENT);
  eat(SB_COLON);
  compileType();
  eat(SB_SEMICOLON);
}

void compileSubDecls(void) {
  assert("Parsing subtoutines ....");
  // SubDecls -> FuncDecl SubDecls | ProcDecl SubDecls | ε
  // FIRST(FuncDecl) = {FUNCTION}, FIRST(ProcDecl) = {PROCEDURE}
  while (lookAhead->tokenType == KW_FUNCTION || lookAhead->tokenType == KW_PROCEDURE) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else
      compileProcDecl();
  }
  assert("Subtoutines parsed ....");
}

void compileFuncDecl(void) {
  assert("Parsing a function ....");
  // FuncDecl -> FUNCTION IDENT Params : BasicType ; Block ;
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Function parsed ....");
}

void compileProcDecl(void) {
  assert("Parsing a procedure ....");
  // ProcDecl -> PROCEDURE IDENT Params ; Block ;
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Procedure parsed ....");
}

void compileUnsignedConstant(void) {
  // UnsignedConstant -> NUMBER | CHAR
  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  default:
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileConstant(void) {
  // Constant -> + NUMBER | - NUMBER | NUMBER | CHAR
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    eat(TK_NUMBER);
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    eat(TK_NUMBER);
    break;
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  default:
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileConstant2(void) {
  // Constant2 -> ε (not used in current grammar)
}

void compileType(void) {
  // Type -> INTEGER | CHAR | ARRAY [NUMBER] OF Type
  switch (lookAhead->tokenType) {
  case KW_INTEGER:
    eat(KW_INTEGER);
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);  // [
    eat(TK_NUMBER);
    eat(SB_RSEL);  // ]
    eat(KW_OF);
    compileType();
    break;
  default:
    error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileBasicType(void) {
  // BasicType -> INTEGER | CHAR
  switch (lookAhead->tokenType) {
  case KW_INTEGER:
    eat(KW_INTEGER);
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    break;
  default:
    error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileParams(void) {
  // Params -> ( Param Params2 ) | ε
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    compileParams2();
    eat(SB_RPAR);
  }
}

void compileParams2(void) {
  // Params2 -> ; Param Params2 | ε
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileParam();
  }
}

void compileParam(void) {
  // Param -> IDENT : BasicType
  eat(TK_IDENT);
  eat(SB_COLON);
  compileBasicType();
}

void compileStatements(void) {
  // Statements -> Statement Statements2
  compileStatement();
  compileStatements2();
}

void compileStatements2(void) {
  // Statements2 -> ; Statement Statements2 | ε
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
  case KW_REPEAT:
    compileRepeatSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_UNTIL:
    break;
    // Error occurs
  default:
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileAssignSt(void) {
  assert("Parsing an assign statement ....");
  // AssignSt -> Variable , Variable , ... := Expression , Expression , ...
  //          OR Variable := Expression (single assignment)
  
  // First variable
  eat(TK_IDENT);
  compileIndexes();
  
  // Check for multiple assignment: x, y, z := ...
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
    compileIndexes();
  }
  
  eat(SB_ASSIGN);  // :=
  
  // First expression
  compileExpression();
  
  // Additional expressions for multiple assignment
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileExpression();
  }
  
  assert("Assign statement parsed ....");
}

void compileCallSt(void) {
  assert("Parsing a call statement ....");
  // CallSt -> CALL IDENT Arguments
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
  assert("Call statement parsed ....");
}

void compileGroupSt(void) {
  assert("Parsing a group statement ....");
  // GroupSt -> BEGIN Statements END
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
  assert("Group statement parsed ....");
}

void compileIfSt(void) {
  assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
  assert("If statement parsed ....");
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  assert("Parsing a while statement ....");
  // WhileSt -> WHILE Condition DO Statement
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  assert("While statement pased ....");
}

void compileForSt(void) {
  assert("Parsing a for statement ....");
  // ForSt -> FOR IDENT := Expression TO Expression DO Statement
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);  // :=
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
  assert("For statement parsed ....");
}

void compileRepeatSt(void) {
  assert("Parsing a repeat statement ....");
  // RepeatSt -> REPEAT Statements UNTIL Condition
  eat(KW_REPEAT);
  compileStatements();
  eat(KW_UNTIL);
  compileCondition();
  assert("Repeat statement parsed ....");
}

void compileArguments(void) {
  // Arguments -> ( Expression Arguments2 ) | ε
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileExpression();
    compileArguments2();
    eat(SB_RPAR);
  }
}

void compileArguments2(void) {
  // Arguments2 -> , Expression Arguments2 | ε
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileExpression();
  }
}

void compileCondition(void) {
  // Condition -> Expression Condition2
  compileExpression();
  compileCondition2();
}

void compileCondition2(void) {
  // Condition2 -> = Expression | != Expression | < Expression | <= Expression | > Expression | >= Expression
  switch (lookAhead->tokenType) {
  case SB_EQ:
    eat(SB_EQ);
    compileExpression();
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    compileExpression();
    break;
  case SB_LT:
    eat(SB_LT);
    compileExpression();
    break;
  case SB_LE:
    eat(SB_LE);
    compileExpression();
    break;
  case SB_GT:
    eat(SB_GT);
    compileExpression();
    break;
  case SB_GE:
    eat(SB_GE);
    compileExpression();
    break;
  default:
    error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileExpression(void) {
  assert("Parsing an expression");
  // Expression -> + Term Expression2 | - Term Expression2 | Term Expression2
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileTerm();
    compileExpression2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileTerm();
    compileExpression2();
    break;
  default:
    compileTerm();
    compileExpression2();
    break;
  }
  assert("Expression parsed");
}

void compileExpression2(void) {
  // Expression2 -> + Term Expression2 | - Term Expression2 | ε
  while (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS) {
    if (lookAhead->tokenType == SB_PLUS) {
      eat(SB_PLUS);
      compileTerm();
    } else {
      eat(SB_MINUS);
      compileTerm();
    }
  }
}

void compileExpression3(void) {
  // Expression3 -> ε (not used in current grammar)
}

void compileTerm(void) {
  // Term -> Factor Term2
  compileFactor();
  compileTerm2();
}

void compileTerm2(void) {
  // Term2 -> * Factor Term2 | / Factor Term2 | ε
  while (lookAhead->tokenType == SB_TIMES || lookAhead->tokenType == SB_SLASH) {
    if (lookAhead->tokenType == SB_TIMES) {
      eat(SB_TIMES);
      compileFactor();
    } else {
      eat(SB_SLASH);
      compileFactor();
    }
  }
}

void compileFactor(void) {
  // Factor -> UnsignedConstant | Variable | FunctionApplication | ( Expression )
  // Variable và FunctionApplication đều bắt đầu bằng IDENT
  switch (lookAhead->tokenType) {
  case TK_NUMBER:
  case TK_CHAR:
    compileUnsignedConstant();
    break;
  case TK_IDENT:
    // Cần phân biệt Variable và FunctionApplication
    // FunctionApplication: IDENT ( Arguments )
    // Variable: IDENT Indexes
    eat(TK_IDENT);
    if (lookAhead->tokenType == SB_LPAR) {
      // FunctionApplication
      compileArguments();
    } else {
      // Variable with possible indexes
      compileIndexes();
    }
    break;
  case SB_LPAR:
    eat(SB_LPAR);
    compileExpression();
    eat(SB_RPAR);
    break;
  default:
    error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileIndexes(void) {
  // Indexes -> [ Expression ] Indexes | ε
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);  // [
    compileExpression();
    eat(SB_RSEL);  // ]
  }
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
