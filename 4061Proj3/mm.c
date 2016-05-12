#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "mm.h"

/* Return usec */
double comp_time(struct timeval time_s, struct timeval time_e) {

  double elap = 0.0;

  if (time_e.tv_sec > time_s.tv_sec) {
    elap += (time_e.tv_sec - time_s.tv_sec - 1) * 1000000.0;
    elap += time_e.tv_usec + (1000000 - time_s.tv_usec);
  }
  else {
    elap = time_e.tv_usec - time_s.tv_usec;
  }
  return elap;
}

/* Return 0 for success, or -1 and set errno on fail. */
int mm_init(mm_t *mm, int hm, int sz) {
 	int i, mem_size;
 	mem_size = hm * sz;
	void *chunks_init = malloc((size_t)mem_size); //allocate the memory for the chunks array
	if(chunks_init == NULL || mm == NULL){errno = EPERM; return -1;}
	mm->chunk_sz = sz; //set attributes for mm
	mm->num_chunks_used = 0;
	mm->num_chunks = hm;
	mm->chunks = chunks_init;
	mm_node *nodeArray = (mm_node*)malloc(hm * sizeof(mm_node));
	for(i = 0; i < hm; i++){
		mm_node newNode;
		newNode.data = mm->chunks + (i * sz);
		newNode.used = 0;
		nodeArray[i] = newNode;	
	}
	mm->nodes = nodeArray;
	
  	return 0;  
}

//return the first free chunk of memory
void *mm_get(mm_t *mm) {
	
	if(mm == NULL){errno = EPERM; return NULL;}
	void *memory_pointer; 
	int i;
	for ( i=0; i<mm->num_chunks; i++ )
	{
		if ( mm->nodes[i].used == 0 )
		{
			mm->nodes[i].used = 1;
			memory_pointer = mm->nodes[i].data;
			return memory_pointer; 
		}	
	}	
	errno = EPERM;
    return;  
}

//free the specified memory for use by others
void mm_put(mm_t *mm, void *chunk) {
	if(mm == NULL){errno = EPERM; return;}
	
	int i;
	for ( i=0; i<mm->num_chunks; i++ )
	{
		if ( mm->nodes[i].data == chunk )
		{
			mm->nodes[i].used = 0;
			return;
		}	
	}	
	
	errno = EPERM;
	return;
}

//free the memory manager
void mm_release(mm_t *mm) {
	free(mm->chunks);
	free(mm->nodes);
	return;
}

/*
 * This is just an example of how to use the timer.  Notice that
 * this function is not included in mm_public.h, and it is defined as static,
 * so you cannot call it from other files.  Instead, just follow this model
 * and implement your own timing code where you need it.
 */
static void timer_example() {
  struct timeval time_s, time_e;

  
  gettimeofday (&time_s, NULL);

  

  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);
}
