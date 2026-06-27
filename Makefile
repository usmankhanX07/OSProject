CFLAGS = -Wall -pthread

all: banker rag wait_die

banker:     main.c banker.c
	gcc $(CFLAGS) -o banker main.c banker.c


wait_die:   main.c wait_die.c
	gcc $(CFLAGS) -o wait_die main.c wait_die.c

rag:        main.c rag.c
	gcc $(CFLAGS) -o rag main.c rag.c

clean:
	rm -f banker wait_die wound_wait no_wait rag
