#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/time.h>
#include<time.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_CLIENTS	10
#define BUFLEN 4000
#define BUFFILE 1024
#define NAMELEN 255
#define TRUE 1
#define FALSE 0
#define LOG 0
#define STATUS 1
#define KICK 2
#define QUIT 3
#define LIST_CLIENTS 4
#define INFO_CLIENT 5
#define MESSAGE 6
#define BROADCAST 7
#define SEND_FILE 8
#define HISTORY 9
#define ACCEPT 10
#define REJECT 11

/*structura care retine 
informatii despre client*/
typedef struct {
	//true daca numele clientului este setat
	int has_name;
	//socket-ul clientului
	int socket;
	//adresa IP a clientului
	struct in_addr ip;
	//port-ul clientului
	int port;
	//retine timpul conectarii
	double log_time;
	//numele clientului
	char name[255];

} info_clients;

void error(char*);

int findByName(info_clients*, char*);

int findbySocket(info_clients*, int);

int findFree(info_clients*);

int getCommand(char*);

#endif
