#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "entropy.h"

static inline void ex(const char *msg, bool t)
{
	(t) ? fprintf(stderr, KRED "[!] " RESET "%s:%s\n", msg, strerror(errno))
		: fprintf(stderr, KRED "[!] " RESET "%s\n", msg);

	exit(EXIT_SUCCESS);
}

#define EX(message) ex((message), true)
#define WA(message) ex((message), false)

static bool filter_number(const char *arg)
{
	if (!arg) {
		return false;
	}

	const char *p = arg;
	for (; *p != '\0'; p++) {
		if (*p == 46) continue;
		// if it's not in this interval
		if (!(*p >= 0x30 && (*p <= 0x39)))
			return false;
	}

	return true;
}

static double convert_p(char *a)
{
	char n[BUFF_MAX + BUFF_MAX];
	if (!a)
		EX("Can't convert probability: NULL ptr");
	if (!filter_number(a)) {
		snprintf(n, BUFF_MAX + BUFF_MAX, "Can't convert this %s to double", a);
		WA(n);
	}

	double p = strtod(a, NULL);
	if (errno == ERANGE)
		EX("Failed to convert into double because of overflow");
	if (p < 0)
		WA("Probability val could not be less than 0");

	return p;
}

static void parse_file(const char *_name, double **_cnt, uint64_t *_n)
{
	if (!_name)
		WA("Can't open file: name file invalid");
	if (!_n)
		WA("Can't assign n values to NULL ptr");

	double *h = NULL;
	FILE *fp  = fopen(_name, "r");

	if (!fp)
		EX("Failed to open the file");

	uint64_t n = 0;
	char buff[BUFF_MAX];
	memset(buff, 0, BUFF_MAX);
	while ((fgets(buff, BUFF_MAX, fp) != NULL) && (!ferror(fp))) {
		n++;
	}

	if (n == 0)
		WA("File empty, can't compute the entropy");

	*_n = n;

	if (fseek(fp, 1L, SEEK_SET) != 0)
		EX("Failed to move the cursor at the begining");

	h = malloc(sizeof(double) * *_n);
	if (!h)
		EX("Failed to alloc a new container");

	memset(h, 0, sizeof(*h));

	uint64_t i = 0;
	while (fgets(buff, BUFF_MAX, fp) != NULL) {
		buff[strlen(buff) - 1] = '\0';					  // just throw away that \n nonsens
		h[i]				   = convert_p(buff);
		i++;
	}
	fclose(fp);
	*_cnt = h;
}

static double entropy(double *p, uint64_t n)
{
	double entropy = 0;
	for (uint64_t i = 0; i < n; i++) {
		if (p[i] == 0.0) continue;
		entropy -= (p[i] * log2l(p[i]));
	}
	return entropy;
}

static clock_t begin, end;
static double time_spent;

int main(int argc, char **argv)
{
	double *p_val = NULL;
	uint64_t n = 0;
	double e = 0.0;

	if (argc < 2) {
		fprintf(stdout, KRED "[*] " RESET " To few argumnets\n"
							 "Usage: ./entropy probability.txt\n");
		exit(EXIT_FAILURE);
	}

	parse_file(argv[1], &p_val, &n);

	begin = clock();
	e	 = entropy(p_val, n);
	end   = clock();

	free(p_val);

	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("time: %F\n", time_spent);
	if (e <= 0)
		WA("Entropy is equal less than 0");

	printf("entropy %F bits of information\n", e);
}
