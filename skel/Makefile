CC=gcc
CFLAGS=-Wall -Wextra

LOAD=load_balancer
SERVER=server
CACHE=lru_cache
UTILS=utils

# Add new source file names here:
# Nice try dar nu prea merge
# EXTRA=hash_map doubly_linked_list simply_linked_list queue
HASH=hash_map
DLL=doubly_linked_list
SLL=simply_linked_list
QUEUE=queue

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(CACHE).o $(UTILS).o $(HASH).o $(DLL).o $(SLL).o $(QUEUE).o # $(EXTRA).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(CACHE).o: $(CACHE).c $(CACHE).h
	$(CC) $(CFLAGS) $^ -c

$(UTILS).o: $(UTILS).c $(UTILS).h
	$(CC) $(CFLAGS) $^ -c

#$(EXTRA).o: $(EXTRA).c $(EXTRA).h
#	$(CC) $(CFLAGS) $^ -c

$(HASH).o: $(HASH).c $(HASH).h
	$(CC) $(CFLAGS) $^ -c

$(DLL).o: $(DLL).c $(DLL).h
	$(CC) $(CFLAGS) $^ -c

$(SLL).o: $(SLL).c $(SLL).h
	$(CC) $(CFLAGS) $^ -c

$(QUEUE).o: $(QUEUE).c $(QUEUE).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
