/* CSci 4061 F2014 Assignment 3
 * Login: ernst183
 * Date: 11/11/14
 * Name: Jangyoon Kim, Fletcher Thomas, Matthew Ernst
 * ID: kimx2873, thom4409, ernst183
 */

 How to compile the program && use it correctly from shell:
	1. In the terminal, type 'make' in the correct directory. This will produce a bunch of executable files.
	2. First, run the sender executable: ./packet_sender <any positive integer>
	3. Now, open a different terminal and run the receiver executable: ./packet_receiver <same number>
	4. Observe the output on each terminal window.

What exactly is this program doing?
So, you are curious what's under the hood? Well, here is a short description of actions that our program does.
	1. When the sender starts, it tries to find the recevier pid in the key specified message queue. If it's not there yet, it will wait until the receiver sends it.
	2. Once the sender-receiver connection is estabilished, the sender starts sending packets to receiver.
	3. Now, the receiver will receive data and store it in the memory manager (one we've created).
	   Message_Assemble stores the packets in the order of the data. Each packet is indexed by the field 'which.'
	4. After returning ordered data, receiver will free all memories and close.
	
	WARNING: main_mm takes a VERY long time to run.
