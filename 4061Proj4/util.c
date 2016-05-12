#ifndef _REENTRANT
#define _REENTRANT
#endif


#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

//EXTRA CREDIT WAS COMPLETED
static int master_fd = -1;
pthread_mutex_t accept_con_mutex = PTHREAD_MUTEX_INITIALIZER;
int alive_flag = 0;

// this function takes a hostname and returns the IP address
int lookup_host (const char *host)
{
  struct addrinfo hints, *res;
  int errcode;
  char addrstr[100];
  void *ptr;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  errcode = getaddrinfo (host, NULL, &hints, &res);
  if (errcode != 0)
    {
      perror ("getaddrinfo");
      return -1;
    }

  printf ("Host: %s\n", host);
  while (res)
    {
      inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

      switch (res->ai_family)
        {
        case AF_INET:
          ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
          break;
        }
      inet_ntop (res->ai_family, ptr, addrstr, 100);
      printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
              addrstr, res->ai_canonname);
      res = res->ai_next;
    }

  return 0;
}

int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }
   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
     but exactly one time, in the main thread of your program)
     BEFORE USING ANY OF THE FUNCTIONS BELOW
   - if init encounters any errors, it will call exit().
************************************************/
enum boolean {FALSE, TRUE};
void init(int port) {

  	master_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
 	addr.sin_family = AF_INET;
 	addr.sin_addr.s_addr = INADDR_ANY;
  	addr.sin_port = htons(port);
	
  int enable = 1;
  setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));
  bind(master_fd, (struct sockaddr*) &addr, sizeof(addr));

  listen(master_fd, 100);
}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the thread calling
     accept_connection must exit by calling pthread_exit().
***********************************************/
int accept_connection(void) {
  	struct sockaddr_in cli_addr;
  	socklen_t addr_len = sizeof(cli_addr);

  	int new_fd = accept(master_fd, (struct sockaddr*) &cli_addr, &addr_len);

	if(new_fd < 0)
    	printf("Error accepting connection...\n");

  	return new_fd;
}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/
void alarm_handler( int sig) {
	
}
int get_request(int fd, char *filename) {
	struct sigaction act;
	act.sa_sigaction = (void *)alarm_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction (SIGALRM, &act, NULL);
	//Read the HTTP request from the fd into the filename buffer temporarily
	if(alive_flag == 1) {
		alarm(5);
	}
	int request = read(fd, filename, 1024);
	if(request == 0){
		close(fd);
		return -1;
	}
	if(request < 0) {
		printf("Error getting request...\n");
		close(fd);
		return -1;
	}
	//Search the request to determine whether connection should be kept alive
	
	char * conn_search = strchr(filename, '\n');
	conn_search = strchr(conn_search+1, '\n');
	conn_search = strchr(conn_search+1, '\n');
	conn_search = strchr(conn_search+1, '\n');	
	if ( conn_search[13] == 'K' )
	{
		alive_flag = 1;
	}	
	
	
	char * new_filename = strchr(filename, '/');	
	int i = 0;
	while ( new_filename[i] != ' ') {
		filename[i] = new_filename[i];
		i++;
	}	
	
	filename[i] = '\0';
	return alive_flag;
}



/**********************************************
 * return_result
   - returns the contents of a file to the requesting client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes) {
	char *message = "HTTP/1.1 200 OK\n";
	int success;	
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error: %d\n", errno);
		return -1;
	}
	message = "Content-Type: ";
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error2: %d\n", errno);
		return -1;
	}
	
	if(success = write(fd, content_type, strlen(content_type))< 0){
		printf("write error... terminating...\n");
		printf("Error3: %d\n", errno);
		return -1;
	}
	message = "\nContent-Length: ";
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error4: %d\n", errno);
		return -1;
	}
	char str[15];
	sprintf(str, "%d", numbytes);
	if(success = write(fd, str, 15)< 0){
		printf("write error... terminating...\n");
		printf("Error5: %d\n", errno);
		return -1;
	}
	message = "\nConnection: Close\n\n";
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error6: %d\n", errno);
		return -1;
	}
	if(success = write(fd, buf, numbytes)< 0){
		printf("write error... terminating...\n");
		printf("Error7: %d\n", errno);
		return -1;
	}
	
	if ( alive_flag <= 0 )
	{
		close(fd);
	}
  	
  	alive_flag = 0;
  
  	return 0;
  
}


/**********************************************
 * return_error
   - returns an error message in response to a bad request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf) {
	printf("return_error\n");
	char *message = "HTTP/1.1 404 NOT Found\n";
	int success;	
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error: %d\n", errno);
		return -1;
	}
	message = "Content-Type: text/html";
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error2: %d\n", errno);
		return -1;
	}
	
	
	message = "\nContent-Length: ";
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error3: %d\n", errno);
		return -1;
	}
	
	size_t numbytes = strlen(buf);
	char str[15];
	sprintf(str, "%d", numbytes);
	if(success = write(fd, str, 15)< 0){
		printf("write error... terminating...\n");
		printf("Error4: %s\n", strerror(errno));
		return -1;
	}
	message = "\nConnection: Close\n\n";
	if(success = write(fd, message, strlen(message))< 0){
		printf("write error... terminating...\n");
		printf("Error5: %d\n", errno);
		return -1;
	}
	
	if(success = write(fd, buf, strlen(buf))< 0){
		printf("write error... terminating...\n");
		printf("Error6: %d\n", errno);
		return -1;
	}
	
	if ( alive_flag <= 0 )
	{
		close(fd);
	}
  
  	alive_flag = 0;
  	
  	return 0;
}

