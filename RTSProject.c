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

#include "Environment.h"

#define TIME_PERIOD 5
#define SIZE 1
#define ENV_SIZE 10
pthread_t collect_t1;
pthread_t collect_t2;
pthread_t reader_t1;
pthread_t reader_t2;
pthread_t reader_t3;

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
               safeWait(env1);

               // collect the junk data
               junkData(env1);

               // set the read flag
               env1->rflag = 0;
               
               safePost(env1)
          }
          if (checkFlags(env3))
          {
               safeWait(env3);

               // collect the junk data
               junkData(env3);
          
               safePost(env3);
          }
     }
}

void *collect2()
{
     static char fill = 'A';

     while (1)
     {
          if (checkFlags(env2))
          {
               safeWait(env2);

               // collect the junk data
               junkData(env2);

               // set the read flag
               env2->rflag = 0;

               safeWait(env2);
          }
          if (checkFlags(env3))
          {
               safeWait(env3);

               // fill the second half with an incrementing character. 
               // This makes it more likely that every colour will appear
               int i = env3->size / 2;
               for (i; i < env3->size; i++)
               {
                    env3->data[i] = fill;
               }

               if(fill < 'Z'){
                    fill ++;
               }else{
                    fill = 'A';
               }

               // set the read flag
               env3->rflag = 0;

               safePost(env3);
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
               safeWait(env1);

               printf("Reader1: %s\n", env1->data);
               env1->rflag = 1;

               safePost(env1);
          }
     }
}

void *reader2()
{
     while (1)
     {
          if (!checkFlags(env2))
          {
               safeWait(env2);
               lower(env2);
               printf("Reader2: %s\n", env2->data);
               env2->rflag = 1;
               safePost(env2);
          }
          // TODO instead of sleeping, we need to wake up this thread from a timer event
          sleep(1);
     }
}

void *reader3()
{
     while (1)
     {
          if (!checkFlags(env3))
          {
               safeWait(env3);

               changeColor(env3);
               printf("Reader3: Changing color based on the stats of the environment!\n");
               printf("Reader3: %s\n", env3->data);
               env3->rflag = 1;

               safePost(env3);
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



// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.prog%2Ftopic%2Finthandler_Attaching.html
// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fclockperiod.html