/* CSci4061 F2014 Assignment 2
* date: 10/28/14
* name: Jangyoon Kim, Fletcher Thomas, Matthew Ernst
* id:  kimx2873, thom4409, ernst183 */

#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <gtk/gtk.h>

#define MAX_TAB 100

/*
 * Name:		uri_entered_cb
 * Input arguments:'entry'-address bar where the url was entered
 *			 'data'-auxiliary data sent along with the event
 * Output arguments:void
 * Function:	When the user hits the enter after entering the url
 *			in the address bar, 'activate' event is generated
 *			for the Widget Entry, for which 'uri_entered_cb'
 *			callback is called. Controller-tab captures this event
 *			and sends the browsing request to the router(/parent)
 *			process.
 */
void uri_entered_cb(GtkWidget* entry, gpointer data)
{
	if(data == NULL)
	{	
		return;
	}

	browser_window* b_window = (browser_window*)data;
	comm_channel channel = b_window->channel;
	
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);

	if(tab_index < 0)
	{
		printf("error\n");
                return;
	}

	// Get the URL.
	char *uri = get_entered_uri(entry);

	// Now you get the URI from the controller.
	// Build a new request to send to the router, which will in turn send it to the specified tab
  	child_req_to_parent new_req;
  	new_req.type = NEW_URI_ENTERED;
  	new_req.req.uri_req.render_in_tab = tab_index;
  	int i = 0;
  	while ( uri[i] != '\0' && i < 512 )
  	{
  		new_req.req.uri_req.uri[i] = uri[i];
  		i++;
  	}
  	write(channel.child_to_parent_fd[1], &new_req, sizeof(child_req_to_parent));
  	
	// HINT: there is a callback in wrapper.c that should give
    // you can idea of what the code should look like
    
    return;
}

/*
 * Name:		new_tab_created_cb
 * Input arguments:	'button' - whose click generated this callback
 *			'data' - auxillary data passed along for handling
 *			this event.
 * Output arguments:    void
 * Function:		This is the callback function for the 'create_new_tab'
 *			event which is generated when the user clicks the '+'
 *			button in the controller-tab. The controller-tab
 *			redirects the request to the parent (/router) process
 *			which then creates a new child process for creating
 *			and managing this new tab.
 */ 
void new_tab_created_cb(GtkButton *button, gpointer data)
{
	if(data == NULL)
	{
		return;
	}

 	int tab_index = ((browser_window*)data)->tab_index;
 	
	
	//This channel have pipes to communicate with router. 
	comm_channel channel = ((browser_window*)data)->channel;

	// Create a new request of type CREATE_TAB
	child_req_to_parent new_req;

	// Users press + button on the control window. 
	// Create new request to send to router, which will create a new child process/tab
  	new_req.type = CREATE_TAB;
  	new_req.req.new_tab_req.tab_index = tab_index;
  	write(channel.child_to_parent_fd[1], &new_req, sizeof(child_req_to_parent));  	
  	
  	return;
}

/*
 * Name:                run_control
 * Input arguments:     'comm_channel': Includes pipes to communctaion with Router process
 * Output arguments:    void
 * Function:            This function will make a CONTROLLER window and be blocked until the program terminate.
 */
int run_control(comm_channel comm)
{
	browser_window * b_window = NULL;

	//Create controler process
	create_browser(CONTROLLER_TAB, 0, G_CALLBACK(new_tab_created_cb), G_CALLBACK(uri_entered_cb), &b_window, comm);

	//go into infinite loop.
	
	show_browser();
	return 0;
}

/*
* Name:                 run_url_browser
* Input arguments:      'nTabIndex': URL-RENDERING tab index
                        'comm_channel': Includes pipes to communctaion with Router process
* Output arguments:     void
* Function:             This function will make a URL-RENDRERING tab Note.
*                       You need to use below functions to handle tab event. 
*                       1. process_all_gtk_events();
*                       2. process_single_gtk_event();
*                       3. render_web_page_in_tab(uri, b_window);
*                       For more details please Appendix B.
*/
int run_url_browser(int nTabIndex, comm_channel comm)
{
	browser_window * b_window = NULL;
	
	//Create controller window
	create_browser(URL_RENDERING_TAB, nTabIndex, G_CALLBACK(new_tab_created_cb), G_CALLBACK(uri_entered_cb), &b_window, comm);

	
	while (1) 
	{		
		child_req_to_parent request;
		process_single_gtk_event();
		
		//Need to communicate with Router process here.
	    	//Handle each type of message (few mentioned below)
		//  NEW_URI_ENTERED: render_web_page_in_tab(uri, b_window);
		//  TAB_KILLED: process all gtk events();
		
		read(comm.parent_to_child_fd[0], &request, sizeof(child_req_to_parent));  
		// Ignore CREATE_TAB request
		if(request.type == CREATE_TAB)
		{
			continue;
		}
		// Otherwise, if NEW_URI_ENTERED, render the URL in the tab
		else if(request.type == NEW_URI_ENTERED)
		{
			render_web_page_in_tab(request.req.uri_req.uri, b_window);
		}
		// Otherwise, if TAB_KILLED, process gtk events and kill the tab
		else if(request.type == TAB_KILLED)
		{
			process_all_gtk_events();
			exit(0);
		}
		// Otherwise, it is an invalid command and we should continue (-1 is default empty request)
		else if(request.type != -1)
		{
			continue;
		}
		usleep(1000);
		// Reset request type to default
		request.type = -1;
	}

	return 0;
}

int main()
{
	int i, j, comm_counter;
	comm_counter = 0;
	int r_c[2]; //pipe array for router to controller
	int c_r[2]; //pipe array for controller to router
	comm_channel comm[MAX_TAB];
	//This is Router process
	//Make a controller and URL-RENDERING tab when user request it. 
	if(pipe(r_c) == -1){printf("Failed to create r_c\n");}
	if(pipe(c_r) == -1){printf("Failed to create c_r\n");}
	if(fcntl(r_c[0], F_SETFL, O_NONBLOCK) == -1){printf("Failed to set nonblocking");}
	if(fcntl(c_r[0], F_SETFL, O_NONBLOCK) == -1){printf("Failed to set nonblocking");}
	comm_channel rcChannel;
	rcChannel.parent_to_child_fd[0] = r_c[0];
	rcChannel.child_to_parent_fd[0] = c_r[0];
	rcChannel.parent_to_child_fd[1] = r_c[1];
	rcChannel.child_to_parent_fd[1] = c_r[1];
	comm[comm_counter] = rcChannel;
	comm_counter++;
	pid_t pid;
	pid = fork();
	if(pid == 0) //child
	{
		close(comm[comm_counter - 1].parent_to_child_fd[1]);
		close(comm[comm_counter - 1].child_to_parent_fd[0]);
		run_control(comm[comm_counter - 1]);
	}
	else
	{
		close(comm[comm_counter - 1].parent_to_child_fd[0]);
		close(comm[comm_counter - 1].child_to_parent_fd[1]);
		
		while(1) //check if still has children, if not, terminate
		{	
					
			child_req_to_parent request;
			request.type = -1;
			// If j surpasses the comm_counter, reset j
			if ( j >= comm_counter ) 
			{
				j = 0;
			}
			// If the controller is dead, kill all of the tabs (and thus the program)
			if(comm[0].parent_to_child_fd[0] == -1 ) 
			{
				for(i = 0; i < comm_counter; i++)
				{
					if(comm[i].parent_to_child_fd[0] == -1)
					{
						continue;
					}
					child_req_to_parent request2;
					tab_killed_req kill_tab_req;
					kill_tab_req.tab_index = i;
					request2.type = TAB_KILLED;
					request2.req.killed_req = kill_tab_req;
					write(comm[i].parent_to_child_fd[1], &request2, sizeof(child_req_to_parent));
					close(comm[i].parent_to_child_fd[1]);
					close(comm[i].child_to_parent_fd[0]);
					comm[i].parent_to_child_fd[0] = -1;
				}
				return 0;
			}
			// If this tab has been killed, increment j and contiue to the next one
			if ( comm[j].parent_to_child_fd[0] == -1 )	{ j++;  continue; }
			read(comm[j].child_to_parent_fd[0], &request, sizeof(child_req_to_parent)); //read from current 
			
			
			j++;
			// If request is CREATE_TAB, set up a new pipe, add it to the comm array, and create the tab
			if(request.type == CREATE_TAB)
			{
				int r_t[2]; //pipe array for router to controller
				int t_r[2]; //pipe array for controller to router
				if(pipe(r_t) == -1){printf("Failed to create r_t\n");}
				if(pipe(t_r) == -1){printf("Failed to create t_r\n");}
				if(fcntl(r_t[0], F_SETFL, O_NONBLOCK) == -1){printf("Failed to set nonblocking");}
				if(fcntl(t_r[0], F_SETFL, O_NONBLOCK) == -1){printf("Failed to set nonblocking");}
				comm_channel rtChannel;
				rtChannel.parent_to_child_fd[0] = r_t[0];
				rtChannel.child_to_parent_fd[0] = t_r[0];
				rtChannel.parent_to_child_fd[1] = r_t[1];
				rtChannel.child_to_parent_fd[1] = t_r[1];
				comm[comm_counter] = rtChannel;
				comm_counter++;
				pid_t tab_pid;
				tab_pid = fork();
				if(tab_pid == 0)
				{
					close(comm[comm_counter - 1].parent_to_child_fd[1]);
					close(comm[comm_counter - 1].child_to_parent_fd[0]);
					run_url_browser(comm_counter-1, comm[comm_counter-1]);
				}
				else
				{
					close(comm[comm_counter - 1].parent_to_child_fd[0]);
					close(comm[comm_counter - 1].child_to_parent_fd[1]);
				}				
			}
			// If the request is NEW_URI_ENTERED, send the specified tab a request to render the specified URL
			else if(request.type == NEW_URI_ENTERED)
			{
				write(comm[request.req.uri_req.render_in_tab].parent_to_child_fd[1], &request, sizeof(child_req_to_parent));
			}
			// If the request is TAB_KILLED, send the specified tab a TAB_KILLED request
			else if(request.type == TAB_KILLED)
			{
				write(comm[request.req.killed_req.tab_index].parent_to_child_fd[1], &request, sizeof(child_req_to_parent));
				close(comm[request.req.killed_req.tab_index].parent_to_child_fd[1]);
				close(comm[request.req.killed_req.tab_index].child_to_parent_fd[0]);
				
				comm[request.req.killed_req.tab_index].parent_to_child_fd[0] = -1;
				
			}
			usleep(300);
		}
	}
	//With pipes, this process should communicate with controller and tabs.

	//General layout of main:
	//create pipe for communication with controller
	//Fork controller
	//poll for requests from child on one to many pipes
	//Use non-blocking read call to read data, identify the type of message and act accordingly
	//  CREATE_TAB:
	//	Create two pipes for bi-directional communication
	//	Fork URL_RENDERING process
	//  NEW_URI_ENTERED:
	//	Write this message on the pipe connecting to ROUTER and URL_RENDERING process.
	//  TAB_KILLED:
	//	Close file descriptors of corresponding tab's pipes.
	//When all child processes exit including controller, exit a success!
	//For more accurate details see section 4.1 in writeup.

	return 0;
}
