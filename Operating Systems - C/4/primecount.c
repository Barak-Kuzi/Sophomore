#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>


void parseargs(char *argv[], int argc, int *lval, int *uval, int *nval, int *tval);
int isprime(int n);
void errExit(const char* errMsg);
void* threadFunctionOfPrimesNumbers(void* arg);

int lval = 1;
int uval = 100;
int count = 0;
int currentNumber;
pthread_mutex_t numberLock;
pthread_rwlock_t writeLock;
pthread_mutex_t flagarrLock;

int main (int argc, char **argv)
{
    int nval = 10;		/* Number of primes to print - the defalut is 10 */
    int tval = 4;			/* Amount of threads - the defalut is 4 */
    char *flagarr = NULL;
    int num;
    int creat_result;
    int join_result;

    // Parse arguments
    parseargs(argv, argc, &lval, &uval, &nval, &tval);
    if (uval < lval)
    {
        fprintf(stderr, "Upper bound should not be smaller then lower bound\n");
        exit(1);
    }

    if (tval < 1)
        errExit("the minimum value of tval it's 1 ");

    if (lval < 2)
    {
        lval = 2;
        uval = (uval > 1) ? uval : 1;
    }
    /* initialize the current number to check, from the lowest value */
    currentNumber = lval;
    // Allocate flags
    flagarr= (char *)malloc(sizeof(char) * (uval-lval+1));
    if (flagarr == NULL)
        exit(1);

    pthread_t* threadsArray = (pthread_t*)malloc(tval * sizeof(pthread_t));
    if (threadsArray == NULL)
        errExit("the allocation is failed");

    /* initialize the locks*/
    if (pthread_mutex_init(&numberLock, NULL) != 0)
        errExit("failed to initialize the lock");

    if (pthread_rwlock_init(&writeLock, NULL) != 0)
        errExit("failed to initialize the writeLock");

    if (pthread_mutex_init(&flagarrLock, NULL) != 0)
        errExit("failed to initialize the flagarrLock");

    /* Creating threads */
    for (int i = 0; i < tval; i++)
    {
        creat_result = pthread_create(&threadsArray[i], NULL, threadFunctionOfPrimesNumbers, (void*)flagarr);
        if (creat_result != 0)
            errExit("thread creation failed");
    }

    /* Waiting to threads until they finished */
    for (int i = 0; i < tval; i++)
    {
        join_result = pthread_join(threadsArray[i], NULL);
        if (join_result != 0)
            errExit("thread join failed");
    }

    /* Printing the results */
    printf("Found %d primes%c\n", count, count ? ':' : '.');
    for (num = lval; num <= uval && nval > 0; num++)
    {
        if (flagarr[num - lval])
        {
            count--;
            printf("%d%c", num, nval > 1 ? ',' : '\n');
            nval--;
        }
    }
    /* Destroy the lock when it is no longer needed */
    pthread_mutex_destroy(&numberLock);
    pthread_rwlock_destroy(&writeLock);
    pthread_mutex_destroy(&flagarrLock);

    /* Deallocate the memory when it is no longer needed */
    free(threadsArray);
    free(flagarr);

    return 0;
}

// NOTE : use 'man 3 getopt' to learn about getopt(), opterr, optarg and optopt
void parseargs(char *argv[], int argc, int *lval, int *uval, int *nval, int *tval)
{
    int ch;

    opterr = 0;
    while ((ch = getopt (argc, argv, "l:u:n:t:")) != -1)
        switch (ch)
        {
            case 'l':  // Lower bound flag
                *lval = atoi(optarg);
                break;
            case 'u':  // Upper bound flag
                *uval = atoi(optarg);
                break;
            case 'n': /* Number of primes to print */
                *nval = atoi(optarg);
                break;
            case 't': /* Amount of threads */
                *tval = atoi(optarg);
                break;
            case '?':
                if ((optopt == 'l') || (optopt == 'u') || (optopt == 'n') || (optopt == 't'))
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
    pthread_rwlock_wrlock(&writeLock); // Only one thread will write and nobody will can't read
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
    pthread_rwlock_unlock(&writeLock);

    root = (int)(sqrt(n));

    // Update primes array, if needed
    while (root > maxprime)
        for (i = maxprime + 2 ;  ; i+=2)
            if (isprime(i))
            {
                pthread_rwlock_wrlock(&writeLock);
                size++;
                primes = (int *)realloc(primes, size * sizeof(int));
                /* if the thread using the function exit(), the thread's lock is automatically released by
                the operating system as part of the process termination */
                if (primes == NULL)
                    exit(1);
                primes[size-1] = i;
                maxprime = i;
                pthread_rwlock_unlock(&writeLock);
                break;
            }

    // Check 'special' cases
    if (n <= 0)
        return -1;
    if (n == 1)
        return 0;

    // Check prime
    pthread_rwlock_rdlock(&writeLock); // Function of rwlock, everyone can read but nobody can write
    for (i = 0 ; ((i < size) && (root >= primes[i])) ; i++)
        if ((n % primes[i]) == 0)
        {
            pthread_rwlock_unlock(&writeLock);
            return 0;
        }
    pthread_rwlock_unlock(&writeLock);
    return 1;

}

void errExit(const char* errMsg)
{
    printf("Error message: %s\n", errMsg);
    exit(EXIT_FAILURE);
}

void* threadFunctionOfPrimesNumbers(void* arg)
{
    int numToCheck;
    char* threadFlagarr = (void*)arg;

    while(1)
    {
        pthread_mutex_lock(&numberLock);
        numToCheck = currentNumber++;
        pthread_mutex_unlock(&numberLock);
        /* Check if the current number bigger than the upper bound */
        if (numToCheck > uval)
            break;

        if(isprime(numToCheck))
        {
            pthread_mutex_lock(&flagarrLock);
            threadFlagarr[numToCheck - lval] = 1;
            count++;
            pthread_mutex_unlock(&flagarrLock);
        } else
        {
            pthread_mutex_lock(&flagarrLock);
            threadFlagarr[numToCheck - lval] = 0;
            pthread_mutex_unlock(&flagarrLock);
        }
    }
    return NULL;
}