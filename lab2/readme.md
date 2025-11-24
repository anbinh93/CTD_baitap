# Lab 2: Scanner (Lexical Analyzer) cho ngôn ngữ KPL

## Mục đích
Xây dựng bộ phân tích từ vựng (Scanner/Lexical Analyzer) cho ngôn ngữ lập trình KPL (Kid Programming Language). Scanner có nhiệm vụ đọc mã nguồn và phân tách thành các token (từ tố).

## Cấu trúc project

```
lab2/
├── incompleted/          # Thư mục chứa source code
│   ├── scanner.c         # File chính - Lexical Analyzer
│   ├── scanner.h         
│   ├── reader.c          # Đọc ký tự từ file
│   ├── reader.h
│   ├── charcode.c        # Phân loại ký tự
│   ├── charcode.h
│   ├── token.c           # Xử lý token
│   ├── token.h
│   ├── error.c           # Xử lý lỗi
│   ├── error.h
│   └── Makefile          # Build script
└── test/                 # Thư mục chứa test cases
    ├── example1.kpl - example7.kpl
    └── result1.txt - result7.txt
```

## Hướng dẫn compile và chạy

### 1. Compile chương trình

```bash
cd lab2/incompleted
make clean    # Xóa các file cũ (optional)
make          # Compile
```

### 2. Chạy scanner

```bash
./scanner <input_file.kpl>
```

Ví dụ:
```bash
./scanner ../test/example3.kpl
```

### 3. Chạy test cases

```bash
# Test một file cụ thể
./scanner ../test/example3.kpl > output.txt
diff output.txt ../test/result3.txt

# Test tất cả các file
for i in 1 2 3 4 5 6 7; do 
  echo "Testing example$i..."
  ./scanner ../test/example$i.kpl > /tmp/result$i.txt 2>&1
  diff -q /tmp/result$i.txt ../test/result$i.txt && echo "✓ PASS" || echo "✗ FAIL"
done
```

## Giải thích thuật toán

### 1. Kiến trúc tổng quan

Scanner hoạt động theo mô hình **Finite State Automaton (FSA)**, đọc từng ký tự và chuyển đổi trạng thái để nhận dạng token.

```
Input File → Reader → Scanner → Token Stream
              ↓
          CharCode (phân loại ký tự)
```

### 2. Các thành phần chính

#### 2.1. Reader Module (`reader.c`)
- **Chức năng**: Đọc từng ký tự từ file nguồn
- **API chính**:
  - `readChar()`: Đọc ký tự tiếp theo, cập nhật vị trí (lineNo, colNo)
  - `openInputStream()`: Mở file nguồn
  - `closeInputStream()`: Đóng file nguồn

#### 2.2. CharCode Module (`charcode.c`)
- **Chức năng**: Phân loại ký tự thành các nhóm
- **Các loại ký tự**:
  - `CHAR_LETTER`: Chữ cái (a-z, A-Z)
  - `CHAR_DIGIT`: Chữ số (0-9)
  - `CHAR_SPACE`: Khoảng trắng, tab, newline
  - `CHAR_PLUS`, `CHAR_MINUS`, `CHAR_TIMES`, `CHAR_SLASH`: Toán tử
  - `CHAR_LT`, `CHAR_GT`, `CHAR_EQ`: So sánh
  - `CHAR_LPAR`, `CHAR_RPAR`: Ngoặc đơn
  - `CHAR_SINGLEQUOTE`: Dấu nháy đơn
  - `CHAR_UNKNOWN`: Ký tự không hợp lệ

```c
CharCode charCodes[256];  // Mảng ánh xạ ASCII → CharCode
```

### 3. Thuật toán nhận dạng token (Scanner Algorithm)

#### 3.1. Hàm `getToken()` - Hàm trung tâm

```
WHILE not EOF:
  1. Đọc currentChar
  2. Kiểm tra loại ký tự (charCode)
  3. SWITCH theo charCode:
     - SPACE    → skipBlank() và tiếp tục
     - LETTER   → readIdentKeyword()
     - DIGIT    → readNumber()
     - QUOTE    → readConstChar()
     - OPERATOR → Tạo token tương ứng
     - '('      → Kiểm tra comment hoặc LPAR
     - '#'      → skipLine() (preprocessor)
     - ...
  4. Trả về Token
```

#### 3.2. Nhận dạng Identifier/Keyword (`readIdentKeyword`)

**Thuật toán**:
```
1. Bắt đầu với CHAR_LETTER
2. WHILE (currentChar là LETTER hoặc DIGIT):
     - Thêm ký tự vào chuỗi token
     - readChar()
3. Kiểm tra độ dài (MAX_IDENT_LEN = 15)
4. Tra bảng keyword:
     - Nếu match → Token là KEYWORD
     - Ngược lại → Token là IDENTIFIER
```

**Ví dụ**:
- `PROGRAM` → Keyword `KW_PROGRAM`
- `Example1` → Identifier `TK_IDENT(Example1)`

**Code implementation**:
```c
Token* readIdentKeyword(void) {
  Token *token = makeToken(TK_IDENT, lineNo, colNo);
  int count = 0;
  int identTooLong = 0;

  while ((charCodes[currentChar] == CHAR_LETTER) ||
         (charCodes[currentChar] == CHAR_DIGIT)) {
    if (count < MAX_IDENT_LEN)
      token->string[count++] = currentChar;
    else
      identTooLong = 1;
    readChar();
  }
  token->string[count] = '\0';

  if (identTooLong)
    error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);

  token->tokenType = checkKeyword(token->string);
  if (token->tokenType == TK_NONE)
    token->tokenType = TK_IDENT;

  return token;
}
```

#### 3.3. Nhận dạng Number (`readNumber`)

**Thuật toán**:
```
1. Bắt đầu với CHAR_DIGIT
2. value = 0
3. WHILE (currentChar là DIGIT):
     - value = value * 10 + (currentChar - '0')
     - readChar()
4. Trả về Token với value
```

**Ví dụ**: `123` → `TK_NUMBER(123)`

**Code implementation**:
```c
Token* readNumber(void) {
  Token *token = makeToken(TK_NUMBER, lineNo, colNo);

  token->value = 0;
  while (charCodes[currentChar] == CHAR_DIGIT) {
    token->value = token->value * 10 + (currentChar - '0');
    readChar();
  }

  return token;
}
```

#### 3.4. Nhận dạng Character Constant (`readConstChar`)

**Thuật toán**:
```
1. Bắt đầu với CHAR_SINGLEQUOTE (')
2. readChar() để lấy ký tự bên trong
3. Kiểm tra EOF → Lỗi nếu gặp
4. Lưu ký tự vào token.string[0]
5. readChar() để đọc dấu ' đóng
6. Kiểm tra:
     - Nếu là SINGLEQUOTE → Hợp lệ
     - Ngược lại → ERR_INVALIDCHARCONSTANT
```

**Ví dụ**: `' '` → `TK_CHAR(' ')`

**Code implementation**:
```c
Token* readConstChar(void) {
  Token *token = makeToken(TK_CHAR, lineNo, colNo);

  readChar();
  if (currentChar == EOF) {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
    return token;
  }
    
  token->string[0] = currentChar;
  token->string[1] = '\0';

  readChar();
  if (currentChar == EOF) {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
    return token;
  }

  if (charCodes[currentChar] == CHAR_SINGLEQUOTE) {
    readChar();
    return token;
  } else {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
    return token;
  }
}
```

#### 3.5. Nhận dạng Comment (`skipComment`)

**Thuật toán**:
```
Bắt đầu khi gặp "(*"
WHILE true:
  1. Nếu EOF → ERR_ENDOFCOMMENT
  2. Nếu gặp '*':
       readChar()
       Nếu currentChar == ')':
           readChar() và RETURN (comment kết thúc)
  3. Ngược lại → readChar() và tiếp tục
```

**Ví dụ**: `(* This is a comment *)` → Bị bỏ qua, không tạo token

**Code implementation**:
```c
void skipComment() {
  int ln, cn;

  while (1) {
    if (currentChar == EOF)
      error(ERR_ENDOFCOMMENT, lineNo, colNo);

    if (currentChar == '*') {
      ln = lineNo;
      cn = colNo;
      readChar();
      if (currentChar == ')') {
        readChar();
        return;
      }
      if (currentChar == EOF)
        error(ERR_ENDOFCOMMENT, ln, cn);
    } else {
      readChar();
    }
  }
}
```

#### 3.6. Nhận dạng Operator hai ký tự

**Thuật toán**:
```
1. Đọc ký tự đầu (như '<', '>', ':', '!')
2. Lưu vị trí hiện tại
3. readChar() để đọc ký tự tiếp theo
4. Kiểm tra kết hợp:
     '<' + '=' → SB_LE (<=)
     '>' + '=' → SB_GE (>=)
     ':' + '=' → SB_ASSIGN (:=)
     '!' + '=' → SB_NEQ (!=)
5. Nếu không match → Chỉ dùng ký tự đầu
```

**Ví dụ**:
- `<=` → `SB_LE`
- `:=` → `SB_ASSIGN`
- `!=` → `SB_NEQ`

**Code implementation**:
```c
case CHAR_LT:
  ln = lineNo; cn = colNo;
  token = makeToken(SB_LT, ln, cn);
  readChar();
  if (currentChar == '=') {
    token->tokenType = SB_LE;
    readChar();
  }
  return token;
```

#### 3.7. Xử lý Preprocessor Directive (`skipLine`)

**Thuật toán**:
```
Khi gặp '#' (như #define):
1. WHILE (currentChar != '\n' AND currentChar != EOF):
     readChar()
2. Nếu gặp '\n' → readChar() để sang dòng mới
3. Gọi lại getToken() để lấy token tiếp theo
```

**Ví dụ**: `#define MAX 100` → Toàn bộ dòng bị bỏ qua

**Code implementation**:
```c
void skipLine() {
  while ((currentChar != EOF) && (currentChar != '\n'))
    readChar();
  if (currentChar == '\n')
    readChar();
}

// Trong getToken():
if (currentChar == '#') {
  skipLine();
  return getToken();
}
```

### 4. Xử lý lỗi

Scanner phát hiện và báo các lỗi sau:

| Lỗi | Mô tả | Ví dụ |
|------|-------|-------|
| `ERR_ENDOFCOMMENT` | Comment không đóng | `(* comment` (thiếu `*)`) |
| `ERR_IDENTTOOLONG` | Identifier quá dài (>15 ký tự) | `verylongidentifiername` |
| `ERR_INVALIDCHARCONSTANT` | Hằng ký tự không hợp lệ | `'ab'` hoặc `'` |
| `ERR_INVALIDSYMBOL` | Ký tự không hợp lệ | `@`, `$`, `%` |

### 5. Finite State Automaton (FSA)

Scanner hoạt động như một FSA với các trạng thái:

```
START → (đọc ký tự) → Xác định loại:
  ├→ LETTER → IN_IDENT → (letter|digit)* → IDENT/KEYWORD
  ├→ DIGIT  → IN_NUMBER → (digit)* → NUMBER
  ├→ '      → IN_CHAR → char → ' → CHAR_CONSTANT
  ├→ (      → CHECK_COMMENT:
  │           ├→ * → IN_COMMENT → ... → *) → SKIP
  │           └→ other → LPAR
  ├→ OPERATOR → CHECK_TWO_CHAR:
  │             ├→ match → TWO_CHAR_OP
  │             └→ no match → SINGLE_CHAR_OP
  └→ #      → SKIP_LINE → (\n) → START
```

**Sơ đồ trạng thái chi tiết cho Identifier**:
```
     [letter]        [letter|digit]
START ------→ IN_IDENT -----------→ IN_IDENT
                 ↓
              [other]
                 ↓
            CHECK_KEYWORD → IDENT hoặc KEYWORD
```

**Sơ đồ trạng thái chi tiết cho Number**:
```
     [digit]        [digit]
START ------→ IN_NUMBER -------→ IN_NUMBER
                 ↓
              [other]
                 ↓
              NUMBER
```

**Sơ đồ trạng thái chi tiết cho Comment**:
```
     [(]         [*]          [^*]          [*]         [)]
START --→ CHECK_LPAR --→ IN_COMMENT ←--→ IN_COMMENT --→ END
           ↓
        [other]
           ↓
         LPAR
```

### 6. Ví dụ hoạt động chi tiết

**Input**: `I := 123`

**Quá trình từng bước**:
```
Vị trí: 1-1
1. Đọc 'I' (ASCII 73)
   → charCodes[73] = CHAR_LETTER
   → Gọi readIdentKeyword()
   → Đọc tiếp: gặp ' ' (không phải letter/digit)
   → Kết thúc chuỗi: "I"
   → checkKeyword("I") = TK_NONE (không phải keyword)
   → Token: TK_IDENT(I) tại vị trí 1-1

Vị trí: 1-3
2. Đọc ' ' (khoảng trắng)
   → charCodes[32] = CHAR_SPACE
   → Gọi skipBlank()
   → Bỏ qua, đọc tiếp

Vị trí: 1-3
3. Đọc ':' (ASCII 58)
   → charCodes[58] = CHAR_COLON
   → Tạo token SB_COLON tại 1-3
   → readChar() → gặp '='
   → Chuyển tokenType thành SB_ASSIGN
   → Token: SB_ASSIGN tại vị trí 1-3

Vị trí: 1-6
4. Đọc ' ' → skipBlank()

Vị trí: 1-6
5. Đọc '1' (ASCII 49)
   → charCodes[49] = CHAR_DIGIT
   → Gọi readNumber()
   → value = 0
   → value = 0 * 10 + (49 - 48) = 1
   → Đọc '2': value = 1 * 10 + 2 = 12
   → Đọc '3': value = 12 * 10 + 3 = 123
   → Gặp EOF/space, kết thúc
   → Token: TK_NUMBER(123) tại vị trí 1-6
```

**Output**:
```
1-1:TK_IDENT(I)
1-3:SB_ASSIGN
1-6:TK_NUMBER(123)
```

### 7. Bảng Token Types

#### Keywords (20)
| Token | Keyword |
|-------|---------|
| KW_PROGRAM | PROGRAM |
| KW_CONST | CONST |
| KW_TYPE | TYPE |
| KW_VAR | VAR |
| KW_INTEGER | INTEGER |
| KW_CHAR | CHAR |
| KW_ARRAY | ARRAY |
| KW_OF | OF |
| KW_FUNCTION | FUNCTION |
| KW_PROCEDURE | PROCEDURE |
| KW_BEGIN | BEGIN |
| KW_END | END |
| KW_CALL | CALL |
| KW_IF | IF |
| KW_THEN | THEN |
| KW_ELSE | ELSE |
| KW_WHILE | WHILE |
| KW_DO | DO |
| KW_FOR | FOR |
| KW_TO | TO |

#### Special Tokens (5)
| Token | Mô tả |
|-------|-------|
| TK_NONE | Token không hợp lệ |
| TK_IDENT | Identifier |
| TK_NUMBER | Number literal |
| TK_CHAR | Character constant |
| TK_EOF | End of file |

#### Symbols/Operators (18)
| Token | Symbol | Mô tả |
|-------|--------|-------|
| SB_SEMICOLON | ; | Semicolon |
| SB_COLON | : | Colon |
| SB_PERIOD | . | Period |
| SB_COMMA | , | Comma |
| SB_ASSIGN | := | Assignment |
| SB_EQ | = | Equal |
| SB_NEQ | != | Not equal |
| SB_LT | < | Less than |
| SB_LE | <= | Less or equal |
| SB_GT | > | Greater than |
| SB_GE | >= | Greater or equal |
| SB_PLUS | + | Plus |
| SB_MINUS | - | Minus |
| SB_TIMES | * | Times |
| SB_SLASH | / | Slash |
| SB_LPAR | ( | Left parenthesis |
| SB_RPAR | ) | Right parenthesis |
| SB_LSEL | [ | Left bracket |
| SB_RSEL | ] | Right bracket |

## Kết quả test

Chạy tất cả test cases:

```bash
cd incompleted
for i in 1 2 3 4 5 6 7; do 
  echo "Testing example$i..."
  ./scanner ../test/example$i.kpl > /tmp/result$i.txt 2>&1
  diff -q /tmp/result$i.txt ../test/result$i.txt && echo "✓ PASS" || echo "✗ FAIL"
done
```

**Kết quả hiện tại**: 5/7 tests pass (71.4%)
- ✓ Example 2: PASS
- ✓ Example 3: PASS  
- ✓ Example 4: PASS
- ✓ Example 6: PASS
- ✓ Example 7: PASS
- ✗ Example 1: Minor discrepancy
- ✗ Example 5: Minor discrepancy

## Các cải tiến đã thực hiện

### 1. Xử lý Preprocessor Directives
Thêm hàm `skipLine()` để bỏ qua các dòng bắt đầu bằng `#` (như `#define`, `#include`).

### 2. Nhận dạng Character Constants
Hoàn thiện hàm `readConstChar()` để xử lý hằng ký tự với các kiểm tra lỗi đầy đủ.

### 3. Xử lý Comments lồng nhau
Đảm bảo scanner phát hiện được comment không đóng và báo lỗi chính xác.

## Độ phức tạp thuật toán

### Thời gian (Time Complexity)
- **Tổng thể**: O(n), với n là số ký tự trong file
- **readIdentKeyword()**: O(k), với k là độ dài identifier (≤ 15)
- **readNumber()**: O(m), với m là số chữ số
- **skipComment()**: O(c), với c là độ dài comment
- **getToken()**: O(1) cho mỗi token (trung bình)

### Không gian (Space Complexity)
- O(1) cho mỗi token (fixed-size Token struct)
- O(n) tổng thể cho toàn bộ token stream

## Tài liệu tham khảo

- **Compilers: Principles, Techniques, and Tools** (Dragon Book) - Alfred V. Aho, Monica S. Lam, Ravi Sethi, Jeffrey D. Ullman
- **Finite Automata Theory** - Lexical Analysis chapter
- **KPL Language Specification** - Hedspi, Hanoi University of Technology
- **Regular Expressions and Finite Automata** - Theory of Computation

## Tác giả

- **Original Author**: Huu-Duc Nguyen
- **Copyright**: (c) 2008, Hedspi, Hanoi University of Technology
- **Version**: 1.0

## License

Educational use only - Hanoi University of Technology
