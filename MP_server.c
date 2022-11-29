#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/msg.h>
#include <fcntl.h>
#include "mem.h"
#include <sys/types.h>
#include <sys/ipc.h>

#define MAXOBN 1024 
#define CONNECT 327687+1

typedef struct MessageType{
	long mtype;
	char data[MAXOBN];
	long source;
}Message_t;

typedef struct DataMessage{
	long mtype;
	struct data_buf* data;
}Message_d;

typedef struct Room{
	long clients[2];
}Room;

data_buf* mem1;
data_buf* mem2;
data_buf* mem;
Room* room;
key_t mqid;

void initMemoryData();
void recieveConnection();
void* waitingRoomDataCommunication();
void* gameRoomDataCommunication();
void initque();
void* judgeOmok(void* turn);
void setReadydata();

int main(){
	room->clients[0] = room->clients[1] = 0;

	pthread_t waiting_thread;
	pthread_t game_thread;
	
	initque();
	initMemoryData();
	recieveConnection();	

	pthread_create(&waiting_thread, NULL, waitingRoomDataCommunication, NULL);
	
	recieveConnection();

	pthread_join(waiting_thread, NULL);
	
	//pthread_create(&game_thread, NULL, gameRoomDataCommunication, NULL);
        //pthread_join(game_thread, NULL);

	sleep(5);

}

void initque(){
	mqid = msgget((key_t)60109, IPC_CREAT | 0666);
	if(mqid == -1){
		printf("error\n");
		exit(0);
	}
}

void initMemoryData(){
	mem1->wait_msg.connect = 0;
	mem1->wait_msg.ready = 0;
	mem1->wait_msg.status_change = 0;
	mem1->wait_msg.opponent_connect = 0;
	mem1->wait_msg.opponent_ready = 0;
	mem1->wait_msg.opponent_change = 0;

	mem1->game_msg.my_turn = 0;
	mem1->game_msg.result = 0;
	mem1->game_msg.row = 0;
	mem1->game_msg.col = 0;
	mem1->game_msg.turn_end = 0;

	mem2->wait_msg.connect = 0;
        mem2->wait_msg.ready = 0;
        mem2->wait_msg.status_change = 0;
        mem2->wait_msg.opponent_connect = 0;
        mem2->wait_msg.opponent_ready = 0;
        mem2->wait_msg.opponent_change = 0;

	mem2->game_msg.my_turn = 0;
        mem2->game_msg.result = 0;
	mem2->game_msg.row = 0;
        mem2->game_msg.col = 0;
        mem2->game_msg.turn_end = 0;

	for(int i = 0; i < ROW; i++){
		for(int k = 0; k < COLUMN; k++){
			mem1->game_msg.omok_board[i][k] = '+';
			mem2->game_msg.omok_board[i][k] = '+';
		}
	}
}

void recieveConnection(){
	Message_t message;
	if((msgrcv(mqid, &message, sizeof(message) - sizeof(long), CONNECT, 0))<0){
		printf("error\n");
	}
	else{
		if(room->clients[0]==0)
			room->clients[0] = message.source;
		else
			room->clients[1] = message.source;
	}
}

void recieveData(){
	Message_d message;
	if((msgrcv(mqid, &message, sizeof(message) - sizeof(long), CONNECT, 0))<0){
		printf("error\n");
	}
	else{
		mem = message.data;
	}
}

void setReadydata(){
	mem1->wait_msg.opponent_connect = mem2->wait_msg.connect;
        mem1->wait_msg.opponent_ready = mem2->wait_msg.ready;
        mem1->wait_msg.opponent_change = mem2->wait_msg.status_change;
	
	mem2->wait_msg.opponent_connect = mem1->wait_msg.connect;
        mem2->wait_msg.opponent_ready = mem1->wait_msg.ready;
        mem2->wait_msg.opponent_change = mem1->wait_msg.status_change;

}

void* waitingRoomDataCommunication(){
	int player1_ready = 0;
	int player2_ready = 0;
	double accum;
	while(player1_ready == 0 || player2_ready == 0){
		
		if(mem1->wait_msg.status_change == 1){
			if(mem1->wait_msg.connect == 1){
				mem2->wait_msg.opponent_change = 1;
				mem2->wait_msg.opponent_connect = 1;
				setReadydata();
			}
			else if(mem1->wait_msg.connect == 0){
				mem2->wait_msg.opponent_change = 1;
				mem2->wait_msg.opponent_connect = 0;
				setReadydata();
			}
			if(player1_ready == 0 && mem1->wait_msg.ready == 1){
				mem2->wait_msg.opponent_change = 1;
				mem2->wait_msg.opponent_ready = 1;
				setReadydata();

				player2_ready = 1;
			}
			mem2->wait_msg.status_change = 0;
		}
	}

	pthread_exit(NULL);
}
