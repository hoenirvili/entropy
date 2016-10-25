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

	exit(EXIT_FAILURE);
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
		buff[strlen(buff) - 1] = '\0';
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

static void verify_probability(double *p, uint64_t n)
{
	double sum = 0;
	for (uint64_t i = 0; i < n; i++) {
		sum += p[i];
	}

	if (sum != 1.0)
		EX("The sum of all probabilities is not 1");
}

static clock_t begin, end;
static double time_spent;

static void init_pma(uint64_t n, uint64_t m, double p[n][m])
{
	for (uint64_t i = 0; i < n; i++) {
		for (uint64_t j = 0; j < m; j++) {
			printf("m[%lu][%lu] = ", i, j);
			scanf("%lf", &p[i][j]);
		}
	}
}

static void check_ma(uint64_t n, uint64_t m, double p[n][m])
{
	double sum = 0;

	for (uint64_t i = 0; i < n; i++)
		for (uint64_t j = 0; j < m; j++)
			sum += p[i][j];

	if (sum != 1)
		EX("The sum of ma probabilities is not 1");
}

static void compute_marginal_distributions_p(uint64_t n, uint64_t m, double marginal_x[n], double marginal_y[m], double p[n][m])
{
	uint64_t i = 0;
	uint64_t j = 0;

	// compute all P(X=x) where little x is all values of X
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++)
			marginal_x[i] += p[i][j];
	}

	// compute all P(Y=y) where lilttle y is all values	of Y
	for (j = 0; j < m; j++) {
		for (i = 0; i < n; i++)
			marginal_y[j] += p[i][j];
	}
}

static void verify_marginal_distribution(uint64_t n, uint64_t m, double marginal_x[n], double marginal_y[m])
{
	double sum = 0;
	for (uint64_t i = 0; i<n; i++) {
		sum += marginal_x[i];
	}

	if (sum != 1)
		EX("Pmf of marginal distribution of X must be 1");

	sum = 0;

	for (uint64_t i = 0; i<m; i++) {
		sum += marginal_y[i];
	}

	if (sum != 1)
		EX("Pmf of marginal distribution of Y must be 1");
}

static void compute_conditional_distributions_p(uint64_t n, uint64_t m, 
		double conditional_x[n], double conditional_y[m], 
		double marginal_x[n], double marginal_y[m], 
		double p[n][m]) {
	
	uint64_t i, j;


	for (j=0; j<m; j++) {
		for(i=0; i<n; i++) {
			conditional_x[i] = p[i][j] / marginal_x[j];
		}
	}

	for (i=0; i<n; i++) {
		for(j=0; j<n; j++) {
			conditional_y[i] = p[i][j] / marginal_y[i];
		}
	}
}

int main(int argc, char **argv)
{
	uint64_t n = 0, m = 0;
	double e = 0.0;

	if (argc < 2) {
		fprintf(stdout, KRED "[*] " RESET " To few argumnets\n"
							 "Usage: ./entropy -in probability.txt\n");
		exit(EXIT_FAILURE);
	}

	// check if we feed with a file
	// and compute just the entropy
	if (strcmp(argv[1], "-in") == 0) {
		double *p_val = NULL;
		parse_file(argv[2], &p_val, &n);
		verify_probability(p_val, n);

		begin = clock();
		e	 = entropy(p_val, n);
		end   = clock();

		free(p_val);

		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("time: %F\n", time_spent);
		if (e <= 0)
			WA("H(x) is equal less than 0");

		printf("H(x) %F bits of information\n", e);
	}

	// manually chose the insert the matrix from stdin
	if (strcmp(argv[1], "-ma") == 0) {
		// input from keyboard
		fprintf(stdout, "N = ");
		scanf("%lu", &n);
		fprintf(stdout, "M = ");
		scanf("%lu", &m);
		
		begin = clock(); // start recording time

		// read all p's form keybaord and check them
		double pma_val[n][m];
		init_pma(n, m, pma_val);
		check_ma(n, m, pma_val);

		// compute the col, line max p
		double marginal_x[n];		// result of P(X=x) marginal distribution value
		double marginal_y[m];		// result of P(Y=y) marginal distribution values
		memset(marginal_x, 0, sizeof(marginal_x));
		memset(marginal_y, 0, sizeof(marginal_y));

		// compute marginal distributions
		compute_marginal_distributions_p(n, m, marginal_x, marginal_y, pma_val);
		verify_marginal_distribution(n, m, marginal_x, marginal_y); // verify if the marginal distribution is correct
		double x_entropy = entropy(marginal_x, n); // result of H(x)
		double y_entropy = entropy(marginal_y, m); // result of H(y)
		printf("H(X) =  %F\n", x_entropy);
		printf("H(Y) = %F\n", y_entropy);

		double conditional_x[n], conditional_y[m];
		compute_conditional_distributions_p(n,m, conditional_x, conditional_y, marginal_x, marginal_y, pma_val); // p(x|y) = f(x,y)/ f(y) and p(y|x) = f(x,y)/f(x)
		
		printf("f(Y|X=x) = ");
		for(uint64_t i=0; i<n; i++) 
			printf("%f ", conditional_x[i]);
		printf("\n");	
		printf("f(X|Y=y) = ");
		for (uint64_t i=0; i<n; i++) 
			printf("%f ",conditional_y[i]);
		printf("\n");

		end   = clock(); // end recording time
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("time: %F\n", time_spent);
	}
}
