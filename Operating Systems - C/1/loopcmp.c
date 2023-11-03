#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINELEN (80)

char *mygets(char *buf, int len);

int main(int argc, char *argv[])
{
	char firstString[LINELEN + 1];
	char secondString[LINELEN + 1];
	
	if (argc != 2)
	{
		fprintf(stderr, "You need to send 2 parameters (program and comparison function)\n");
		return 0;
	}
	else
	{
		char* theProgram = NULL;
		
		if (argv[1][0] == '/')
		{
			theProgram = strdup(argv[1]);
			if(!theProgram)
				exit(0);
		}		
			
		else
		{
			int lenOfTheProgramCommand = strlen("./") + strlen(argv[1]) + 1;
			theProgram = (char*)malloc(lenOfTheProgramCommand * sizeof(char));
			if(!theProgram)
				return 0;
				
			sprintf(theProgram, "./%s", argv[1]);	// create the string -> ./nameOfProgramFormArgv[1]
		}
		
		while (mygets(firstString, LINELEN) != NULL && mygets(secondString, LINELEN) != NULL)
		{	
		
			int pid = fork();
			int status = 0;
			int resultProcessChild;
			
			if (pid < 0)	// fork failed; exit with the value -2
			{
				fprintf(stderr, "fork failed\n");
				free(theProgram);
				exit(-2);
			} 
			else if (pid == 0)	// child (new process)
			{
				char* newArgv[] = {theProgram, firstString, secondString, NULL};
				if (execvp(newArgv[0], newArgv))
				{
					exit(-2);	// If execve returns so have a system-call error
				}	
			}
			else	// the parent process
			{	
				wait(&status);	//waitpid(pid, &status, 0) - another option is to use, but the difference is to wait for the specific child process
				if (WIFEXITED(status))
				{
					resultProcessChild = WEXITSTATUS(status);
					if (resultProcessChild == 254)	// in Linux -2 it is 254
						printf("System Call Error: %d\nCheck the parameter you sent when running the program\nPlease press: * Ctrl-d * and restart the program\n", resultProcessChild);
					else
						printf("The result of the child process is %d\n", resultProcessChild);		
				}	
			}
		}
		free(theProgram);
	}
	exit(1);
} 

char *mygets(char *buf, int len)
{
	char *retval;

	retval = fgets(buf, len, stdin);
	buf[len] = '\0';
	if (buf[strlen(buf) - 1] == 10) /* trim \r */
		buf[strlen(buf) - 1] = '\0';
	else if (retval)
		while (getchar() != '\n')
			; /* get to eol */

	return retval;
}

