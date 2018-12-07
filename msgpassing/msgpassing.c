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

#include "Channel.h"
volatile int counter1 = 0;
volatile int counter2 = 0;
volatile int counter3= 0;

#define RUN_TIME 27
#define SIZE 1
#define BUFFER_SIZE 10

typedef union {
    struct _pulse pulse;
} message_t;

message_t msg1, msg2;

pthread_t collect_t1;
pthread_t collect_t2;
pthread_t reader_t1;
pthread_t reader_t2;
pthread_t reader_t3;

// environment struct
Channel *ch1;
Channel *ch2;
Channel *ch3;
Channel *ch4;

volatile unsigned counter;

int N = 0;
int chid1, chid2;
int is_running = 1;
int nbytes;
void init();
void join();
void clean();

const struct sigevent *isr(void *area, int id);
void *killer();
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
//    killer();
}

void init()
{

    // Create environments
    ch1 = createChannel(BUFFER_SIZE);
    ch2 = createChannel(BUFFER_SIZE);
    ch3 = createChannel(BUFFER_SIZE);
    ch4 = createChannel(BUFFER_SIZE);

    int id;
    int result;
    timer_t timer;

    struct sigevent event, event2;
    struct itimerspec itime;

    // Requensting IO privileges
    result = ThreadCtl(_NTO_TCTL_IO, 0);

    // setup timer interval for 0.5 seconds
    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_nsec = (int)5e8;
    itime.it_value.tv_sec = 0;
    itime.it_value.tv_nsec = (int)5e8;

    // init event structures and pulse channel
    chid1 = ChannelCreate(0);
    SIGEV_INTR_INIT(&event);
    InterruptAttachEvent(10, &event, 0);

    event.sigev_notify = SIGEV_PULSE;
    event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid1, _NTO_SIDE_CHANNEL,
                                     0);
    event.sigev_priority = getprio(0);
    event.sigev_code = _PULSE_CODE_MINAVAIL;

    timer_create(CLOCK_MONOTONIC, &event, &timer);
    timer_settime(timer, 0, &itime, NULL);

    // setup timer interval for 1 second
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0;
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0;

    // init event structures and pulse channel

    chid2 = ChannelCreate(0);
    SIGEV_INTR_INIT(&event);
    InterruptAttachEvent(9, &event, 0);

    event.sigev_notify = SIGEV_PULSE;
    event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid2, _NTO_SIDE_CHANNEL,
                                     0);
    event.sigev_priority = getprio(0);
    event.sigev_code = _PULSE_CODE_MINAVAIL;

    timer_create(CLOCK_MONOTONIC, &event, &timer);
    timer_settime(timer, 0, &itime, NULL);

    //	Collect1 Connect to chid3 and chid5
    result = pthread_create(&collect_t1, NULL, collect1, NULL);
    checkresult(result, "Thread create failed");

    //	Collect2 Connect to chid4 and chid5
    result = pthread_create(&collect_t2, NULL, collect2, NULL);
    checkresult(result, "Thread create failed");

    // Reader1 Connect to chid3
    result = pthread_create(&reader_t1, NULL, reader1, NULL);
    checkresult(result, "Thread create failed");

    // Reader2 Connect to chid4
    result = pthread_create(&reader_t2, NULL, reader2, NULL);
    checkresult(result, "Thread create failed");

    // Reader3 Connect to chid5
    result = pthread_create(&reader_t3, NULL, reader3, NULL);
    checkresult(result, "Thread create failed");

    // register killer to terminate the program
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = killer;

    if (sigaction(SIGALRM, &sa, NULL) < 0)
    {
        printf("Failed to set up signal handler.\n");
        exit(EXIT_FAILURE);
    }

    alarm(RUN_TIME);
}

void join()
{
    int result;

    result = pthread_join(reader_t1, NULL);
    checkresult(result, "Thread join failed");
    result = pthread_join(reader_t2, NULL);
    checkresult(result, "Thread join failed");
    result = pthread_join(reader_t3, NULL);
    checkresult(result, "Thread join failed");

    sleep(1);
}

void clean()
{
    destroyCh(ch1);
    destroyCh(ch2);
    destroyCh(ch3);
    destroyCh(ch4);
}

void *collect1() //Communicate with reader1
{
    int rcvid;

    int conid1 = connect(ch1);
    while (is_running)
    {
        rcvid = MsgReceive(chid1, &msg1, sizeof(msg1), NULL); //wait for message on the channel
        if (rcvid == 0)
        {
            // collect the junk data
            junkData(ch1);

            //send to reader 1
            send(ch1, conid1);
        }
    }
}

void *collect2()
{
    int rcvid;

    int conid2 = connect(ch2);
    while (is_running)
    {
        rcvid = MsgReceive(chid2, &msg2, sizeof(msg2), NULL); //wait for message on the channel
        if (rcvid == 0)
        {
            // collect the junk data
            junkData(ch2);

            // send to reader2
            send(ch2, conid2);
        }
    }
}

void *reader1()
{
	int recvid1;
	connect(ch1);
	int conid3 = connect(ch3);

    while (is_running)
    {
        // receive from collect1
    	recvid1 = recv(ch1);

        printf("Reader1: %s\n", ch1->recv_buffer);

        fwd(ch1, ch3);

        // send to Reader3.. wait for reply
        send(ch3, conid3);

		// Print Message
		// relpy to collect1
        reply(ch1, recvid1);
        counter1++;

    }
}

void *reader2()
{
	int recvid2;
	connect(ch2);
	int conid4 = connect(ch4);

    while (is_running)
    {
        //receive from collect2
        recvid2 = recv(ch2);

        // make lowercase
        lower(ch2);
        printf("Reader2: %s\n", ch2->recv_buffer);


        fwd(ch2, ch4);

        //send to reader3
        send(ch4, conid4);

        // reply to Collect2
        reply(ch2, recvid2);
        counter2++;

    }
}

void *reader3()
{
	connect(ch3);
	connect(ch4);

	int recvid3, recvid4;
    while (is_running)
    {
        if ((recvid3 = recv(ch3))!= -1)
        {
            printf("Reader3: %s\n", ch3->recv_buffer);
            changeColor(ch3);
            printf("Reader3: Changing color based on the stats of the message on channel 3!\n");
            reply(ch3, recvid3);
        }

        if ((recvid4 = recv(ch4)) != -1)
        {
            printf("Reader3: %s\n", ch4->recv_buffer);
            changeColor(ch4);
            printf("Reader3: Changing color based on the stats of the message on channel 4!\n");
            reply(ch4, recvid4);
        }
        counter3++;
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

void *killer()
{
    printf("%s", WHITE);
    printf("Killed!\n");
    is_running = 0;
	printf("Counter1 = %d, Counter2 = %d, Counter3 = %d, Total per second = %d\n",counter1,counter2,counter3,((counter1+counter2+counter3)/RUN_TIME));
    clean();
    exit(EXIT_SUCCESS);
};
