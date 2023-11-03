#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define LINELEN (80)
#define READ_END 0
#define WRITE_END 1

char *mygets(char *buf, int len);
int mygeti();
void errExit(const char* errMsg);	/* The function prints an error message and returns -2 */

int main(int argc, char *argv[])
{
	char *cmpstr[] = {"lexcmp", "lencmp"};
	int veclen = sizeof(cmpstr)/sizeof(char *);
	char firstString[LINELEN + 1];
	char secondString[LINELEN + 1];
	int index;
	
	int pfdParentToFirstChild[2];  		 /* Pipe file descriptors: Parent Write -> child 1*/
    int pfdParentToSecondChild[2]; 	 	/* Pipe file descriptors: Parent Write -> child 2*/
    int pfdOneOfChildernToParent[2];    /* Pipe file descriptors: One of childern Write -> parent*/
    
    if (pipe(pfdParentToFirstChild) == -1)		/* Create pipe */
    	errExit("pipe creation failed");
    	
    if (pipe(pfdParentToSecondChild) == -1)		/* Create pipe */	
    	errExit("pipe creation failed");
    	
    if (pipe(pfdOneOfChildernToParent) == -1)	/* Create pipe */
    	errExit("pipe creation failed");
    	
    switch(fork())		/* The program loopcmp with the comparison function lexcmp (first child) */
	{
		case -1:   	/*fork failed*/	
			errExit("the first fork is failed");
	
		case 0:		/* First Child Process */
			if (close(pfdParentToFirstChild[WRITE_END]) == -1)      /* Write end is unused */
                errExit("the closing of write end of file descriptor failed");

            if (close(pfdParentToSecondChild[READ_END]) == -1)       /* Read end is unused */
                errExit("the closing of read end of file descriptor failed");
        
            if (close(pfdParentToSecondChild[WRITE_END]) == -1)       /* Write end is unused */
                errExit("the closing of write end of file descriptor failed");

            if (close(pfdOneOfChildernToParent[READ_END]) == -1)       /* Read end is unused */
                errExit("the closing of read end of file descriptor failed");
		
			if (pfdParentToFirstChild[READ_END] != STDIN_FILENO)         /* Defensive check */
			{ 			
				if (dup2(pfdParentToFirstChild[READ_END], STDIN_FILENO) == -1)      /* The child will read from the standard input */
                    errExit("redirected of read end of the pipe to standard input failed");
				if (close(pfdParentToFirstChild[READ_END]) == -1)       /* The original read end is no longer needed */
                    errExit("the closing of read end of file descriptor failed");				
			}
		
			if (pfdOneOfChildernToParent[WRITE_END] != STDOUT_FILENO)       /* Defensive check */
			{	
				if (dup2(pfdOneOfChildernToParent[WRITE_END], STDOUT_FILENO) == -1)       /* The child will write to the standard output (the print of loopcmp) */
                    errExit("redirected of write end of the pipe to standard output failed");
				if (close(pfdOneOfChildernToParent[WRITE_END]) == -1)       /* The original write end is no longer needed */
                    errExit("the closing of write end of file descriptor failed"); 
			}
		
			char* newArgv1[] = {"./loopcmp", "lexcmp", NULL};
			if (execvp(newArgv1[0], newArgv1))
				errExit("execve returns so have a system-call error");				
			
		default: 	/* if (pid > 0), the parent go there and don't do anything and continue to the second fork   */
			break;
	}
	
	switch(fork())		/* The program loopcmp with the comparison function lencmp (second child) */
	{
		case -1:   	/*fork failed*/
			
			errExit("the second fork is failed");
	
		case 0:		/* Second Child Process */
			if (close(pfdParentToFirstChild[READ_END]) == -1)      /* Read end is unused */
                errExit("the closing of read end of file descriptor failed");

            if (close(pfdParentToFirstChild[WRITE_END]) == -1)      /* Write end is unused */
                errExit("the closing of write end of file descriptor failed");    

            if (close(pfdParentToSecondChild[WRITE_END]) == -1)       /* Write end is unused */
                errExit("the closing of write end of file descriptor failed");

            if (close(pfdOneOfChildernToParent[READ_END]) == -1)       /* Read end is unused */
                errExit("the closing of read end of file descriptor failed");
		
			if (pfdParentToSecondChild[READ_END] != STDIN_FILENO)         /* Defensive check */
			{ 			
				if (dup2(pfdParentToSecondChild[READ_END], STDIN_FILENO) == -1)      /* The child will read from the standard input */
                    errExit("redirected of read end of the pipe to standard input failed");
				if (close(pfdParentToSecondChild[READ_END]) == -1)       /* The original read end is no longer needed */
                    errExit("the closing of read end of file descriptor failed");		
			}
		
			if (pfdOneOfChildernToParent[WRITE_END] != STDOUT_FILENO)       /* Defensive check */
			{	
				if (dup2(pfdOneOfChildernToParent[WRITE_END], STDOUT_FILENO) == -1)       /* The child will write to the standard output (the print of loopcmp) */
                    errExit("redirected of write end of the pipe to standard output failed");
				if (close(pfdOneOfChildernToParent[WRITE_END]) == -1)       /* The original write end is no longer needed */
                    errExit("the closing of write end of file descriptor failed");
			}
		
			char* newArgv2[] = {"./loopcmp", "lencmp", NULL};
            if (execvp(newArgv2[0], newArgv2))
            	errExit("execve returns so have a system-call error");
			
		default: 	/* if (pid > 0), the parent go there and don't do anything and continue to the second fork   */
			break;
	}
	
	if (close(pfdParentToFirstChild[READ_END]) == -1)      /* Read end is unused */
        errExit("the closing of read end of file descriptor failed");
        
	if (close(pfdParentToSecondChild[READ_END]) == -1)       /* Read end is unused */
        errExit("the closing of read end of file descriptor failed");
        
	if (close(pfdOneOfChildernToParent[WRITE_END]) == -1)       /* Write end is unused */
        errExit("the closing of write end of file descriptor failed");
		
	while (1)
	{
		printf("\nAfter entering the strings, select a comparison function\n");
        printf("\nPlease enter first string:\n");
		if (mygets(firstString, LINELEN) == NULL)
            break;
        printf("\nPlease enter second string:\n");
        if (mygets(secondString, LINELEN) == NULL)
            break;
		do {
			printf("\nPlease select the string comparison type:\n");
			for (int i=0 ; i < veclen ; i++)
				printf("%d - %s\n", i, cmpstr[i]);
			index = mygeti();
		} while ((index < 0) || (index >= veclen));

		int lenOfFirstString = strlen(firstString);
        int lenOfSecondString = strlen(secondString);
        
        switch(index)
		{
			case 0:		/* The user choose the lexcmp program */
				if (write(pfdParentToFirstChild[WRITE_END], firstString, lenOfFirstString) != lenOfFirstString)
                    errExit("the writing to the pipe is failed - 1");

				if (write(pfdParentToFirstChild[WRITE_END], "\n", 1) != 1)		/* The function mygets in the program loopcmp, reading from stdin until \n */
					errExit("the writing to the pipe is failed - 2");	
                    
				if (write(pfdParentToFirstChild[WRITE_END], secondString, lenOfSecondString) != lenOfSecondString)
                    errExit("the writing to the pipe is failed - 3");

				if (write(pfdParentToFirstChild[WRITE_END], "\n", 1) != 1)		/* The function mygets in the program loopcmp, reading from stdin until \n */
					errExit("the writing to the pipe is failed - 4");	
                    
                break;
                
			case 1:		/* The user choose the lencmp program */
				if (write(pfdParentToSecondChild[WRITE_END], firstString, lenOfFirstString) != lenOfFirstString)
                    errExit("the writing to the pipe is failed - 5");

				if (write(pfdParentToSecondChild[WRITE_END], "\n", 1) != 1)		/* The function mygets in the program loopcmp, reading from stdin until \n */
					errExit("the writing to the pipe is failed - 6");	
				
				if (write(pfdParentToSecondChild[WRITE_END], secondString, lenOfSecondString) != lenOfSecondString)
                    errExit("the writing to the pipe is failed - 7");

				if (write(pfdParentToSecondChild[WRITE_END], "\n", 1) != 1)		/* The function mygets in the program loopcmp, reading from stdin until \n */
					errExit("the writing to the pipe is failed - 8");		
                             
				break;
		}
		
		char resultFromLoopCmp;
		if (read(pfdOneOfChildernToParent[READ_END], &resultFromLoopCmp, sizeof(resultFromLoopCmp)) != sizeof(resultFromLoopCmp))
            errExit("the reading from the pipe is failed - 1");
		
		char cleaningBuffer;
		if (read(pfdOneOfChildernToParent[READ_END], &cleaningBuffer, sizeof(char)) != sizeof(char))
            errExit("the reading from the pipe is failed - 2");

		int finalResult = (int)(resultFromLoopCmp - '0');
        printf("\n%s(%s, %s) == %d\n", cmpstr[index], firstString, secondString, finalResult);
        fflush(stdout);		
	}
	
	if (close(pfdParentToFirstChild[WRITE_END]) == -1)      /* Write end is unused */
        errExit("the closing of write end of file descriptor failed");
        
	if (close(pfdParentToSecondChild[WRITE_END]) == -1)       /* Write end is unused */
        errExit("the closing of write end of file descriptor failed");
        
	if (close(pfdOneOfChildernToParent[READ_END]) == -1)       /* Read end is unused */
        errExit("the closing of read end of file descriptor failed");
        
	exit(EXIT_SUCCESS);
		
}

char *mygets(char *buf, int len)
{
	char *retval;

	retval = fgets(buf, len, stdin);
	buf[len] = '\0';
	if (buf[strlen(buf) - 1] == 10) /* trim \r */
		buf[strlen(buf) - 1] = '\0';
	else if (retval) 
		while (getchar() != '\n'); /* get to eol */

	return retval;
}

int mygeti()
{
	int ch;
	int retval=0;

	while(isspace(ch=getchar()));
	while(isdigit(ch))
	{
		retval = retval * 10 + ch - '0';
		ch = getchar();
	}
	while (ch != '\n')
		ch = getchar();
	return retval;
}

void errExit(const char* errMsg)
{
	printf("Error message: %s\n", errMsg);
	exit(-2);
}
		
