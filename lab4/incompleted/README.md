# Symbol Table Implementation - KPL Toy Compiler

## Tổng quan
Module bảng ký hiệu (symbol table) cho toy compiler ngôn ngữ KPL, hỗ trợ phân tích ngữ nghĩa (semantic analysis).

## Cấu trúc dữ liệu chính

### 1. Type System
- **TP_INT**: Kiểu số nguyên
- **TP_CHAR**: Kiểu ký tự
- **TP_ARRAY**: Kiểu mảng (có `arraySize` và `elementType`)

#### Các hàm Type:
- `makeIntType()`, `makeCharType()`, `makeArrayType()`: Tạo type
- `duplicateType()`: Copy sâu Type (đệ quy với mảng)
- `compareType()`: So sánh 2 Type (đệ quy với mảng)
- `freeType()`: Giải phóng Type (đệ quy với elementType)

### 2. ConstantValue
Lưu giá trị hằng số (int hoặc char) với union.

#### Các hàm ConstantValue:
- `makeIntConstant()`, `makeCharConstant()`: Tạo hằng số
- `duplicateConstantValue()`: Copy ConstantValue

### 3. Object System
7 loại Object: `CONSTANT`, `TYPE`, `VARIABLE`, `FUNCTION`, `PROCEDURE`, `PARAMETER`, `PROGRAM`

Mỗi Object có:
- `name`: Tên định danh (tối đa 15 ký tự)
- `kind`: Loại object
- Union chứa attributes tương ứng

#### Các hàm Object:
- `createConstantObject()`: Tạo object hằng số
- `createTypeObject()`: Tạo object kiểu
- `createVariableObject()`: Tạo object biến
- `createFunctionObject()`: Tạo object hàm (tự động tạo scope)
- `createProcedureObject()`: Tạo object thủ tục (tự động tạo scope)
- `createParameterObject()`: Tạo object tham số (liên kết với owner)
- `createProgramObject()`: Tạo object chương trình (tự động tạo scope global)

### 4. Scope - Bảng ký hiệu phân cấp
Scope quản lý các object trong một khối (block):
- `objList`: Danh sách object trong scope
- `owner`: Object chủ (program/function/procedure)
- `outer`: Scope bao ngoài (parent scope)

#### Quản lý Scope:
- `createScope()`: Tạo scope mới
- `enterBlock()`: Vào một scope (cập nhật currentScope)
- `exitBlock()`: Thoát scope (quay về outer scope)
- `declareObject()`: Khai báo object vào currentScope
  - Tự động thêm parameter vào paramList của function/procedure owner
  - Thêm object vào objList của currentScope

### 5. SymTab - Bảng ký hiệu toàn cục
- `program`: Object chương trình chính
- `currentScope`: Scope hiện tại đang xử lý
- `globalObjectList`: Các hàm/thủ tục built-in (READI, WRITEI, READC, WRITEC, WRITELN)

## Quản lý bộ nhớ

### Nguyên tắc:
1. **freeType**: Đệ quy free elementType trước khi free type (với mảng)
2. **freeObject**: Free attributes tùy theo kind:
   - Constant: free value
   - Type: free actualType
   - Variable: free type (không free scope vì là reference)
   - Function/Procedure: 
     - `freeReferenceList` cho paramList (chỉ free node, không free object)
     - free returnType (function)
     - free scope (đệ quy)
   - Parameter: free type (không free function owner vì là reference)
   - Program: free scope (đệ quy)

3. **freeScope**: Free objList rồi free scope
4. **freeObjectList**: Duyệt và free từng object và node
5. **freeReferenceList**: Chỉ free node, không free object (dùng cho paramList)

### Tại sao có freeReferenceList?
- `paramList` trong Function/Procedure chứa **reference** đến các Parameter objects
- Các Parameter objects thực sự được lưu trong `objList` của scope
- Khi free scope, objList sẽ free các Parameter objects
- Do đó paramList chỉ cần free các node, không free object

## Debug Functions

### printType(Type*)
In kiểu: `Int`, `Char`, hoặc `Arr(size,elementType)`

### printObject(Object*, indent)
In object với thông tin chi tiết:
- Constant: tên và giá trị
- Type: tên và kiểu thực
- Variable: tên và kiểu
- Parameter: tên, kind (VALUE/REFERENCE) và kiểu
- Function: tên, kiểu trả về và scope (đệ quy)
- Procedure: tên và scope (đệ quy)
- Program: tên và scope (đệ quy)

### printObjectList(ObjectNode*, indent)
In danh sách object

### printScope(Scope*, indent)
In toàn bộ scope (gọi printObjectList)

## Biên dịch và chạy

```bash
# Biên dịch
make clean
make

# Chạy
./symtab
```

## Output mẫu

```
Program PRG
    Const c1 = 10
    Const c2 = 'a'
    Type t1 = Arr(10,Int)
    Var v1 : Int
    Var v2 : Arr(10,Arr(10,Int))
    Function f : Int
        Param p1 : Int
        Param VAR p2 : Char

    Procedure p
        Param v1 : Int
        Const c1 = 'a'
        Const c3 = 10
        Type t1 = Int
        Type t2 = Arr(10,Int)
        Var v2 : Arr(10,Int)
        Var v3 : Char
```

## Các điểm quan trọng

### 1. Scope nesting (Phân cấp scope)
- Program scope (global) là root
- Function/Procedure scope có outer trỏ về scope chứa nó
- Khi enterBlock, currentScope được cập nhật
- Khi exitBlock, currentScope quay về outer

### 2. Parameter handling
- Parameter được tạo với owner là function/procedure chủ
- Khi declareObject(parameter), nó được thêm vào:
  - `paramList` của owner (function/procedure)
  - `objList` của currentScope (scope của function/procedure)

### 3. Reference vs Ownership
- **Ownership**: Object sở hữu và chịu trách nhiệm free
  - Scope owns objList objects
  - Type owns elementType (array)
  - Object owns attributes
  
- **Reference**: Chỉ tham chiếu, không free
  - paramList references parameters (actual objects in objList)
  - parameter->function references owner
  - variable->scope có thể là reference (tùy design)

### 4. Built-in functions
initSymTab() tạo sẵn 5 hàm/thủ tục built-in:
- READC(): Char
- READI(): Int
- WRITEI(i: Int)
- WRITEC(ch: Char)
- WRITELN()

## Lưu ý khi mở rộng

1. **Kiểm tra trùng tên**: declareObject nên kiểm tra trùng tên trong cùng scope
2. **findObject đa cấp**: Cần implement hàm tìm kiếm từ currentScope lên outer scopes
3. **Type checking**: compareType đã hỗ trợ so sánh mảng đa chiều
4. **Memory leak**: Luôn đảm bảo mọi malloc đều có free tương ứng

## Author
Implementation for Compiler Construction Lab - Semantic Analysis
