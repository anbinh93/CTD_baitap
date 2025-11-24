# Experiment in Compiler Construction – Scanner Design

Tài liệu này mô tả lại bối cảnh bài tập và **chứa sẵn một prompt** để bạn sử dụng với một AI khác (hoặc chính ChatGPT) nhằm sinh code hoàn chỉnh cho file `scanner.c`.

---

## 1. Bối cảnh & cấu trúc project

Project “Experiment in Compiler Construction – Scanner design” gồm các file chính:

- `scanner.c`: file chính cần hoàn thiện.
- `reader.h`, `reader.c`: cung cấp hàm đọc ký tự nguồn:
  - `int readChar(void);`
  - `int openInputStream(char *fileName);`
  - `void closeInputStream(void);`
  - Biến toàn cục: `int lineNo, colNo; int currentChar;`
- `charcode.h`, `charcode.c`: phân loại ký tự, định nghĩa:
  ```c
  typedef enum {
    CHAR_SPACE,
    CHAR_LETTER,
    CHAR_DIGIT,
    CHAR_PLUS,
    CHAR_MINUS,
    CHAR_TIMES,
    CHAR_SLASH,
    CHAR_LT,
    CHAR_GT,
    CHAR_EXCLAIMATION,
    CHAR_EQ,
    CHAR_COMMA,
    CHAR_PERIOD,
    CHAR_COLON,
    CHAR_SEMICOLON,
    CHAR_SINGLEQUOTE,
    CHAR_LPAR,
    CHAR_RPAR,
    CHAR_UNKNOWN
  } CharCode;

  extern CharCode charCodes[256];
