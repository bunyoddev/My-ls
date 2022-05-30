CC=gcc
CFLAGS=-I.
DEPS = my_ls.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

make:
	$(CC) my_ls.c -o my_ls
.PHONY: clear
clear:
	rm -f *.o my_ls 