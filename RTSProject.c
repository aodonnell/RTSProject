#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>
#include <float.h>
#include <pthread.h>

#define BLACK "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define YELOW "\x1B[33m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"

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
     int rflag;
     int size;
     char *data;
} Environment;

// environment struct
Environment *env1;
Environment *env2;
Environment *env3;

struct sigevent event;
volatile unsigned counter;
const struct sigevent *handler(void *area, int id)
{
     // Wake up thread every 100th interrupt
     if (++counter == 100)
     {
          counter = 0;
          return (&event);
     }
     else
     {
          return (NULL);
     }
}

int N = 0;
int nbytes;
void init();
void join();
void clean();

void *collect1();
void *collect2();
void *reader1();
void *reader2();
void *reader3();

void checkresult(int result, char *text);
int checkFlags(Environment *env);
void lower(Environment *env);
void junkData(Environment *env);
void changeColor(Environment *env);

Environment *createEnv(size_t size);
void destroyEnv(Environment *env);

int main()
{
     init();
     join();
     clean(); 
}

void init()
{
     int id;

     // Requensting IO privileges
     ThreadCtl(_NTO_TCTL_IO, 0);
     //Initialize event structure
     event.sigev_notify = SIGEV_INTR;

     //TODO set clock period using ClockPeriod()

     //Attach ISR vector
     id = InterruptAttach(0, &handler, NULL, 0, 0);

     // Create environments
     env1 = createEnv(ENV_SIZE);
     env2 = createEnv(ENV_SIZE);
     env3 = createEnv(ENV_SIZE);

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

void join()
{
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

void clean()
{
     destroyEnv(env1);
     destroyEnv(env2);
     destroyEnv(env3);
}

void *collect1()
{
     while (1)
     {
          if (checkFlags(env1))
          {
               if (sem_wait(&env1->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };

               // collect the junk data
               junkData(env1);

               // set the flag
               env1->rflag = 0;
               if (sem_post(&env1->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
          }
          if (checkFlags(env3))
          {
               if (sem_wait(&env3->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };

               // collect the junk data
               junkData(env3);

               if (sem_post(&env3->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
          }
     }
}

void *collect2()
{
     while (1)
     {
          if (checkFlags(env2))
          {
               if (sem_wait(&env2->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
               // collect the junk data
               junkData(env2);

               // set the flag
               env2->rflag = 0;

               if (sem_post(&env2->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
          }
          if (checkFlags(env3))
          {
               if (sem_wait(&env3->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };

               // fill the second half with Ms
               // M is halfway through the alphabet
               int i = env3->size / 2;
               for (i; i < env3->size; i++)
               {
                    env3->data[i] = 'M';
               }

               // set the flag
               env3->rflag = 0;

               if (sem_post(&env3->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
          }
     }
}

void *reader1()
{
     char val;
     while (1)
     {
          if (!checkFlags(env1))
          {
               if (sem_wait(&env1->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
               printf("Reader1: %s\n", env1->data);
               env1->rflag = 1;

               if (sem_post(&env1->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
          }
     }
}

void *reader2()
{
     while (1)
     {
          if (!checkFlags(env2))
          {
               if (sem_wait(&env2->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
               lower(env2);
               printf("Reader2: %s\n", env2->data);
               env2->rflag = 1;

               if (sem_post(&env2->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
          }
          sleep(1);
     }
}

void *reader3()
{
     while (1)
     {
          if (!checkFlags(env3))
          {
               if (sem_wait(&env3->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
               printf("Reader3: Changing color based on environment!\n");
               changeColor(env3);
               printf("Reader3: %s\n", env3->data);
               env3->rflag = 1;

               if (sem_post(&env3->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };
          }
          sleep(1);
     }
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

void lower(Environment *env)
{
     int i;
     for (i = 0; i < env->size; i++)
     {
          env->data[i] |= ' ';
     }
}

int checkFlags(Environment *env)
{
     return env->rflag;
}

Environment *createEnv(size_t size)
{
     Environment *env = calloc(1, sizeof(Environment));

     if (sem_init(&env->mutex, 1, 1) == -1)
     {
          printf("Sem init ERROR = %i.\n", errno);
          exit(1);
     }
     env->size = size;
     env->data = malloc(sizeof(char) * size);
     env->rflag = 1;

     return env;
}

void destroyEnv(Environment *env)
{
     if (env)
     {
          if (env->data)
          {
               free(env->data);
          }
          free(env->data);
     }
}

void changeColor(Environment *env)
{
     // compute the average of the characters
     int sum = 0;
     float avg = 0.0;
     int i = 0;

     for (i; i < env->size; size++)
     {
          sum += env->data[i];
     }

     avg = sum / env->size;

     if (avg < 4)
     {
          printf("%s", RED);
     }
     else if (avg < 8)
     {
          printf("%s", GREEN);
     }
     else if (avg < 12)
     {
          printf("%s", YELLOW);
     }
     else if (avg < 16)
     {
          printf("%s", BLUE);
     }
     else if (avg < 20)
     {
          printf("%s", CYAN);
     }
     else if (avg < 24)
     {
          printf("%s", MAGENTA);
     }
     else
     {
          printf("%s", WHITE);
     }
};

// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.prog%2Ftopic%2Finthandler_Attaching.html
// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fclockperiod.html