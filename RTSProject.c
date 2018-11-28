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
#include <sys/stat.h>
#include <signal.h>
#include <math.h>
#include <float.h>
#include <pthread.h>

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

Environment *createEnv(size_t size);
void destroyEnv(Environment *env);

int main()
{
	 puts("init");
     init();
     join();
     puts("joined");
     clean();
}

void init()
{
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

//	puts("Starting collect 1");

     while (1)
     {
          if (checkFlags(env1))
          {
               if (sem_wait(&env1->mutex) == -1)
               {
                    exit(EXIT_FAILURE);
               };

//               puts("Collecting data for env1");


               // collect the junk data
               junkData(env1);

//               puts("Setting flag1");
               // set the flag
               env1->rflag = 0;
//               puts("Posting Sem1");
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
//               puts("Collecting data for env3");

               // fill the first half with As
               int i = 0;
               for (i; i < env3->size / 2; i++)
               {
                    env3->data[i] = 'A';
               }
//               puts("Posting Sem3");
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

               // fill the second half with Bs
               int i = env3->size / 2;
               for (i; i < env3->size; i++)
               {
                    env3->data[i] = 'B';
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
     char data;
     while (1)
     {
          printf("leds are stupid");
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
// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.prog%2Ftopic%2Finthandler_Attaching.html
// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fclockperiod.html