CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
LDFLAGS = -lm

TARGET  = graphics_editor
SRC     = graphics_editor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
