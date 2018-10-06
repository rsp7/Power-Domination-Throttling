#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Generate all possible configurations of initially colored vertices for a graph of the input 'order'.
 *
 * Returns a (2 ^ 'order') by ('order' + 1) array, where the first 'order' values in each inner array (representing
 * a configuration) are 1 if the respective vertex is initially colored and 0 if not. The last value is the number
 * of colored vertices in the configuration. The memory for the array and its inner arrays must be freed later.
 */
int ** gen_sets(int order) {
	int num = (int)pow(2, order);
	int **sets = malloc(num * sizeof(int *));
	for (int i = 0; i < num; i++) {
		sets[i] = malloc((order + 1) * sizeof(int));
		sets[i][order] = 0;
		for (int j = 0; j < order; j++) {
			sets[i][j] = (i >> j) & 1;	// get jth bit of i
			sets[i][order] += sets[i][j];
		}
	}
	return sets;
}

/*
 * Compute the power propagation time of the input 'graph' (of order 'order') using the input 'set' of initially colored vertices.
 *
 * 'graph' is in adjacency matrix format. The first 'order' values in 'set' are 1 if the respective vertex is initially colored
 * and 0 if not. The last value is the number of initially colored vertices.
 * Returns 'order' if the set of vertices is not a power dominating set.
 */
int ppt(int order, int graph[order][order], int *set) {
	if (set[order] == order) {	// if all vertices initially colored
		return 0;
	}

	/* Domination step */
	int colored[order];
	for (int i = 0; i < order; i++) {
		colored[i] = set[i];
	}
	for (int i = 0; i < order; i++) {
		if (set[i]) {	// if INITIALLY colored
			for (int j = 0; j < order; j++) {
				if (graph[i][j]) {
					colored[j] = 1;
				}
			}
		}
	}
	int ret = 1;

	/* Forcing steps */
	int forces[order];	// keeps track of force targets for a forcing step
	int forced;	// whether any forces are available
	int target;	// target vertex for a force; -1 if no target
	while (1) {
		for (int i = 0; i < order; i++) {
			forces[i] = 0;
		}
		forced = 0;
		for (int i = 0; i < order; i++) {
			if (colored[i]) {
				target = -1;
				for (int j = 0; j < order; j++) {
					if (graph[i][j] && !colored[j]) {
						if (target == -1) {
							target = j;
						} else {	// else i adjacent to multiple uncolored vertices
							target = -1;
							break;
						}
					}
				}
				if (target != -1) {
					forces[target] = 1;
					forced = 1;
				}
			}
		}
		if (!forced) {
			break;
		}
		for (int i = 0; i < order; i++) {	// apply forces
			if (forces[i]) {
				colored[i] = 1;
			}
		}
		ret++;
	}

	/* Check if PDS */
	for (int i = 0; i < order; i++) {
		if (!colored[i]) {
			return order;
		}
	}
	return ret;
}

/*
 * Compute the power throttling number of the input 'graph' (of order 'order'), where 'sets' enumerates all possible configurations
 * of initially colored vertices.
 *
 * 'graph' is in adjacency matrix format. The first 'order' values in each inner array of 'sets' are 1 if the respective vertex is
 * initially colored and 0 if not. The last value is the number of colored vertices in the configuration.
 */
int pth(int order, int graph[order][order], int **sets) {
	int ret = order;
	int throttling;		// the throttling value of a set of vertices
	for (int i = 0; i < (int)pow(2, order); i++) {
		throttling = sets[i][order] + ppt(order, graph, sets[i]);
		if (throttling < ret) {
			ret = throttling;
		}
	}
	return ret;
}

int main(void) {
	char order_string[10];		// stores the current graph order as a string
	char input_file_name[50];
	char output_file_name[50];
	int fc;		// reads characters from the input file
	int index;	// the index of the current graph among all graphs of the same order
	for (int order = 1; order < 10; order++) {
		sprintf(order_string, "%d", order);
		strcpy(input_file_name, "order");
		strcat(input_file_name, order_string);
		strcat(input_file_name, "AM.txt");
		FILE *ifp = fopen(input_file_name, "r");
		strcpy(output_file_name, "order");
		strcat(output_file_name, order_string);
		strcat(output_file_name, "PTH.txt");
		FILE *ofp = fopen(output_file_name, "w");
		fprintf(ofp, "Power throttling numbers for order %d\n\n", order);

		int graph[order][order];
		int **sets = gen_sets(order);
		index = 1;
		while ((fc = fgetc(ifp)) != EOF) {
			if (fc == '.') {
				fc = fgetc(ifp);	// read newline character
				for (int i = 0; i < order; i++) {
					for (int j = 0; j < order; j++) {
						fc = fgetc(ifp);
						graph[i][j] = fc - '0';
					}
					fc = fgetc(ifp);	// read newline character
				}
				fprintf(ofp, "Graph %d: %d\n", index, pth(order, graph, sets));
				index++;
			}
		}
		printf("Generated power throttling numbers for all graphs on %d vertices in %s\n", order, output_file_name);
		fclose(ofp);
		fclose(ifp);
		for (int i = 0; i < (int)pow(2, order); i++) {
			free(sets[i]);
		}
		free(sets);
	}
}
