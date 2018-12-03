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
volatile int counter1 = 0;
volatile int counter2 = 0;
volatile int counter3= 0;

// TODO add control c sig handler that changes the font back to white once the program is termninated
// TODO set an alarm to kill the threads once it expires. Maybe use a global is_running var to replace the while(1)s in the threads

#define RUN_TIME 27
#define SIZE 1
#define ENV_SIZE 10

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
Environment *env1;
Environment *env2;
Environment *env3;
Environment *env4;

volatile unsigned counter;

int N = 0;
int chid1, chid2;
int is_running = 1;
int nbytes;
void init();
void join();
void clean();

const struct sigevent *isr(void *area, int id);
void * killer();
void *collect1();
void *collect2();
void *reader1();
void *reader2();
void *reader3();

void checkresult(int result, char *text);

int main() {
	init();
	join();
	clean();
}

void init() {

	// Create environments
	env1 = createEnv(ENV_SIZE);
	env2 = createEnv(ENV_SIZE);
	env3 = createEnv(ENV_SIZE);
	env4 = createEnv(ENV_SIZE);

	int id;
	int result;
	timer_t timer;

	struct sigevent event, event2;
	struct itimerspec itime;


	// Requensting IO privileges
	result = ThreadCtl(_NTO_TCTL_IO, 0);

	// setup timer interval for 0.5 seconds
	itime.it_interval.tv_sec = 0;
	itime.it_interval.tv_nsec = (int) 5e6;
	itime.it_value.tv_sec = 0;
	itime.it_value.tv_nsec = (int) 5e6;

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


	//	Collect1
	result = pthread_create(&collect_t1, NULL, collect1, NULL);
	checkresult(result, "Thread create failed");

	//	Collect2
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

	// register killer to terminate the program
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = killer;

	if(sigaction(SIGALRM, &sa, NULL) < 0){
		printf("Failed to set up signal handler.\n");
		exit(EXIT_FAILURE);
	}

	alarm(RUN_TIME);

}

void join() {
	int result;

	result = pthread_join(reader_t1, NULL);
	checkresult(result, "Thread join failed");
	result = pthread_join(reader_t2, NULL);
	checkresult(result, "Thread join failed");
	result = pthread_join(reader_t3, NULL);
	checkresult(result, "Thread join failed");

	sleep(1);
}

void clean() {
	destroyEnv(env1);
	destroyEnv(env2);
	destroyEnv(env3);
	destroyEnv(env4);
}

void *collect1() {
	int rcvid;
	while (is_running) {

		rcvid = MsgReceive(chid1, &msg1, sizeof(msg1), NULL); //wait for message on the channel
		if (rcvid == 0) {
			if (checkFlags(env1)) {
				safeWait(env1);

				// collect the junk data
				junkData(env1);

				// set the read flag
				env1->rflag = 0;

				safePost(env1);
			}


		}
	}
}

void *collect2() {
	static char fill = 'A';
	int rcvid;

	while (is_running) {
		rcvid = MsgReceive(chid2, &msg2, sizeof(msg2), NULL); //wait for message on the channel
		if (rcvid == 0) {
			if (checkFlags(env2)) {
				safeWait(env2);

				// collect the junk data
				junkData(env2);

				// set the read flag
				env2->rflag = 0;

				safePost(env2);
			}
		}
	}
}

void *reader1() {
	char val;
	while (is_running) {
		if (!checkFlags(env1)) {
			safeWait(env1);

			printf("Reader1: %s\n", env1->data);
			fwd(env1,env3);
			if (checkFlags(env3)) {
				safeWait(env3);
				// increment the rflag
				env3->rflag=0;
				safePost(env3);
			}

			env1->rflag = 1;

			safePost(env1);
		}
		counter1++;
	}
}

void *reader2() {
	while (is_running) {
		if (!checkFlags(env2)) {
			safeWait(env2);
			lower(env2);
			printf("Reader2: %s\n", env2->data);
			fwd(env2,env4);
			if (checkFlags(env4) == 1) {
				safeWait(env4);
				// increment the rflag
				env4->rflag=0;
				safePost(env4);
			}
			env2->rflag = 1;
			safePost(env2);
		}
		// TODO instead of sleeping, we need to wake up this thread from a timer event
		counter2++;
	}
}

void *reader3() {
	while (is_running) {
		if (!checkFlags(env3)) {
			safeWait(env3);

			changeColor(env3);
			printf(
					"Reader3: Changing color based on the Collect1 of the environment!\n");
			printf("Reader3: %s\n", env3->data);
			env3->rflag = 1;

			safePost(env3);
		}
		if(!checkFlags(env4)){
			safeWait(env4);

			changeColor(env4);
			printf(
					"Reader3: Changing color based on the Collect2 of the environment!\n");
			printf("Reader3: %s\n", env4->data);
			env4->rflag = 1;

			safePost(env4);
		}
		counter3++;
	}
}

void checkresult(int result, char *text) {
	if (result == -1) {
		perror(text);
		exit(1);
	}
}

void * killer(){
	printf("Killed!\n");
	is_running = 0;
	printf("Counter1 = %d, Counter2 = %d, Counter3 = %d, Total per second = %d",counter1,counter2,counter3,((counter1+counter2+counter3)/RUN_TIME));
    printf("%s", WHITE);

};


// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.prog%2Ftopic%2Finthandler_Attaching.html
// http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fclockperiod.html
