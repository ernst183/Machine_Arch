#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "mm.h"

void main(){
	struct timeval time_s, time_e;

	printf("Timer for mm.c\n");

	mm_t manager;
	mm_init(&manager, 1000000, 64);

	gettimeofday(&time_s, NULL);

	int i;
	for(i = 0; i < 1000000; i++){
		mm_get(&manager);
		mm_put(&manager, (void *)1234);
	}
	printf("\n");
	gettimeofday(&time_e, NULL);

	fprintf(stderr, "Time taken = %f msec\n",comp_time(time_s, time_e) / 1000.0);

}
