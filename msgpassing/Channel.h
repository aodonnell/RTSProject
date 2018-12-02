/*
 * Channel.h
 *
 *  Created on: 2018-11-30
 *      Author: aodonne1
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

typedef struct channel
{
     int chid;
     int size;
     char *send_buffer;
     char *recv_buffer;
     char *reply_buffer;
} Channel;

// constructor for an environment
Channel *createChannel(size_t size);

void setReply(Channel * ch, char * reply);

// safely free an environment
void destroyCh(Channel * ch);

// fill the evironment with random letters
void junkData(Channel * ch);

// convert an environment to lower case
void lower(Channel * ch);

// change colour based on the average letter in an environment
void changeColor(Channel * ch);

// connect to a channel returns the connection id
int connect(Channel *ch);

// wait to recv on a certain channel
int recv(Channel * ch);

// send the
void send(Channel * ch, int conid);

// reply to the last message recieved
void reply(Channel * ch, int recvid);

void fwd(Channel * ch1, Channel * ch2);

#endif /* MESSAGE_H */

