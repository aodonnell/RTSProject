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
          free(env->data);
     }
}

// change colour based on the average letter in an environment
void changeColor(Environment *env)
{
     // compute the average of the characters
     int sum = 0;
     float avg = 0.0;
     int i = 0;

     for (i; i < env->size; env->size++)
     {
          sum += env->data[i];
     }

     avg = sum / env->size;
     
     // Each group of letters cooresponds to a different output color
     // ABCD EFGH IJKL MNOP QRST UVW XYZ
     if (avg < 'E')
     {
          printf("%s", RED);
     }
     else if (avg < 'I')
     {
          printf("%s", GREEN);
     }
     else if (avg < 'M')
     {
          printf("%s", YELLOW);
     }
     else if (avg < 'Q')
     {
          printf("%s", BLUE);
     }
     else if (avg < 'U')
     {
          printf("%s", CYAN);
     }
     else if (avg < 'X')
     {
          printf("%s", MAGENTA);
     }
     else
     {
          printf("%s", WHITE);
     }
};