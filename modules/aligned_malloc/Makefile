CC = cl
CFLAGS = /W3
TARGET = aligned_malloc.exe
SRCS = main.c aligned_malloc.c
OBJS = main.obj aligned_malloc.obj

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) /Fe:$(TARGET)

%.obj: %.c
	$(CC) $(CFLAGS) /c $<

clean:
	del /Q *.obj $(TARGET) 2>nul

run: $(TARGET)
	.\$(TARGET)

.PHONY: all clean run
