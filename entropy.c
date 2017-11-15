#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// compute the entropy of a given p array of node attribute
static double entropy(double *p, uint64_t n)
{
	double sum = 0;
	for (uint64_t i=0; i<n; i++) {
		sum += p[i];
	}

	double entropy = 0;

	for (uint64_t i = 0; i < n; i++) {
		if (p[i] == 0.0) continue;
		entropy -= (p[i]/sum * log2l(p[i]/sum));
	}

	return entropy;
}

int main(void) 
{
	uint64_t n_instances = 0;
	uint64_t n_nodes = 0;
	uint64_t n_node_atributes = 0;

	fprintf(stdout, "numbers of instances = ");
	scanf("%lu", &n_instances);
	fprintf(stdout, "number of nodes = ");
	scanf("%lu", &n_nodes);
	fprintf(stdout, "number of attribute for every node = ");
	scanf("%lu", &n_node_atributes);
	
	double container[n_nodes][n_node_atributes];
	fprintf(stdout, "\nadd every attribute in the node\n");
	for (size_t i =0; i<n_nodes; i++) {
		for(size_t j=0; j<n_node_atributes; j++) {
			printf("Node %lu , attribute %lu = ", i,j);
			scanf("%lf", &container[i][j]);
		}
	}

	double entropy_nodes[n_nodes];
	// compute for every node the entropy
	for(size_t i=0; i<n_nodes; i++) {
		entropy_nodes[i] = entropy(container[i], n_node_atributes);
		printf("Entropy of this node %lu is H[x] = %F\n", i, entropy_nodes[i]);
	}

	double sum = 0, ig = 0;
	//compute the information gain from these nodes
	for(size_t i=1; i<n_nodes; i++) {
		for (size_t j=0; j<n_node_atributes; j++) {
			sum += container[i][j];
		}
		ig = ig - (sum/n_instances) * entropy_nodes[i];
		sum = 0;
	}

	ig = entropy_nodes[0] + ig;

	printf("Information gain for this entry IG(x) = %F\n", ig);
}
