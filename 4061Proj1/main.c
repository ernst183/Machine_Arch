#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"



target_t targets[10];	//array for holding parsed targets
int num_targets = 0;	//number of targets
int nflag = 0; //used for -n command line option
int Bflag = 0; //used for -B command line option
int mflag = 0; //used for -m command line option
int fd; //used to globally keep track of the log file
FILE *logFile;//holds the log file if output is being redirected (-m)

//This function will parse makefile input from user or default makeFile. 
int parse(char * lpszFileName)
{
	char szLine[1024];
	char * lpszLine;
	FILE * fp = file_open(lpszFileName);

	int pos;		
	int tflag;		//if 1, reading a target line
	int dflag;		//if 1, reading dependencies on target line
	int zflag;		//if 1, characters for dependency have been read
	int commentFlag;//if 1, the rest of the line is a comment

	if(fp == NULL)
	{
		return -1;
	}

	while(file_getline(szLine, fp) != NULL) 
	{
		int sizeDependencies = 0;
		commentFlag = 0;//initializing
		target_t target;		
		pos = 0; 
		tflag = 0;
		dflag = 0;
		zflag = 0;
		int i, counter;
		target.num_dependencies = 0;
		char* dependencies[10];
		target.command = "";
		target.name = "";


		// this loop will go through the given file, one line at a time
		// this is where you need to do the work of interpreting
		// each line of the file to be able to deal with it later

		//Remove newline character at end if there is one
		lpszLine = strtok(szLine, "\n"); 
		if(lpszLine == NULL) {continue;}
		if ( lpszLine[pos] != '\t' ) {tflag = 1;} //if 1, we're not looking at a command line
		else {pos++;}
		//You need to check below for parsing. 
		//Skip if blank or comment.
		//Remove leading whitespace.
		//Skip if whitespace-only.
		//Only single command is allowed.
		//If you found any syntax error, stop parsing. 
		//If lpszLine starts with '\t' it will be command else it will be target.
		//It is possbile that target may not have a command as you can see from the example on project write-up. (target:all)
		//You can use any data structure (array, linked list ...) as you want to build a graph


		if ( lpszLine[pos] != ' ' )
		{
			counter = 0;
			while ( lpszLine[pos] != '\0' )
			{
				if ( lpszLine[pos] == '#' ) {commentFlag = 1; break;}//if this line is a comment, go to the next line
				if ( tflag )
				{							
					if ( dflag )
					{
						while(lpszLine[pos] == ' ' && lpszLine[pos] != '\0')//eating whitespace until the end of the line
						{					
							pos++;
						}
						char dependency[128];

						counter = 0;
						zflag = 0;			//reset zflag to 0 until characters in a dependency are read
						while(lpszLine[pos] != ' ' && lpszLine[pos] != '\0')
						{
							zflag = 1;	
							dependency[counter] = lpszLine[pos];
							counter++;
							pos++;
						}

						if ( zflag )		//if dependency characters were actually read, add a new dependency
						{
							char *realDependency = (char*)malloc(counter*sizeof(char));//resizing dependency array
							for (i=0; i<counter; i++)
							{
								realDependency[i] = dependency[i];
							}							
							dependencies[target.num_dependencies] = realDependency;
							sizeDependencies += sizeof(realDependency);
							target.num_dependencies++;	//added a dependency to the target struct	
						}				
					}

					else
					{
						char targetName[128];
						counter = 0;
						while(lpszLine[pos] != ':' && lpszLine[pos] != ' ' && lpszLine[pos] != '\0' )
						//continue reading target name until a colon is found
						{
							targetName[counter] = lpszLine[pos];
							counter++;
							pos++;
						}
						char* realName = (char*)malloc(counter*sizeof(char));
						for (i=0; i<counter; i++)
						{
							realName[i] = targetName[i]; //resizing target name array
						}
						target.name = realName;
						dflag = 1;
						pos++;
					}
				}
				else
				{
					if ( num_targets < 1 )
					{
						printf("Error: parsing command with no targets");
						return -1;
					}
					char currentCommand[128];
					while(lpszLine[pos] != '\0' ) //read command until end of line
					{
						currentCommand[counter] = lpszLine[pos];
						counter++;
						pos++;
					}
					char *realCommand = (char*)malloc(counter*sizeof(char));
					for (i=0; i<counter; i++)
					{
						realCommand[i] = currentCommand[i];//resizing command array
					}
					targets[num_targets - 1].command = realCommand;
				}				
			}
			if( tflag && !commentFlag )
			{
				char** realDependencies = (char**)malloc(sizeDependencies);
				for (i=0; i<target.num_dependencies; i++) { realDependencies[i] = dependencies[i]; }//resizing dependencies array
				target.dependencies = realDependencies;				
				targets[num_targets] = target;//adding built target to global target array
				num_targets++;
			}
		}
	}

	//Close the makefile. 
	fclose(fp);

	return 0;
}

void show_error_message(char * lpszFileName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a maumfile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
	fprintf(stderr, "-m FILE\t\tRedirect the output to the file specified .\n");
	exit(0);
}

int stringEqual(char* s1, char* s2) //checks if two string are equivalent
{
	int i = 0;
	int s1_l = strlen(s1);
	int s2_l = strlen(s2);
	while ( i < s1_l && i < s2_l )
	{
		if ( s1[i] != s2[i] ) {return 0;}
		i++;
	}
	return 1;
}

int build(char* mainTargetName)
{
	if(mflag == 1)
	{
		dup2(fd, 1);//if the -m command was used, redirect the output from stdout to the log file
	}
	int i,j,
		foundTargetFlag = 0, //1 when the dependency was found in the target array
		t_num = -1; //array position of the main target
	int compFlag = 0; //used to compare the modification times of two files
	pid_t childpid;
	char *currentDependency, *command;
	char delim[] = " ";
	char** splitCommand;

	for (i = 0; i<num_targets; i++)
	{
		if ( stringEqual(mainTargetName, targets[i].name) ) {t_num = i; break;}//find the main target in the global target array
	}

	if (t_num == -1)
	{
		printf("%s\n", mainTargetName);
		printf("Error: target does not exist\n");
		return -1;
	}
	
	if(targets[t_num].num_dependencies == 0)//if the target has no dependencies, run its command
	{
			
		if(targets[t_num].command != "")//make sure command exists
		{
			for (i=0; i<targets[t_num].num_dependencies; i++)
			{
				compFlag = compare_modification_time(targets[t_num].name, targets[t_num].dependencies[i]);
				//check if the file is older than any of its dependencies before the command is run
				if ( compFlag == -1 || compFlag == 2 || Bflag == 1 )	{ break; }
			}
			
			if ( compFlag == -1 || compFlag == 2 || Bflag == 1 )
			{
				printf("%s\n", targets[t_num].command);
				if(nflag != 1)
				{
					int numCommands = makeargv(targets[t_num].command, delim, &splitCommand);
					execvp(*splitCommand, splitCommand);//execute command
					freemakeargv(splitCommand);
				}
				
			}
			else { printf("%s is up to date.\n", targets[t_num].name); }
		}
		
		return 1;
	}	

	for (i = 0; i < targets[t_num].num_dependencies; i++)
	{
		currentDependency = targets[t_num].dependencies[i];	//check to see if dependencies are targets in the global target array
		foundTargetFlag = 0;
		int d_num = -1; //array position of the new target being built	
		for (j = 0; j < num_targets; j++)
		{

			if ( stringEqual(currentDependency, targets[j].name) ) 
			{
				foundTargetFlag = 1;//set flag if dependency is a target
				d_num = j;
				break;
			}
		}
		if ( is_file_exist(currentDependency) == -1 && !foundTargetFlag )//If the dependency neither exists as a file or a target, exit program (bogus dependency)
		{
			printf("Error: dependency %s does not exist\n", currentDependency);
			kill(0,SIGKILL);
		}

		if ( foundTargetFlag )//if dependency was found as a target, fork and build that dependency as a target
		{
			childpid = fork();
			if (childpid == -1) {perror("Failed to fork"); return 1;}
			if (childpid == 0)
			{
				build(targets[d_num].name);
				return 1;
			}	
		}

		if ( !foundTargetFlag )//otherwise, run the command for the target (after checking modification times)
		{
			if(targets[t_num].command != "")
			{
				for (j=0; j<targets[t_num].num_dependencies; j++)
				{
					compFlag = compare_modification_time(targets[t_num].name, targets[t_num].dependencies[j]);
					if ( compFlag == -1 || compFlag == 2 || Bflag == 1 )	{ break; }
				}
			
				if ( compFlag == -1 || compFlag == 2 || Bflag == 1 )
				{
					printf("%s\n", targets[t_num].command);
					if(nflag != 1)
					{
						int numCommands = makeargv(targets[t_num].command, delim, &splitCommand);
						execvp(*splitCommand, splitCommand);
						freemakeargv(splitCommand);
					}
					
				}
				else { printf("%s is up to date.\n", targets[t_num].name); }
			}
			return 1;
		}
	}
	pid_t pid;
	while (pid = waitpid(-1, NULL, 0) && errno != ECHILD) {}//wait for all children to finish running
	
	if(targets[t_num].command != "")//run the command for the target after checking modification times
	{
		for (i=0; i<targets[t_num].num_dependencies; i++)
		{
			compFlag = compare_modification_time(targets[t_num].name, targets[t_num].dependencies[i]);
			if ( compFlag == -1 || compFlag == 2 || Bflag == 1 )	{ break; }
		}
		
		if ( compFlag == -1 || compFlag == 2 || Bflag == 1 )
		{
			printf("%s\n", targets[t_num].command);
			if(nflag != 1)
			{
				int numCommands = makeargv(targets[t_num].command, delim, &splitCommand);
				execvp(*splitCommand, splitCommand);
				freemakeargv(splitCommand);
			}
			
		}
		else { printf("%s is up to date.\n", targets[t_num].name); }
	}
	
	return 1;
}

int main(int argc, char **argv) 
{
	
	int i,j;
	int targ_specified_flag = 1;//1 if target is specified by user
	// Declarations for getopt
	extern int optind;
	extern char * optarg;
	int ch;
	char * format = "f:hnBm:";

	// Default makefile name will be Makefile
	char szMakefile[64] = "Makefile";
	char szTarget[64];
	char szLog[64];

	while((ch = getopt(argc, argv, format)) != -1) 
	{
		switch(ch) //set flags as necessary due to user input
		{
			case 'f':
				strcpy(szMakefile, strdup(optarg));
				break;
			case 'n':
				nflag = 1;
				break;
			case 'B':
				Bflag = 1;
				break;
			case 'm':
				mflag = 1;
				strcpy(szLog, strdup(optarg));
				logFile = fopen(szLog, "a+");//open the file specified with read, write privileges in append mode
				fd = fileno(logFile);//get the file descriptor for the set file
				break;
			case 'h':
			default:
				show_error_message(argv[0]);
				exit(1);
		}
	}
	
	argc -= optind;
	argv += optind;
	
	// at this point, what is left in argv is the targets that were 
	// specified on the command line. argc has the number of them.
	// If getopt is still really confusing,
	// try printing out what's in argv right here, then just running 
	// with various command-line arguments.

	if(argc > 1)
	{
		show_error_message(argv[0]);
		return EXIT_FAILURE;
	}

	//You may start your program by setting the target that make4061 should build.
	//if target is not set, set it to default (first target from makefile)
	char *mainTargetName;
	if(argc == 1)
	{
		mainTargetName = argv[0];
	}
	else
	{
		targ_specified_flag = 0;
	}


	/* Parse graph file or die */
	if((parse(szMakefile)) == -1) 
	{
		return EXIT_FAILURE;
	}

	//after parsing the file, you'll want to check all dependencies (whether they are available targets or files)
	//then execute all of the targets that were specified on the command line, along with their dependencies, etc.
	if ( !targ_specified_flag ) { mainTargetName = targets[0].name; }
	//if target not specified, set target to first target in global target array

	build(mainTargetName);//run build on the main target
	if(mflag == 1){fclose(logFile);}//close the log file if it was opened
	return EXIT_SUCCESS;
}
