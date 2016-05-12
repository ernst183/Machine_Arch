/* CSci 4061 F2014 Assignment 2
 * Login: ernst183
 * Date: 10/28/14
 * Name: Jangyoon Kim, Fletcher Thomas, Matthew Ernst
 * ID: kimx2873, thom4409, ernst183
 */

The purpose of this program is to provide users with a multi-process web browser.

How to compile the program && use it correctly from shell:
	1. On the terminal, type make. This will produce executable file.
	2. Run the program: ./browser
	3. Controller window will open. Type URL on the URL box and index on the tab box. This will open the website on the according tab.
	4. To close all the tabs, close the controller window to do so.

What exactly is this program doing?
So, you are curious what's under the hood? Well, here is a short description of actions that our program does.
	1. When the program starts, two processes are produced: the router and the controller. Both of them are connected and can communicate with each other via pipes.
	2. As the user types in URL and tab number into the controller tab, the controller sends commands to the router, which creates URL-Rendering child processes for each tab that has been created.
	3. These childs (or tabs) will call URL-rendering and display the website requested by the user.
	4. When the controller gets killed, the tabs also gets killed and the program exits.

Any explicit assumptions you have made
	We are assuming that the user will enter a valid url in the form of http://www.websitename.com
	We are also assuming that GTK will handle all user input and graphics for us (calling our functions when needed)
	
Your strategies for error handling:
	Our group divided errors into two groups and handled it: user error and system error. 
	We've decided that user errors are predictable and insignificant to cause serious problems.
	Thus, our code ignores user error and waits for the user to give the correct command.
	On the other hand, system errors are unpredictable and significant. 
	One of these errors will cause unpleasant experience to the user.
	For those kinds of errors, we would catch them and stop our program immediately.
	
