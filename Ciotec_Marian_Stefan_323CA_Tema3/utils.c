#include "utils.h"

void error(char *msg) {
    perror(msg);
    exit(1);
}

int findByName(info_clients* client, char* name) {
	int i;
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(client[i].has_name == TRUE) {
			if(strcmp(client[i].name, name) == 0) {
				return i;
			}
		}
	}
	return -1;
}

int findBySocket(info_clients* client, int sockfd) {
	int i;
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(client[i].socket == sockfd) {
			return i;
		}
	}
	return -1;
}

int findFree(info_clients* client) {
	int i;
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(client[i].socket == -1) {
			return i;
		}
	}
	return -1;
}

int getCommand(char* command) {

	if(strcmp("log", command) == 0)
		return LOG;
		
	if(strcmp("status", command) == 0) 
		return STATUS;
	
	if(strcmp("kick", command) == 0) 
		return KICK;
	
	if(strcmp("quit", command) == 0) 
		return QUIT;
	
	if(strcmp("listclients", command) == 0) 
		return LIST_CLIENTS;
		
	if(strcmp("infoclient", command) == 0)
		return INFO_CLIENT;
		
	if(strcmp("message", command) == 0)
		return MESSAGE;
		
	if(strcmp("broadcast", command) == 0)
		return BROADCAST;
		
	if(strcmp("sendfile", command) == 0)
		return SEND_FILE;
		
	if(strcmp("history", command) == 0)
		return HISTORY;
		
	if(strcmp("accept", command) == 0)
		return ACCEPT;
		
	if(strcmp("reject", command) == 0)
		return REJECT;
		
	return -1;
}

