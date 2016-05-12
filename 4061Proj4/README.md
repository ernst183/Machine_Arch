/* CSci 4061 F2014 Assignment 5
 * Login: ernst183
 * Date: 12/10/14
 * Name: Jangyoon Kim, Fletcher Thomas, Matthew Ernst
 * ID: kimx2873, thom4409, ernst183
 */
 Provided server.c was modified
 EXTRA CREDIT WAS COMPLETED
 
 How to compile the program && use it correctly from shell:
	1. In the terminal, type 'make' in the correct directory. This will produce a bunch of executable files.
	2. First, run the server executable: ./web_server_http <port> <path_to_testing>/testing <num_dispatch> <num_worker> 			<queue_len>
	3. Now, open a different terminal and run: wget -i <path-to-urls>/urls -O results 
	4. Observe the output on each terminal window.

What exactly is this program doing?
So, you are curious what's under the hood? Well, here is a short description of actions that our program does.
	1. When the server starts, initializes a master socket at port 9000.
	2. The dispatch thread calls accept_connection, which initializes a new socket for the file transfer.  Then, using that socket, the dispatch thread calls get_request and waits for a request to be sent to the new socket.  Then, it passes that request off to the worker thread.
	3. The worker thread calls return_result or return_error for the given request, downloading the file into the specified place.  If the connection wasn't specified to be kept alive, it will close the socket.
	4. If the connection is marked "keep-alive", the dispatch thread will skip using accept_connection and get another request from the same socket/connection.
	
