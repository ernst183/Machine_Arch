
#include "packet.h"



int msqid = -1;

static message_t message;   /* current message structure */
static mm_t mm;   		    /* memory manager will allocate memory for packets */
static int pkt_cnt = 0;     /* how many packets have arrived for current message */
static int pkt_total = 1;   /* how many packets to be received for the message */

/*
   Handles the incoming packet. 
   Store the packet in a chunk from memory manager.
   The packets for given message will come out of order. 
   Hence you need to take care to store and assemble it correctly.
   Example, message "aaabbb" can come as bbb -> aaa, hence, you need to assemble it
   as aaabbb.
   Hint: "which" field in the packet will be useful.
 */
static void packet_handler(int sig) {
  
  packet_t pkt;
  
 
  mm_t mm2 = mm;
  
  void * chunk = mm_get(&mm);
      
  // get the "packet_queue_msg" from the queue.
  packet_queue_msg pkq;
  msgrcv(msqid, &pkq, sizeof(packet_t), QUEUE_MSG_TYPE, 0);
  
  
  // extract the packet from "packet_queue_msg" and store it in the memory from memory manager
  pkt = pkq.pkt;
  memcpy(chunk, &pkt, MSGSIZE);
  message.data[pkt_cnt] = chunk;
  message.num_packets++;
  pkt_total = pkt.how_many;
  pkt_cnt++;
}

/*
 * Create message from packets ... deallocate packets.
 * Return a pointer to the message on success, or NULL
 */
static char *assemble_message() {

  char *msg;
  int i;
  int msg_len = message.num_packets * sizeof(data_t);

  /* Allocate msg and assemble packets into it */
  msg = (char*)malloc(pkt_total * PACKET_SIZE);
  
  for (i=0; i<pkt_total; i++)
  {
  	packet_t *pkt = message.data[i];
  	memcpy(msg + (pkt->which * PACKET_SIZE), pkt->data, PACKET_SIZE); 
  	mm_put(&mm, pkt);
  }
  	char null = '\0';
  	memcpy(msg + pkt_total * PACKET_SIZE, &null, 1);

  /* reset these for next message */
  pkt_total = 1;
  pkt_cnt = 0;
  message.num_packets = 0;

  return msg;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: packet_sender <num of messages to receive>\n");
    exit(-1);
  }

  int k = atoi(argv[1]); /* num of messages you will get from the sender */
  int i;
  char *msg;
  mm_t mm2;
  mm = mm2;
  
  mm_init(&mm, 10, MSGSIZE);
  message.num_packets = 0;

  pid_queue_msg pqm;
  pqm.pid = getpid();
  pqm.mtype = QUEUE_MSG_TYPE;
 
  msqid = msgget(key, 0666|IPC_CREAT);
  printf("mid: %d\n", msqid);
  msgsnd(msqid, &pqm, sizeof(int), 0);
  printf("Sent pid\n");
  printf("pid = %d\n", pqm.pid);
  
  /* set up SIGIO handler to readhow many: 6
GOT IT: message=aaabbbcccdddeeeff incoming packets from the queue. Check packet_handler() */
  struct sigaction act;
  act.sa_sigaction = (void *)packet_handler;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  sigaction(SIGIO, &act, NULL);
  for (i = 1; i <= k; i++) {
  	packet_t pkt;
    while (pkt_cnt < pkt_total) {
   		pause(); /* block until next packet */
    }
  
    msg = assemble_message();
    if (msg == NULL) {
      	perror("Failed to assemble message");
    }
    else {
      	fprintf(stderr, "GOT IT: message=%s\n", msg);
      	free(msg);
    }
  }

  // deallocate memory manager
  mm_release(&mm);
	
  // remove the queue once done
  msgctl(msqid, IPC_RMID, NULL);
  
  return EXIT_SUCCESS;
}
