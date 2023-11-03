#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

void parseargs(char *argv[], int argc, int *lval, int *uval, int *nval, int *pval);
int isprime(int n);
void errExit(const char* errMsg);

int main (int argc, char **argv)
{
  int lval = 1;
  int uval = 100;
  int nval = 10;		/* Number of primes to print - the defalut is 10 */
  int pval = 4;			/* Amount of process - the defalut is 4 */
  char *flagarr = NULL;
  int num;
  int count = 0;

  // Parse arguments
  parseargs(argv, argc, &lval, &uval, &nval, &pval);
  if (uval < lval)
  {
    fprintf(stderr, "Upper bound should not be smaller then lower bound\n");
    exit(1);
  }   

  if (lval < 2)
  {
    lval = 2;
    uval = (uval > 1) ? uval : 1;
  }

  /* Set flagarr - Use MAP_ANONYMOUS */
  flagarr = mmap(NULL, sizeof(char) * (uval-lval+1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (flagarr == MAP_FAILED)
	  errExit("mmap is failed");  

  /* if the user doesn't send: -p 0, so the process will create children and will set flagarr */
  if (pval > 0)
  {
    for(int i = 0; i < pval; i++)
    { 
      switch (fork())
  		{
  			case -1:
  				errExit("the fork is failed");
  			
  			case 0:
  				for (num = lval + i; num <= uval; num+=pval)
  				{
  					if (isprime(num))
  						flagarr[num - lval] = 1;
  					else
  						flagarr[num - lval] = 0;	
  				}
  				exit(EXIT_SUCCESS);

  			default:
  				break;		
  		}
    }
    /* Wating to all children */
    for (int i = 0; i < pval; i++)
    {
      if (wait(NULL) == -1)
        errExit("the waiting for the child failed");
    }
  }
  else  /* if to the process doesn't has children, so only the main process will set flagarr */
  {
    for (num = lval; num <= uval; num++)
    {
      if (isprime(num))
      {
        flagarr[num - lval] = 1; 
        count ++;
      } 
      else 
      {
        flagarr[num - lval] = 0; 
      }
    }
  }
  
  /* Checking how much primes have in the array, it will happen only if pval > 0 because the main process already has counter */
  if (pval > 0) 
  {
    for (int i = 0; i<=uval; i++)
    {
      if (flagarr[i] == 1)
        count++;
    }  
  }
  
  /* Printing the results */
  printf("Found %d primes%c\n", count, count ? ':' : '.');
  for (num = lval; num <= uval && nval > 0; num++)
    if (flagarr[num - lval])
    {
      count--;
      printf("%d%c", num, nval > 1 ? ',' : '\n');
      nval--;
    }

  /* Closing shared memory (mmap) */
  if (munmap(flagarr, sizeof(char) * (uval-lval+1)) == -1)
    errExit("munmap is failed");

  return 0;
}

// NOTE : use 'man 3 getopt' to learn about getopt(), opterr, optarg and optopt 
void parseargs(char *argv[], int argc, int *lval, int *uval, int *nval, int *pval)
{
  int ch;

  opterr = 0;
  while ((ch = getopt (argc, argv, "l:u:n:p:")) != -1)
    switch (ch)
    {
      case 'l':  // Lower bound flag
        *lval = atoi(optarg);
        break;
      case 'u':  // Upper bound flag 
        *uval = atoi(optarg);
        break;
      case 'n':	/* Number of primes to print */
      	*nval = atoi(optarg);
      	break;
      case 'p':	/* Amount of process */
      	*pval = atoi(optarg);
      	break;	  
      case '?':
        if ((optopt == 'l') || (optopt == 'u') || (optopt == 'n') || (optopt == 'p'))
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        exit(1);
      default:
        exit(1);
    }    
}

int isprime(int n)
{
  static int *primes = NULL; 	// NOTE: static !
  static int size = 0;		// NOTE: static !
  static int maxprime;		// NOTE: static !
  int root;
  int i;

  // Init primes array (executed on first call)
  if (primes == NULL)
  {
    primes = (int *)malloc(2*sizeof(int));
    if (primes == NULL)
      exit(1);
    size = 2;
    primes[0] = 2;
    primes[1] = 3;
    maxprime = 3;
  }

  root = (int)(sqrt(n));

  // Update primes array, if needed
  while (root > maxprime)
    for (i = maxprime + 2 ;  ; i+=2)
      if (isprime(i))
      {
        size++;
        primes = (int *)realloc(primes, size * sizeof(int));
        if (primes == NULL)
          exit(1);
        primes[size-1] = i;
        maxprime = i;
        break;
      }

  // Check 'special' cases
  if (n <= 0)
    return -1;
  if (n == 1)
    return 0;

  // Check prime
  for (i = 0 ; ((i < size) && (root >= primes[i])) ; i++)
    if ((n % primes[i]) == 0)
      return 0;
  return 1;
}

void errExit(const char* errMsg)
{
	printf("Error message: %s\n", errMsg);
	exit(EXIT_FAILURE);
}
