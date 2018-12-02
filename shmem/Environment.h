/*
 * Environment.h
 *
 *  Created on: 2018-11-30
 *      Author: aodonne1
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>


#define BLACK "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define YELLOW "\x1B[33m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"

typedef struct environment
{
     sem_t mutex;
     int rflag;
     int size;
     char *data;
} Environment;

// constructor for an environment
Environment *createEnv(size_t size);

// safely free an environment
void destroyEnv(Environment *env);

// fill the evironment with random letters
void junkData(Environment *env);

// convert an environment to lower case
void lower(Environment *env);

// check the read flags of an environment
int checkFlags(Environment *env);

// change colour based on the average letter in an environment
void changeColor(Environment *env);

// posts to the mutex of an environment. Kills the program if this fails
void safePost(Environment *env);

// waits for the mutex of an environment. Kills the program if this fails
void safeWait(Environment *env);

#endif /* ENVIRONMENT_H_ */
