CFLAGS  = -ggdb -I.
LDFLAGS = -lpthread

all: test-blob

blob.o:blob.c
	gcc -c $(CFLAGS) blob.c $(LDFLAGS)

test-blob: blob.o test-blob.c
	gcc $(CFLAGS) test-blob.c blob.o -o test-blob $(LDFLAGS)

clean:
	rm -f *.o
	rm -f test-blob
