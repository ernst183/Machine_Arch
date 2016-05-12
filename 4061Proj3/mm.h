#ifndef __MM_H
#define __MM_H

#include <sys/time.h>

#define INTERVAL 0
#define CHUNK_SIZE 64
#define NUM_CHUNKS 1000000

typedef struct mm_node{
	void * data;//data held in this node
	int used;
} mm_node;


typedef struct {
	int chunk_sz; //size of chunks in mm
	int num_chunks;
	int num_chunks_used; //number of chunks currently being used in mm
	void * chunks; //chunks of memory
	mm_node *nodes;//array of nodes storing memory 
} mm_t;


double comp_time(struct timeval time_s, struct timeval time_e);
int mm_init(mm_t *mm, int num_chunks, int chunk_size);
void *mm_get(mm_t *mm);
void mm_put(mm_t *mm, void *chunk);
void mm_release(mm_t *mm);

#endif
