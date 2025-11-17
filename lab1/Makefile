CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = checkDoc
SRC = checkDoc.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) alice30.txt stopw.txt output.txt

clean:
	rm -f $(TARGET) output.txt

test: $(TARGET)
	./$(TARGET) vanban.txt stopw.txt output.txt
	@echo "\n--- Output preview ---"
	@head -20 output.txt

.PHONY: all run clean test
