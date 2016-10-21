all:
	gcc -std=gnu11 -g -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align -Wmissing-prototypes -Wmissing-declarations -Winline -lm -Wuninitialized -Wstrict-prototypes -Werror -Wno-variadic-macros entropy.c -o entropy

run:
	./entropy file.txt
