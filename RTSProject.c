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
     int rflags;
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
void checkFlags(Environment *env);

void junkData(Environment *env);

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

void clean(){
     destroyEnv(env1);
     destroyEnv(env2);
     destroyEnv(env3);
}

void *collect1()
{
     while (1)
     {
          if(checkflags(env1))
          {
                   // TODO semaphores
                   junkData(env1);
          }
          // TODO something with env1->data
     }
}

void *collect2()
{
     while (1)
     {
          if(checkflags(env2))
          {
                   if(sem_wait(&env->mutex) == -1){
                        exit(-1);
                   };
                   junkData(env1);
          }
          // TODO something with env2->data
     }
}

void *reader1()
{
     char val;
     while (1)
     {
          if()
          {          
          val = env1->data;          
          printf("Reader1: %s\n", val);
          env1->
          }
     }
}

void *reader2()
{
     char val;
     while (1)
     {
          val =env2->data;
          val = tolower(val);
          printf("Reader2: %s\n",val);
     }
}

void *reader3()
{
     char data;
     while (1)
     {
          //TODO receive/retrieve data through IPC
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

int checkFlags(Environment * env){
     return env->rflags; 
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
          free(data);
     }
}
