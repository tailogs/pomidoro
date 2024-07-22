CC = gcc
CFLAGS = -Wall -Wextra
OBJ = main.o resource.o
OUT = pomodoro.exe

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $@ $^ -lgdi32

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

resource.o: resource.rc
	windres $< -o $@

clean:
	del $(OBJ) $(OUT)
