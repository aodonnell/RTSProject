/*
 * Environment.c
 *
 *  Created on: 2018-11-30
 *      Author: aodonne1
 */


#include "Environment.h"

// fill the evironment with random letters
void junkData(Environment *env)
{
     int i;
     for (i = 0; i < env->size; i++)
     {
          env->data[i] = 'A' + (random() % 26);
     }
}

// convert an environment to lower case
void lower(Environment *env)
{
     int i;
     for (i = 0; i < env->size; i++)
     {
          env->data[i] |= ' ';
     }
}

// check the read flags of an environment
int checkFlags(Environment *env)
{
     return env->rflag;
}

// constructor for an environment
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

// safely free an environment
void destroyEnv(Environment *env)
{
     if (env)
     {
          if (env->data)
          {
               free(env->data);
          }
          free(env);
     }
}

// change colour based on the average letter in an environment
void changeColor(Environment *env)
{
     // compute the average of the characters
     int sum = 0;
     float avg = 0.0;
     int i;

     for (i = 0; i < env->size; i++)
     {
          sum += (int) env->data[i];
     }

     avg = (float)sum / (float)env->size;

     printf("Sum: 0x%X, N: %d. Average: 0x%X ('%c')\n", sum, env->size, (int) avg, (char) avg);

     // Each group of letters cooresponds to a different output color
     // ABCDEF GHIJ KLM N OPQ RSTU VWXYZ
     if (avg < 'G')
     {
          printf("%s", RED);
     }
     else if (avg < 'K')
     {
          printf("%s", GREEN);
     }
     else if (avg < 'N')
     {
          printf("%s", YELLOW);
     }
     else if (avg < 'O')
     {
          printf("%s", BLUE);
     }
     else if (avg < 'R')
     {
          printf("%s", CYAN);
     }
     else if (avg < 'V')
     {
          printf("%s", MAGENTA);
     }
     else
     {
          printf("%s", WHITE);
     }
};

void safePost(Environment *env){
    if (sem_post(&env->mutex) == -1)
    {
        exit(EXIT_FAILURE);
    };
};

void safeWait(Environment *env){
    if (sem_wait(&env->mutex) == -1)
    {
        exit(EXIT_FAILURE);
    };
};

void fwd(Environment *env1, Environment *env2){
	if(env1->size == env2->size){
		safeWait(env2);
		memcpy(env2->data,env1->data,env1->size);
		safePost(env2);
	}
	else{
		printf("WARNING: ENVIRONMENTS NOT SAME SIZE");
	}
}
