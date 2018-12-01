#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/siginfo.h>
#include <signal.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

//The struct below came from the QNX website.
typedef union {
	struct _pulse   pulse;
	// other message structures here
} my_message_t;

//Global variables
int chid, chid2

int main(int argc, char *argv[]) {
	//Create the first timer
	struct sigevent MyEvent, MyEvent2;
	timer_t tid, tid2;
	int coid_Value, coid_Value2;
	
	//must connect the channel to the event before creating timer or it doesn't seem to work
	MyEvent.sigev_notify = SIGEV_PULSE;
	MyEvent.sigev_code = _PULSE_CODE_MINAVAIL;
	MyEvent.sigev_priority = getprio(0);
	//must connect the channel to the event before creating timer or it doesn't seem to work
	MyEvent2.sigev_notify = SIGEV_PULSE;
	MyEvent2.sigev_code = _PULSE_CODE_MINAVAIL+1;
	MyEvent2.sigev_priority = getprio(0);
	
	chid = ChannelCreate(0); //create channel
	chid2 = ChannelCreate(0); //create channel

	coid_Value = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
	MyEvent.sigev_coid = coid_Value; //connect event to channel
	
	coid_Value2 = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
	MyEvent2.sigev_coid = coid_Value2; //connect event to channel
	
	timer_create(CLOCK_REALTIME, &MyEvent, &tid); //create the timer
	timer_create(CLOCK_REALTIME, &MyEvent2, &tid2); //create the timer

	struct itimerspec interval_time, itimerspec interval_time2; //set timer to start in 3 seconds and repeat every 3 seconds thereafter
	interval_time.it_value.tv_sec = 3;
	interval_time.it_value.tv_nsec = 0;
	interval_time.it_interval.tv_sec = 3;
	interval_time.it_interval.tv_nsec = 0;

	interval_time2.it_value.tv_sec = 1;
	interval_time2.it_value.tv_nsec = 500000000;
	interval_time2.it_interval.tv_sec = 3;
	interval_time2.it_interval.tv_nsec = 0;

	timer_settime(tid2, 0, &interval_time2, NULL); //activate timer
	timer_settime(tid, 0, &interval_time, NULL); //activate timer

	int                     rcvid;
	my_message_t            msg;

	// loop below is based on code from the QNX literature on timer pulses.
	for (;;) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) { // got a pulse
			if (msg.pulse.code == _PULSE_CODE_MINAVAIL){
				printf("we got a pulse from timer1\n");
			}
			else if (msg.pulse.code == _PULSE_CODE_MINAVAIL+1){
				printf("we got a pulse from timer2\n");
			}
		}
	}

	return EXIT_SUCCESS;
}


void Reader1(){
	int Temp;
	//access the data here 
	//Temp = data
	printf("%d", Temp);
}

void Reader2(){
	int Temp;
	//access the data here 
	//Temp = data
	printf("%d", Temp);
}

void Reader3(){
	int Temp;
	//access the data here 
	//Temp = data
	printf("%d", Temp);
}

int Access1(){
	
}

int Access2(){
	
}
