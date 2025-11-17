#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define MAX_STOP_WORDS 1000
#define HASH_TABLE_SIZE 10007

typedef struct HashNode {
    char word[MAX_WORD_LEN];
    int count;
    int *lines;
    int line_count;
    int line_capacity;
    struct HashNode *next;
} HashNode;

char stopWords[MAX_STOP_WORDS][MAX_WORD_LEN];
int stopWordCount = 0;
HashNode *hashTable[HASH_TABLE_SIZE];
int totalWords = 0;

void readStopWords(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Không thể mở file %s\n", filename);
        return;
    }
    
    char word[MAX_WORD_LEN];
    while (fscanf(file, "%s", word) == 1 && stopWordCount < MAX_STOP_WORDS) {
        for (int i = 0; word[i]; i++) {
            word[i] = tolower(word[i]);
        }
        strcpy(stopWords[stopWordCount++], word);
    }
    
    fclose(file);
}

int isStopWord(const char *word) {
    for (int i = 0; i < stopWordCount; i++) {
        if (strcmp(word, stopWords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int isProperNoun(const char *originalWord, int afterPunctuation) {
    if (strlen(originalWord) == 0) return 0;
    
    if (isupper(originalWord[0]) && !afterPunctuation) {
        return 1;
    }
    return 0;
}

unsigned int hashFunction(const char *word) {
    unsigned int hash = 5381;
    int c;
    while ((c = *word++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

void addLineToNode(HashNode *node, int lineNumber) {
    for (int i = 0; i < node->line_count; i++) {
        if (node->lines[i] == lineNumber) {
            return;
        }
    }
    
    if (node->line_count >= node->line_capacity) {
        node->line_capacity = (node->line_capacity == 0) ? 10 : node->line_capacity * 2;
        node->lines = realloc(node->lines, node->line_capacity * sizeof(int));
    }
    
    node->lines[node->line_count++] = lineNumber;
}

void findOrInsertWord(const char *word, int lineNumber) {
    unsigned int hashIndex = hashFunction(word);
    
    HashNode *current = hashTable[hashIndex];
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            current->count++;
            addLineToNode(current, lineNumber);
            return;
        }
        current = current->next;
    }
    
    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    strcpy(newNode->word, word);
    newNode->count = 1;
    newNode->lines = NULL;
    newNode->line_count = 0;
    newNode->line_capacity = 0;
    addLineToNode(newNode, lineNumber);
    
    newNode->next = hashTable[hashIndex];
    hashTable[hashIndex] = newNode;
    
    totalWords++;
}

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
    int afterPunctuation = 1;
    
    while ((ch = fgetc(file)) != EOF) {
        if (isalpha(ch)) {
            if (wordIndex < MAX_WORD_LEN - 1) {
                originalWord[wordIndex] = ch;
                lowercaseWord[wordIndex] = tolower(ch);
                wordIndex++;
            }
        } else {
            if (wordIndex > 0) {
                originalWord[wordIndex] = '\0';
                lowercaseWord[wordIndex] = '\0';
                
                if (!isStopWord(lowercaseWord) && !isProperNoun(originalWord, afterPunctuation)) {
                    findOrInsertWord(lowercaseWord, lineNumber);
                }
                
                afterPunctuation = 0;
                wordIndex = 0;
            }
            
            if (ch == '.' || ch == '?' || ch == '!') {
                afterPunctuation = 1;
            } else if (ch == '\n') {
                lineNumber++;
            }
        }
    }
    
    if (wordIndex > 0) {
        originalWord[wordIndex] = '\0';
        lowercaseWord[wordIndex] = '\0';
        
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
    
    HashNode **allWords = (HashNode **)malloc(totalWords * sizeof(HashNode *));
    int index = 0;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *current = hashTable[i];
        while (current != NULL) {
            allWords[index++] = current;
            current = current->next;
        }
    }
    
    qsort(allWords, totalWords, sizeof(HashNode *), compareWords);
    
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
        printf("Usage: %s <input_file> [stopwords_file] [output_file]\n", argv[0]);
        return 1;
    }
    
    char *inputFile = argv[1];
    char *stopWordsFile = (argc >= 3) ? argv[2] : "completed/stopw.txt";
    char *outputFile = (argc >= 4) ? argv[3] : "output.txt";
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }
    
    readStopWords(stopWordsFile);
    processTextFile(inputFile);
    printIndexTable(outputFile);
    freeMemory();
    
    return 0;
}
