#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <signal.h>
#include <math.h>
#include <float.h>
#include <pthread.h>

#define SHMNAME "/my_shm"
#define SEMNAME "/my_sem"
#define TRUE 1
#define TIME_PERIOD 5

#define SIZE 1

int N = 0;
int nbytes;

void * collect1();
void * collect2();
void * reader1();
void * reader2();
void * reader3();

typedef struct environment {
	int x[100];
} Environment;



void timer_expired(int called_via_signal)
{
    printf("%d iterations in %i seconds\n", N, TIME_PERIOD);
    exit(0);
}


main()
{
     extern void timer_expired();
     int i, d, nbytes, cps;
     char *addr;

     struct sigaction sa;

     // The following structure is overlaid on the shared memory.
     struct phu {
          sem_t s;
          char beginning_of_data[SIZE];
     } *p;

     nbytes = sizeof(*p);

     // Set up signal handler.
     sa.sa_handler = timer_expired;
     sa.sa_flags = 0;
          sigemptyset(&sa.sa_mask);
          if (sigaction(SIGALRM, &sa, NULL) < 0) {
              perror("sigaction SIGALRM.\n");
              exit(1);
           }

     // Create shared memory region.
     if ((d = shm_open(SHMNAME, O_RDWR|O_CREAT|O_EXCL, 0666)) == -1) {
          printf("Unable to open shared memory.\n");
          exit(1);
     }
     if (ftruncate(d, nbytes) != 0) {
          close(d);
          shm_unlink(SHMNAME);
          printf("Unable to truncate.\n");
          exit(1);
     }
     p = (struct phu *)mmap(NULL, nbytes, PROT_READ|PROT_WRITE, MAP_SHARED, d, 
0);
     if(p == (struct phu *) -1) {
          close(d);
          printf("Unable to mmap.\n"); 
          exit(1);
     }

     shm_unlink(SHMNAME);

     // Create semaphore.
     if (sem_init(&p->s, 1, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     }

     // Begin test - repeatedly acquire mutual exclusion, write to area and
     // release mutual exclusion.
     
     addr = p->beginning_of_data;
     N = 0;

     alarm(TIME_PERIOD);

     while ( TRUE ) {
 
          // Acquire parents lock.
          if (sem_wait(&p->s) == -1) {
              printf("Sem_wait error.\n");
          }
 
         // Store data in shared memory.
          for (i = 0; i < SIZE; i++) {
               addr[i] = 'A';
//               printf("i = %d. N = %d.\n", i, N);
   // Uncomment this line to debug
          }
 
          // Release parents lock.
          if (sem_post(&p->s) == -1) {
              printf("Sem_post error.\n");
           }

          N++;
     }
} 

void * collect1(){

};

void * collect2(){

};

void * reader1(){

};

void * reader2(){

};
void * reader3(){

};