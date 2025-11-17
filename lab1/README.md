# Chương Trình Tạo Bảng Chỉ Mục (Index Table) cho Văn Bản Tiếng Anh

## Mô Tả

Chương trình đọc một tệp văn bản tiếng Anh và tạo bảng chỉ mục (index) liệt kê tất cả các từ có nghĩa xuất hiện trong văn bản, kèm theo:
- Số lần xuất hiện của từ
- Danh sách các dòng mà từ đó xuất hiện

## Tính Năng

**Hash Table** với độ phức tạp O(1) cho tìm kiếm  
Loại bỏ **stop words** (các từ không có ý nghĩa tra cứu)  
Loại bỏ **danh từ riêng** (từ viết hoa không đứng sau dấu câu)  
**Không phân biệt** chữ hoa/chữ thường  
Sắp xếp theo **thứ tự từ điển**  
Hỗ trợ **truyền tham số** linh hoạt  

## Cấu Trúc Dữ Liệu

### Hash Table với Chaining
- Kích thước: 10007 (số nguyên tố để giảm collision)
- Thuật toán hash: djb2
- Xử lý collision: Chaining (linked list)
- Độ phức tạp: O(1) trung bình cho insert/search

## Biên Dịch

```bash
gcc checkDoc.c -o checkDoc
```

## Cách Sử Dụng

### Cú pháp

```bash
./checkDoc <input_file> [stopwords_file] [output_file]
```

### Các tham số

- `<input_file>`: File văn bản cần xử lý (bắt buộc)
- `[stopwords_file]`: File chứa stop words (mặc định: `completed/stopw.txt`)
- `[output_file]`: File xuất kết quả (mặc định: `output.txt`)

### Ví dụ

```bash
# Cách 1: Chỉ truyền file input (dùng giá trị mặc định cho stopwords và output)
./checkDoc completed/alice30.txt

# Cách 2: Truyền file input và stopwords
./checkDoc completed/vanban.txt completed/stopw.txt

# Cách 3: Truyền đầy đủ 3 tham số
./checkDoc completed/alice30.txt completed/stopw.txt result.txt
```

## Định Dạng Input

### File văn bản (input_file)
- Mã ASCII
- Văn bản tiếng Anh
- Ví dụ: `vanban.txt`, `alice30.txt`

### File stop words (stopwords_file)
- Mỗi từ một dòng
- Ví dụ:
```
a
an
and
the
of
```

## Định Dạng Output

Mỗi dòng trong file kết quả có định dạng:
```
<từ>       <số_lần_xuất_hiện>, <dòng_1>, <dòng_2>, ...
```

### Ví dụ output:
```
about     94, 45, 49, 63, 75, 132, 138, 159, 188, 258
above     3, 901, 1844, 3177
absence   1, 2490
accident  2, 3314, 3333
across    5, 34, 36, 2070, 2304, 2697
```

**Giải thích:**
- `about`: Từ cần tra cứu
- `94`: Xuất hiện 94 lần trong văn bản
- `45, 49, 63, ...`: Các dòng mà từ "about" xuất hiện

## Quy Tắc Xử Lý

### 1. Định nghĩa "từ"
- Dãy ký tự chữ cái (a-z, A-Z)
- Bị ngăn cách bởi: khoảng trắng, dấu câu, số, ký tự đặc biệt

### 2. Chuyển đổi chữ hoa/thường
- Tất cả từ được chuyển về **chữ thường** trước khi lưu
- Ví dụ: "Hello" và "hello" được coi là cùng một từ

### 3. Loại bỏ Stop Words
Các từ không có ý nghĩa tra cứu sẽ bị loại bỏ, ví dụ:
- a, an, and, the, of, in, to, ...

### 4. Loại bỏ Danh Từ Riêng
Từ có chữ cái đầu viết HOA nhưng **KHÔNG** đứng sau dấu câu (. ? !)

**Ví dụ:**
```
Will you visit Hanoi someday?
```
- `Will`: Đứng đầu câu (sau dấu câu ảo) → **Không loại bỏ**
- `Hanoi`: Viết hoa, không sau dấu câu → **Loại bỏ**

## Cấu Trúc Thư Mục

```
Bai1/
├── checkDoc.c          # Mã nguồn chương trình
├── checkDoc            # File thực thi (sau khi biên dịch)
├── README.md           # File hướng dẫn này
├── output.txt          # File kết quả (được tạo sau khi chạy)
└── completed/
    ├── alice30.txt     # File văn bản mẫu (Alice in Wonderland - 30KB)
    ├── vanban.txt      # File văn bản mẫu nhỏ
    └── stopw.txt       # Danh sách stop words
```

## Thuật Toán & Độ Phức Tạp

### Tìm kiếm & Chèn từ
- **O(1)** trung bình (Hash Table với chaining)

### Sắp xếp kết quả
- **O(n log n)** (Quick Sort - qsort)

### Tổng thể
- **O(m + n log n)** với:
  - m: số từ trong văn bản
  - n: số từ duy nhất

## Ví Dụ Minh Họa

### Input (vanban.txt):
```
At least 30 people were killed and 110 wounded Friday afternoon 
in a suicide bombing in Yusifiya, a city about 25 kilometers 
southwest of Baghdad.
```

### Output (output.txt):
```
about     2, 2, 9
afternoon 1, 1
among     1, 6
bombing   2, 1, 4
city      1, 2
friday    1, 1
killed    3, 1, 3, 7
least     1, 1
people    2, 1, 3
suicide   1, 1
wounded   3, 1, 3, 8
...
```

## Xử Lý Lỗi

Chương trình sẽ thông báo nếu:
- Không truyền đủ tham số
- Không thể mở file input
- Không thể mở file stopwords
- Không thể tạo file output

## Giới Hạn

- Độ dài từ tối đa: 100 ký tự
- Số lượng stop words tối đa: 1000 từ
- Kích thước hash table: 10007 buckets


