CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS =
ifeq ($(OS),Windows_NT)
	LIBS += -lws2_32
endif

servidor: servidor.c arvore.c arvore.h avl.c avl.h
	$(CC) $(CFLAGS) -o $@ servidor.c arvore.c avl.c $(LIBS)

run: servidor
	./servidor

clean:
	rm -f servidor servidor.exe *.o
