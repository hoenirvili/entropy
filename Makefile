all:
	gcc -std=gnu11 -g -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align -Wmissing-prototypes -Wmissing-declarations -Winline -Wuninitialized -Wstrict-prototypes -Werror -Wno-variadic-macros entropy.c -o entropy -lm
run:
	./entropy file.txt
