C      = gcc
CFLAGS  = -O2 -Wall -std=c11
TARGET  = wordCount

$(TARGET): wordCount.c uthash.h
	$(CC) $(CFLAGS) -o $@ wordCount.c

clean:
	rm -f $(TARGET)