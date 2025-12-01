# KPL Parser - Lab 3: Compiler Construction

## 📖 Giới Thiệu

Parser cho ngôn ngữ KPL (K Programming Language) được xây dựng theo mô hình **đệ quy xuống** (recursive-descent parser) và tuân thủ văn phạm **LL(1)**.

**Môn học**: Experiment in Compiler Construction – Parser Design  
**Giảng viên**: Nguyễn Hữu Đức  
**Trường**: Khoa Công nghệ Thông tin, Đại học Bách khoa Hà Nội

---

## 🎯 Tính Năng

### ✅ Tính năng cơ bản (KPL Standard)

- ✅ Khai báo hằng số (CONST)
- ✅ Khai báo kiểu (TYPE)
- ✅ Khai báo biến (VAR)
- ✅ Khai báo hàm (FUNCTION)
- ✅ Khai báo thủ tục (PROCEDURE)
- ✅ Các câu lệnh: ASSIGN, CALL, IF...THEN...ELSE, WHILE...DO, FOR...TO...DO
- ✅ Biểu thức số học với +, -, *, /
- ✅ Điều kiện với =, !=, <, <=, >, >=
- ✅ Mảng với ARRAY...OF
- ✅ Tham số và đối số

### 🆕 Tính năng mở rộng (Extended Features)

- 🆕 **REPEAT...UNTIL**: Vòng lặp kiểm tra điều kiện sau khi thực thi
- 🆕 **Multiple Assignment**: Gán nhiều biến cùng lúc (x, y, z := 1, 2, 3)

---

## 📁 Cấu Trúc Project

```
lab3/
├── incompleted/          # Source code parser
│   ├── parser.c          # ⭐ File chính: cài đặt parser
│   ├── parser.h          # Header file parser
│   ├── scanner.c         # Scanner (lexical analyzer)
│   ├── scanner.h
│   ├── token.c           # ⭐ Token definitions
│   ├── token.h           # ⭐ Thêm KW_REPEAT, KW_UNTIL
│   ├── reader.c          # Đọc source code
│   ├── reader.h
│   ├── charcode.c        # Phân loại ký tự
│   ├── charcode.h
│   ├── error.c           # Xử lý lỗi
│   ├── error.h
│   ├── main.c            # Entry point
│   └── Makefile          # Build configuration
├── test/                 # Test cases
│   ├── example1.kpl      # ✅ Chương trình đơn giản
│   ├── example2.kpl      # ✅ Có function, statements
│   ├── example3.kpl      # ✅ Tower of Hanoi (phức tạp)
│   ├── test_repeat.kpl   # 🆕 Test REPEAT...UNTIL
│   ├── test_multi_assign.kpl  # 🆕 Test Multiple Assignment
│   ├── test_combined.kpl      # 🆕 Test cả hai
│   └── advanced_example.kpl   # 🆕 Ví dụ nâng cao
├── EXTENDED_FEATURES.md  # 📚 Hướng dẫn chi tiết tính năng mới
└── SUMMARY.md            # 📋 Tóm tắt nhanh
```

---

## 🚀 Cách Sử Dụng

### 1. Biên dịch

```bash
cd incompleted
make clean
make
```

### 2. Chạy parser

```bash
# Syntax: ./parser <input_file.kpl>
./parser ../test/example1.kpl
./parser ../test/example2.kpl
./parser ../test/test_repeat.kpl
```

### 3. Kết quả

Parser sẽ in ra:
- Từng token được nhận dạng (format: `line-col:TOKEN_TYPE(value)`)
- Các assert messages (ví dụ: "Parsing a Block ....")
- "Program parsed!" nếu thành công
- Thông báo lỗi nếu có lỗi cú pháp

---

## 📝 Ví Dụ

### Chương trình KPL cơ bản

```pascal
PROGRAM Example1;
BEGIN
END.
```

### Sử dụng REPEAT...UNTIL (🆕)

```pascal
PROGRAM TestRepeat;
VAR i : INTEGER;
    sum : INTEGER;

BEGIN
  i := 1;
  sum := 0;
  REPEAT
    sum := sum + i;
    i := i + 1
  UNTIL i > 10;
  CALL WriteI(sum)
END.
```

### Sử dụng Multiple Assignment (🆕)

```pascal
PROGRAM TestMultiAssign;
VAR x : INTEGER;
    y : INTEGER;
    z : INTEGER;

BEGIN
  (* Gán nhiều biến cùng lúc *)
  x, y, z := 1, 2, 3;
  
  (* Swap 2 biến *)
  x, y := y, x;
  
  CALL WriteI(x);
  CALL WriteI(y);
  CALL WriteI(z)
END.
```

---

## 📚 Tài Liệu

### Văn phạm BNF (Cơ bản)

```
Program → PROGRAM IDENT ; Block .
Block → ConstDecls TypeDecls VarDecls SubDecls BEGIN Statements END

ConstDecls → CONST ConstDecl ConstDecls | ε
ConstDecl → IDENT = Constant ;
TypeDecls → TYPE TypeDecl TypeDecls | ε
TypeDecl → IDENT = Type ;
VarDecls → VAR VarDecl VarDecls | ε
VarDecl → IDENT : Type ;

Statement → AssignSt | CallSt | IfSt | WhileSt | ForSt | RepeatSt | ε
AssignSt → Variable := Expression
CallSt → CALL IDENT Arguments
IfSt → IF Condition THEN Statement ElseSt
WhileSt → WHILE Condition DO Statement
ForSt → FOR IDENT := Expression TO Expression DO Statement
RepeatSt → REPEAT Statements UNTIL Condition  (🆕)

Expression → Term Expression2
Term → Factor Term2
Factor → NUMBER | CHAR | IDENT | ( Expression )
```

### Văn phạm Mở rộng (🆕)

```
RepeatSt → REPEAT Statements UNTIL Condition
AssignSt → Variable (, Variable)* := Expression (, Expression)*
```

---


Chạy tất cả test:

```bash
cd incompleted
./parser ../test/example1.kpl
./parser ../test/example2.kpl
./parser ../test/example3.kpl
./parser ../test/test_repeat.kpl
./parser ../test/test_multi_assign.kpl
./parser ../test/test_combined.kpl
./parser ../test/advanced_example.kpl
```

---

## 🔍 Kiến Trúc Parser

### Mô hình Đệ quy Xuống

Parser sử dụng mô hình **recursive-descent**:

```c
Token *currentToken;  // Token hiện tại
Token *lookAhead;     // Token nhìn trước (để quyết định sản xuất)

void scan(void) {
  // Di chuyển lookAhead → currentToken
  // Lấy token mới làm lookAhead
}

void eat(TokenType tokenType) {
  // Kiểm tra và duyệt terminal
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else {
    error(...);
  }
}

void compileXXX(void) {
  // Mỗi nonterminal có một hàm riêng
  // Dùng switch-case với lookAhead để chọn sản xuất
}
```

### Nguyên Tắc LL(1)

- **L**eft-to-right scan: Đọc từ trái sang phải
- **L**eftmost derivation: Dẫn xuất trái nhất
- **1** symbol lookahead: Chỉ cần nhìn trước 1 token

**Điều kiện**:
- ✅ Không đệ quy trái
- ✅ Đã nhân tử trái
- ✅ FIRST(α) ∩ FIRST(β) = ∅
- ✅ Nếu α →* ε thì FIRST(β) ∩ FOLLOW(A) = ∅



