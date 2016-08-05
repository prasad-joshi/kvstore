CFLAGS  = -ggdb -I.
LDFLAGS = -lpthread

all: test-blob test-dict

blob.o:blob.c
	gcc -c $(CFLAGS) blob.c $(LDFLAGS)

dict.o: dict.c
	gcc -c $(CFLAGS) dict.c $(LDFLAGS)

murmur3.o: murmur3.c
	gcc -c $(CFLAGS) murmur3.c $(LDFLAGS)

test-blob: blob.o test-blob.c
	gcc $(CFLAGS) test-blob.c blob.o -o test-blob $(LDFLAGS)

test-dict: dict.o murmur3.o test-dict.c
	gcc $(CFLAGS) test-dict.c blob.o dict.o murmur3.o -o test-dict $(LDFLAGS)

clean:
	rm -f *.o
	rm -f test-blob test-dict
