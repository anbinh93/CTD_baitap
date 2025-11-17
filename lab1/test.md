#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define MAX_STOP_WORDS 1000
#define HASH_TABLE_SIZE 10007  // Số nguyên tố để giảm collision

// Cấu trúc node trong hash table (dùng chaining để xử lý collision)
typedef struct HashNode {
    char word[MAX_WORD_LEN];      // Từ (dạng chữ thường)
    int count;                     // Số lần xuất hiện
    int *lines;                    // Mảng động các số dòng
    int line_count;                // Số phần tử trong mảng lines
    int line_capacity;             // Dung lượng mảng lines
    struct HashNode *next;         // Con trỏ đến node tiếp theo (chaining)
} HashNode;

// Biến toàn cục
char stopWords[MAX_STOP_WORDS][MAX_WORD_LEN];
int stopWordCount = 0;
HashNode *hashTable[HASH_TABLE_SIZE];  // Hash table với chaining
int totalWords = 0;                     // Tổng số từ duy nhất

/*
 * Hàm: readStopWords
 * Đọc danh sách stop words từ file stopw.txt
 */
void readStopWords(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Không thể mở file %s\n", filename);
        return;
    }
    
    char word[MAX_WORD_LEN];
    while (fscanf(file, "%s", word) == 1 && stopWordCount < MAX_STOP_WORDS) {
        // Chuyển về chữ thường
        for (int i = 0; word[i]; i++) {
            word[i] = tolower(word[i]);
        }
        strcpy(stopWords[stopWordCount++], word);
    }
    
    fclose(file);
}

/*
 * Hàm: isStopWord
 * Kiểm tra xem từ có phải stop word không
 */
int isStopWord(const char *word) {
    for (int i = 0; i < stopWordCount; i++) {
        if (strcmp(word, stopWords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * Hàm: isProperNoun
 * Kiểm tra danh từ riêng: chữ cái đầu hoa, không đứng sau dấu câu
 * originalWord: từ gốc chưa chuyển chữ thường
 * afterPunctuation: 1 nếu từ này đứng ngay sau dấu câu (. ? !)
 */
int isProperNoun(const char *originalWord, int afterPunctuation) {
    if (strlen(originalWord) == 0) return 0;
    
    // Nếu chữ cái đầu là chữ hoa VÀ không đứng sau dấu câu
    if (isupper(originalWord[0]) && !afterPunctuation) {
        return 1;
    }
    return 0;
}

/*
 * Hàm: hashFunction
 * Tính hash value cho một từ (dùng thuật toán djb2)
 */
unsigned int hashFunction(const char *word) {
    unsigned int hash = 5381;
    int c;
    while ((c = *word++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % HASH_TABLE_SIZE;
}

/*
 * Hàm: addLineToNode
 * Thêm số dòng vào danh sách dòng của một HashNode
 * Đảm bảo mỗi số dòng chỉ xuất hiện MỘT LẦN (không trùng lặp)
 */
void addLineToNode(HashNode *node, int lineNumber) {
    // Kiểm tra xem số dòng đã tồn tại chưa
    for (int i = 0; i < node->line_count; i++) {
        if (node->lines[i] == lineNumber) {
            return;  // Dòng đã tồn tại, không thêm nữa
        }
    }
    
    // Mở rộng mảng nếu cần
    if (node->line_count >= node->line_capacity) {
        node->line_capacity = (node->line_capacity == 0) ? 10 : node->line_capacity * 2;
        node->lines = realloc(node->lines, node->line_capacity * sizeof(int));
    }
    
    // Thêm số dòng mới
    node->lines[node->line_count++] = lineNumber;
}

/*
 * Hàm: findOrInsertWord
 * Tìm từ trong hash table, nếu không có thì chèn vào
 * Sử dụng chaining để xử lý collision - O(1) trung bình
 */
void findOrInsertWord(const char *word, int lineNumber) {
    unsigned int hashIndex = hashFunction(word);
    
    // Tìm trong chain tại vị trí hashIndex
    HashNode *current = hashTable[hashIndex];
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            // Từ đã tồn tại, cập nhật
            current->count++;
            addLineToNode(current, lineNumber);
            return;
        }
        current = current->next;
    }
    
    // Từ chưa có, tạo node mới
    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    strcpy(newNode->word, word);
    newNode->count = 1;
    newNode->lines = NULL;
    newNode->line_count = 0;
    newNode->line_capacity = 0;
    addLineToNode(newNode, lineNumber);
    
    // Chèn vào đầu chain (insertion at head)
    newNode->next = hashTable[hashIndex];
    hashTable[hashIndex] = newNode;
    
    totalWords++;
}

/*
 * Hàm: processTextFile
 * Đọc file văn bản và xây dựng bảng chỉ mục
 */
void processTextFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Không thể mở file %s\n", filename);
        return;
    }
    
    int lineNumber = 1;
    int ch;
    char originalWord[MAX_WORD_LEN];
    char lowercaseWord[MAX_WORD_LEN];
    int wordIndex = 0;
    int afterPunctuation = 1;  // Bắt đầu coi như sau dấu câu (đầu văn bản)
    int lastCharWasPunctuation = 0;
    
    while ((ch = fgetc(file)) != EOF) {
        if (isalpha(ch)) {
            // Ghép ký tự vào từ
            if (wordIndex < MAX_WORD_LEN - 1) {
                originalWord[wordIndex] = ch;
                lowercaseWord[wordIndex] = tolower(ch);
                wordIndex++;
            }
        } else {
            // Gặp ký tự không phài chữ cái -> kết thúc từ
            if (wordIndex > 0) {
                originalWord[wordIndex] = '\0';
                lowercaseWord[wordIndex] = '\0';
                
                // Kiểm tra điều kiện: không phải stop word, không phải danh từ riêng
                if (!isStopWord(lowercaseWord) && !isProperNoun(originalWord, afterPunctuation)) {
                    findOrInsertWord(lowercaseWord, lineNumber);
                }
                
                wordIndex = 0;
            }
            
            // Kiểm tra dấu câu
            if (ch == '.' || ch == '?' || ch == '!') {
                lastCharWasPunctuation = 1;
            } else if (ch == '\n') {
                lineNumber++;
                // Sau xuống dòng, nếu trước đó có dấu câu thì từ tiếp theo là sau dấu câu
                // Nếu không có dấu câu thì không phải sau dấu câu
                if (!lastCharWasPunctuation) {
                    afterPunctuation = 0;
                }
            } else if (isspace(ch)) {
                // Nếu gặp khoảng trắng sau dấu câu, từ tiếp theo là sau dấu câu
                if (lastCharWasPunctuation) {
                    afterPunctuation = 1;
                    lastCharWasPunctuation = 0;
                }
            } else {
                // Ký tự khác (số, dấu phẩy, ...) không phải dấu câu
                lastCharWasPunctuation = 0;
                afterPunctuation = 0;
            }
        }
    }
    
    // Xử lý từ cuối cùng nếu file không kết thúc bằng ký tự không phải chữ cái
    if (wordIndex > 0) {
        originalWord[wordIndex] = '\0';
        lowercaseWord[wordIndex] = '\0';
        
        // Kiểm tra điều kiện
        if (!isStopWord(lowercaseWord) && !isProperNoun(originalWord, afterPunctuation)) {
            findOrInsertWord(lowercaseWord, lineNumber);
        }
    }
    
    fclose(file);
}


int compareWords(const void *a, const void *b) {
    HashNode *nodeA = *(HashNode **)a;
    HashNode *nodeB = *(HashNode **)b;
    return strcmp(nodeA->word, nodeB->word);
}


void printIndexTable(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        return;
    }
    
    // Thu thập tất cả các node từ hash table vào mảng
    HashNode **allWords = (HashNode **)malloc(totalWords * sizeof(HashNode *));
    int index = 0;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *current = hashTable[i];
        while (current != NULL) {
            allWords[index++] = current;
            current = current->next;
        }
    }
    
    // Sắp xếp theo thứ tự từ điển
    qsort(allWords, totalWords, sizeof(HashNode *), compareWords);
    
    // In ra file
    for (int i = 0; i < totalWords; i++) {
        fprintf(file, "%-10s", allWords[i]->word);
        fprintf(file, "%d", allWords[i]->count);
        
        for (int j = 0; j < allWords[i]->line_count; j++) {
            fprintf(file, ", %d", allWords[i]->lines[j]);
        }
        
        fprintf(file, "\n");
    }
    
    fclose(file);
    free(allWords);
}

/*
 * Hàm: freeMemory
 * Giải phóng bộ nhớ của hash table
 */
void freeMemory() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *current = hashTable[i];
        while (current != NULL) {
            HashNode *temp = current;
            current = current->next;
            if (temp->lines) {
                free(temp->lines);
            }
            free(temp);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Cách sử dụng: %s <input_file> [stopwords_file] [output_file]\n", argv[0]);
        printf("Ví dụ: %s alice30.txt\n", argv[0]);
        printf("       %s alice30.txt stopw.txt output.txt\n", argv[0]);
        return 1;
    }
    
    char *inputFile = argv[1];
    char *stopWordsFile = (argc >= 3) ? argv[2] : "completed/stopw.txt";
    char *outputFile = (argc >= 4) ? argv[3] : "output.txt";
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }
    
    // Bước 1: Đọc stop words
    printf("Đọc stop words từ: %s\n", stopWordsFile);
    readStopWords(stopWordsFile);
    
    // Bước 2: Xử lý file văn bản
    printf("Xử lý file văn bản: %s\n", inputFile);
    processTextFile(inputFile);
    
    // Bước 3: In kết quả
    printf("Ghi kết quả ra: %s\n", outputFile);
    printIndexTable(outputFile);
        
    // Giải phóng bộ nhớ
    freeMemory();
    
    printf("\nHoàn tất!\n");
    return 0;
}
