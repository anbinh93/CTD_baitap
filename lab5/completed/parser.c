/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "debug.h"

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern SymTab* symtab;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  Object* program;

  eat(KW_PROGRAM);
  // Tạo đối tượng chương trình với tên từ currentToken
  program = createProgramObject(currentToken->string);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  // Vào block chính của chương trình
  enterBlock(program->progAttrs->scope);
  compileBlock();
  // Thoát block chính sau khi duyệt xong
  exitBlock();
  eat(SB_PERIOD);
}

void compileBlock(void) {
  Object* constObj;
  ConstantValue* constValue;

  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);

    do {
      // Tạo đối tượng hằng với tên từ lookAhead (trước khi eat)
      constObj = createConstantObject(lookAhead->string);
      eat(TK_IDENT);
      eat(SB_EQ);
      // Lấy giá trị hằng từ compileConstant
      constValue = compileConstant();
      constObj->constAttrs->value = constValue;
      // Khai báo hằng vào block hiện tại
      declareObject(constObj);
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock2();
  } 
  else compileBlock2();
}

void compileBlock2(void) {
  Object* typeObj;
  Type* actualType;

  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);

    do {
      // Tạo đối tượng kiểu với tên từ lookAhead
      typeObj = createTypeObject(lookAhead->string);
      eat(TK_IDENT);
      eat(SB_EQ);
      // Lấy kiểu thực tế từ compileType
      actualType = compileType();
      typeObj->typeAttrs->actualType = actualType;
      // Khai báo kiểu vào block hiện tại
      declareObject(typeObj);
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  Object* varObj;
  Type* varType;

  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);

    do {
      // Tạo đối tượng biến với tên từ lookAhead
      // createVariableObject đã tự động lưu scope hiện tại vào varAttrs->scope
      varObj = createVariableObject(lookAhead->string);
      eat(TK_IDENT);
      eat(SB_COLON);
      // Lấy kiểu của biến từ compileType
      varType = compileType();
      varObj->varAttrs->type = varType;
      // Khai báo biến vào block hiện tại
      declareObject(varObj);
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

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

void compileSubDecls(void) {
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else compileProcDecl();
  }
}
void compileFuncDecl(void) {
  Object* funcObj;
  Type* returnType;

  eat(KW_FUNCTION);
  // Tạo đối tượng hàm với tên từ lookAhead (trước khi eat TK_IDENT)
  // createFunctionObject đã tự động tạo scope với outer là currentScope
  funcObj = createFunctionObject(lookAhead->string);
  eat(TK_IDENT);
  // Khai báo hàm vào block hiện tại (block cha)
  declareObject(funcObj);
  // Chuyển sang block của hàm để khai báo các đối tượng cục bộ (params)
  enterBlock(funcObj->funcAttrs->scope);
  compileParams();
  eat(SB_COLON);
  // Lấy kiểu trả về từ compileBasicType
  returnType = compileBasicType();
  funcObj->funcAttrs->returnType = returnType;
  eat(SB_SEMICOLON);
  compileBlock();
  // Thoát khỏi block của hàm, quay lại block cha
  exitBlock();
  eat(SB_SEMICOLON);
}

void compileProcDecl(void) {
  Object* procObj;

  eat(KW_PROCEDURE);
  // Tạo đối tượng thủ tục với tên từ lookAhead
  // createProcedureObject đã tự động tạo scope với outer là currentScope
  procObj = createProcedureObject(lookAhead->string);
  eat(TK_IDENT);
  // Khai báo thủ tục vào block hiện tại (block cha)
  declareObject(procObj);
  // Chuyển sang block của thủ tục để khai báo các đối tượng cục bộ (params)
  enterBlock(procObj->procAttrs->scope);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  // Thoát khỏi block của thủ tục, quay lại block cha
  exitBlock();
  eat(SB_SEMICOLON);
}

ConstantValue* compileUnsignedConstant(void) {
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    // Tạo giá trị hằng số nguyên
    constValue = makeIntConstant(lookAhead->value);
    eat(TK_NUMBER);
    break;
  case TK_IDENT:
    // Tra bảng ký hiệu để lấy giá trị hằng từ định danh
    obj = lookupObject(lookAhead->string);
    if (obj == NULL)
      error(ERR_UNDECLARED_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    else if (obj->kind != OBJ_CONSTANT)
      error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    else
      constValue = duplicateConstantValue(obj->constAttrs->value);
    eat(TK_IDENT);
    break;
  case TK_CHAR:
    // Tạo giá trị hằng ký tự
    constValue = makeCharConstant(lookAhead->string[0]);
    eat(TK_CHAR);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

ConstantValue* compileConstant(void) {
  ConstantValue* constValue;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    constValue = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    constValue = compileConstant2();
    // Đảo dấu cho giá trị số nguyên
    if (constValue->type == TP_INT)
      constValue->intValue = -constValue->intValue;
    break;
  case TK_CHAR:
    // Tạo giá trị hằng ký tự
    constValue = makeCharConstant(lookAhead->string[0]);
    eat(TK_CHAR);
    break;
  default:
    constValue = compileConstant2();
    break;
  }
  return constValue;
}

ConstantValue* compileConstant2(void) {
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    // Tạo giá trị hằng số nguyên
    constValue = makeIntConstant(lookAhead->value);
    eat(TK_NUMBER);
    break;
  case TK_IDENT:
    // Tra bảng ký hiệu để lấy giá trị hằng từ định danh
    obj = lookupObject(lookAhead->string);
    if (obj == NULL)
      error(ERR_UNDECLARED_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    else if (obj->kind != OBJ_CONSTANT)
      error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    else
      constValue = duplicateConstantValue(obj->constAttrs->value);
    eat(TK_IDENT);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

Type* compileType(void) {
  Type* type;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    type = makeIntType();
    eat(KW_INTEGER);
    break;
  case KW_CHAR: 
    type = makeCharType();
    eat(KW_CHAR); 
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    // Lấy kích thước mảng
    arraySize = lookAhead->value;
    eat(TK_NUMBER);
    eat(SB_RSEL);
    eat(KW_OF);
    // Lấy kiểu phần tử (đệ quy)
    elementType = compileType();
    type = makeArrayType(arraySize, elementType);
    break;
  case TK_IDENT:
    // Tra bảng ký hiệu để lấy kiểu từ định danh
    obj = lookupObject(lookAhead->string);
    if (obj == NULL)
      error(ERR_UNDECLARED_TYPE, lookAhead->lineNo, lookAhead->colNo);
    else if (obj->kind != OBJ_TYPE)
      error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    else
      type = duplicateType(obj->typeAttrs->actualType);
    eat(TK_IDENT);
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

Type* compileBasicType(void) {
  Type* type;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    type = makeIntType();
    eat(KW_INTEGER); 
    break;
  case KW_CHAR: 
    type = makeCharType();
    eat(KW_CHAR); 
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void) {
  Object* paramObj;
  Type* paramType;
  enum ParamKind kind;

  switch (lookAhead->tokenType) {
  case TK_IDENT:
    // Tham số truyền theo giá trị (PARAM_VALUE)
    kind = PARAM_VALUE;
    // Tạo đối tượng tham số với owner là scope->owner hiện tại (function/procedure)
    paramObj = createParameterObject(lookAhead->string, kind, symtab->currentScope->owner);
    eat(TK_IDENT);
    eat(SB_COLON);
    // Lấy kiểu tham số
    paramType = compileBasicType();
    paramObj->paramAttrs->type = paramType;
    // declareObject sẽ tự động add vào paramList của owner VÀ objList của scope
    declareObject(paramObj);
    break;
  case KW_VAR:
    // Tham số truyền theo tham chiếu (PARAM_REFERENCE)
    eat(KW_VAR);
    kind = PARAM_REFERENCE;
    paramObj = createParameterObject(lookAhead->string, kind, symtab->currentScope->owner);
    eat(TK_IDENT);
    eat(SB_COLON);
    paramType = compileBasicType();
    paramObj->paramAttrs->type = paramType;
    declareObject(paramObj);
    break;
  default:
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileStatements(void) {
  compileStatement();
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
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileLValue(void) {
  eat(TK_IDENT);
  compileIndexes();
}

void compileAssignSt(void) {
  compileLValue();
  eat(SB_ASSIGN);
  compileExpression();
}

void compileCallSt(void) {
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
}

void compileGroupSt(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void) {
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

void compileForSt(void) {
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
}

void compileArgument(void) {
  compileExpression();
}

void compileArguments(void) {
  switch (lookAhead->tokenType) {
  case SB_LPAR:
    eat(SB_LPAR);
    compileArgument();

    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      compileArgument();
    }

    eat(SB_RPAR);
    break;
    // Check FOLLOW set 
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void) {
  compileExpression();
  switch (lookAhead->tokenType) {
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  default:
    error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }

  compileExpression();
}

void compileExpression(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileExpression2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileExpression2();
    break;
  default:
    compileExpression2();
  }
}

void compileExpression2(void) {
  compileTerm();
  compileExpression3();
}


void compileExpression3(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileTerm();
    compileExpression3();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileTerm();
    compileExpression3();
    break;
    // check the FOLLOW set
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileTerm(void) {
  compileFactor();
  compileTerm2();
}

void compileTerm2(void) {
  switch (lookAhead->tokenType) {
  case SB_TIMES:
    eat(SB_TIMES);
    compileFactor();
    compileTerm2();
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    compileFactor();
    compileTerm2();
    break;
    // check the FOLLOW set
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileFactor(void) {
  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    switch (lookAhead->tokenType) {
    case SB_LPAR:
      compileArguments();
      break;
    case SB_LSEL:
      compileIndexes();
      break;
    default:
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();

  printObject(symtab->program,0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
