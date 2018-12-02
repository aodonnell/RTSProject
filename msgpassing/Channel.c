/*
 * Channel.c
 *
 *  Created on: 2018-11-30
 *      Author: aodonne1
 */

#include "Channel.h"

// constructor for an environment
Channel *createChannel(size_t size){
     Channel *ch = calloc(1, sizeof(Channel));

     ch->chid = ChannelCreate(0);
     ch->size = size;
     ch->send_buffer = calloc(size, sizeof(char));
     ch->recv_buffer = calloc(size, sizeof(char));
     ch->reply_buffer = calloc(size, sizeof(char));

     setReply(ch, "Ack");

     return ch;
};

void setReply(Channel * ch, char * reply){
	memcpy(ch->reply_buffer, reply, ch->size);
}

// safely free an environment
void destroyCh(Channel * ch){
     if (ch)
     {
          if (ch->send_buffer)
          {
               free(ch->send_buffer);
          }
          if (ch->recv_buffer)
          {
               free(ch->recv_buffer);
          }
          if (ch->reply_buffer)
          {
               free(ch->reply_buffer);
          }
          free(ch);
     }
};

// fill the send buffer with random letters
void junkData(Channel * ch){


	int i;
     for (i = 0; i < ch->size; i++)
     {
          ch->send_buffer[i] = 'A' + (random() % 26);
     }
};

// convert the recv buffer to lower case
void lower(Channel * ch){
     int i;
     for (i = 0; i < ch->size; i++)
     {
          ch->recv_buffer[i] |=' ';
     }
};

// change colour based on the average letter in a recieved message
void changeColor(Channel * ch){
     // compute the average of the characters
     int sum = 0;
     float avg = 0.0;
     int i;

     for (i = 0; i < ch->size; i++)
     {
          sum += (int) ch->recv_buffer[i];
     }

     avg = (float)sum / (float)ch->size;

     printf("Sum: 0x%X, N: %d. Average: 0x%X ('%c')\n", sum, ch->size, (int) avg, (char) avg);

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

// connect to a channel returns the connection id
int connect(Channel *ch){
     return ConnectAttach(0, 0, ch->chid, 0, NULL);
};

// wait to recv on a certain channel
int recv(Channel * ch){
     return MsgReceive(ch->chid, ch->recv_buffer, ch->size, NULL);
};

// copies the recv buffer to the send buffer of a different channel to forward a message
void fwd(Channel * ch1, Channel * ch2){
	puts("forwarding.");
     if(ch1->size == ch2->size){
          memcpy(ch2->send_buffer, ch1->recv_buffer, ch1->size);
     }else{
    	 printf("WARNING: CHANNELS NOT THE SAME SIZE\n");
     }
}

// send the send buffer an a channel
void send(Channel * ch, int conid){
	printf("sending...\n");
    MsgSend(conid, ch->send_buffer, ch->size, ch->reply_buffer);
    puts("sent!");
};

// reply to the last message recieved
void reply(Channel * ch, int recvid){
     MsgReply(recvid, NULL, ch->reply_buffer, ch->size);
};
