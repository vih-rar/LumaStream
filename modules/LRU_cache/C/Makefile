CC = gcc
CFLAGS = ""
TARGET = lru_cache.exe
SRCS = main.c lru_cache.c
OBJS = main.obj lru_cache.obj

all: 
	$(CC) $(SRCS) -o $(TARGET)

# $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CC) $(OBJS) /Fe:$(TARGET)

# %.obj: %.c
# 	$(CC) $(CFLAGS) /c $<

clean:
	del /Q *.obj $(TARGET) 2>nul

run: $(TARGET)
	.\$(TARGET)

.PHONY: all clean run
