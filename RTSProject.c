#include <errno.h>
#include <string.h>
#include <stdlib.h>
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
#define ENV_SIZE 10
pthread_t collect_t1;
pthread_t collect_t2;
pthread_t reader_t1;
pthread_t reader_t2;
pthread_t reader_t3;

typedef struct environment
{
     sem_t mutex;
     int size;
     char *data;
} Environment;

// environment struct
Environment env1;
Environment env2;

int N = 0;
int nbytes;
void init();
void join();
void *collect1();
void *collect2();
void *reader1();
void *reader2();
void *reader3();
void checkresult(int result, char *text);
void timer_expired(int called_via_signal);

void junkData(Environment *env);

int main()
{
     init();
     // init fake environments
     bzero(&env1, sizeof(Environment));
     if (sem_init(&env1.mutex, 1, 1) == -1)
     {
          printf("Sem init ERROR = %i.\n", errno);
          exit(1);
     }
     env1.size = ENV_SIZE;
     env1.data = calloc(ENV_SIZE, sizeof(char));

     bzero(&env2, sizeof(Environment));
     if (sem_init(&env2.mutex, 1, 1) == -1)
     {
          printf("Sem init ERROR = %i.\n", errno);
          exit(1);
     }
     env2.size = ENV_SIZE;
     env2.data = calloc(ENV_SIZE, sizeof(char));

     // extern void timer_expired();
     int i, d, nbytes, cps;
     char *addr;

     nbytes = sizeof(env1);

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
     p = (struct phu *)mmap(NULL, nbytes, PROT_READ|PROT_WRITE, MAP_SHARED, d, 0);

     if(p == (struct phu *) -1) {
          close(d);
          printf("Unable to mmap.\n"); 
          exit(1);
     }

     shm_unlink(SHMNAME);

     // Create semaphore.
     if (sem_init(&p->s, 1, 1) == -1)
     {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     }

     // Begin test - repeatedly acquire mutual exclusion, write to area and
     // release mutual exclusion.

     addr = p->beginning_of_data;
     N = 0;

     //alarm(TIME_PERIOD);

     // todo remove me
     while (TRUE)
     {

          // Acquire parents lock.
          if (sem_wait(&p->s) == -1)
          {
               printf("Sem_wait error.\n");
          }

          // Store data in shared memory.
          for (i = 0; i < SIZE; i++)
          {
               addr[i] = 'A';
               //               printf("i = %d. N = %d.\n", i, N);
               // Uncomment this line to debug
          }

          // Release parents lock.
          if (sem_post(&p->s) == -1)
          {
               printf("Sem_post error.\n");
          }

          N++;
     }

     // deallocate our "Environments"
     free(env1.data);
     free(env2.data);

}

void init()
{
     int result;
     // Collect1
     result = pthread_create(&collect_t1, NULL, collect1, NULL);
     checkresult(result, "Thread create failed");

     // Collect2
     result = pthread_create(&collect_t2, NULL, collect2, NULL);
     checkresult(result, "Thread create failed");

     // Reader1
     result = pthread_create(&reader_t1, NULL, reader1, NULL);
     checkresult(result, "Thread create failed");

     // Reader2
     result = pthread_create(&reader_t2, NULL, reader2, NULL);
     checkresult(result, "Thread create failed");

     // Reader3
     result = pthread_create(&reader_t3, NULL, reader3, NULL);
     checkresult(result, "Thread create failed");
}

void join(){
     int result;
     result = pthread_join(collect_t1, NULL);
     checkresult(result, "Thread join failed");
     result = pthread_join(collect_t2, NULL);
     checkresult(result, "Thread join failed");
     result = pthread_join(reader_t1, NULL);
     checkresult(result, "Thread join failed");
     result = pthread_join(reader_t2, NULL);
     checkresult(result, "Thread join failed");
     result = pthread_join(reader_t3, NULL);
     checkresult(result, "Thread join failed");          
}



void timer_expired(int called_via_signal)
{
     printf("%d iterations in %i seconds\n", N, TIME_PERIOD);
     exit(0);
}

void *collect1()
{    
     int fd;
     while (1)
     {
          // TODO semaphores
          junkData(&env1);
          fd = shm_open("SHMNAME",O_RDONLY, S_IREAD);   
	     if (fd == -1) {
		     printf("Error opening the shared memory object: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	     }                           
     }
}

void *collect2()
{
     while (1)
     {
          // TODO semaphores
          junkData(&env2);
     }
}

void *reader1()
{
}

void *reader2()
{
}

void *reader3()
{
}

void checkresult(int result, char *text)
{
     if (result == -1)
     {
          perror(text);
          exit(1);
     }
}

void junkData(Environment *env)
{
     int i;
     for (i = 0; i < env->size; i++)
     {
          env->data[i] = 'A' + (random() % 26);
     }
}
