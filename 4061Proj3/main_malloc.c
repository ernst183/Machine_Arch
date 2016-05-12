#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "mm.h"

void main(){
	struct timeval time_s, time_e;

	printf("Timer for malloc\n");

	gettimeofday(&time_s, NULL);

	int i;
	void *mem;
	for(i = 0; i < 1000000; i++){
		mem = malloc((size_t) 64);
		free(mem);
	}

	gettimeofday(&time_e, NULL);

	fprintf(stderr, "Time taken = %f msec\n", comp_time(time_s, time_e) /1000.0);

}
