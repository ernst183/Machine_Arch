/* CSci 4061 F2014 Assignment 1
 * Login: ernst183
 * Date: 10/2/14
 * Name: Jangyoon Kim, Fletcher Thomas, Matthew Ernst
 * ID: kimx2873, thom4409, ernst183
 */

The purpose of this program is to build executable programs or libraries from source files given by the user.
However, to aid the user, we have several options.

How to compile the program && use it correctly from shell:
	1. Using given files (main.c, util.c, util.h, Makefile), create an executable.
	2. Run the program: ./make4061 (*options, see below)
	3. Verify the output files.

	* Note: the following list is the options you could run.
	* ./make4061 : This is default mode and only builds the first target within the "makefile" (Assuming there is a file named Makefile).
	* ./make4061 specificTarget : This will build only specificTarget within the makefile.
	* ./make4061 -f filename : This option will build the filename. If not given, it will use the default name ("makefile").
	* ./make4061 -n : Displays all the options available.
	* ./make4061 -B : Completely recompile every single target files.
	* ./make4061 -m log.txt : Compiles the files and the logs from make will be stored on file "log.txt".
	* All these options can be combined.

What exactly is this program doing?
So, you are curious what's under the hood? Well, here is a short description of actions that our program does.
	1. The program will recognize the user input and seperate the options from filename.
	2. Open and scan the makefile given by the user and parse what targets have to be built. 
	   The program will store targets and dependencies and order them accordingly.
	3. Execute commands according to targets and dependencies. 
	   However, the program will wait if the dependent files don't exist and have to be compiled (such as .o files).
	4. Before running the commands, it will check modification timestamps on the files in order to make sure it isn't unnessecarily
	   recompiling files
