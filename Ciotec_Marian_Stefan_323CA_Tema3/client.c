#include "utils.h"

int main(int argc, char *argv[]) {
    	int sock_server, sock_client, sock_send, n, portno, i, j, pos,
    		 d, command, shutdown, no_of_clients, clilen, newsockfd,
    		 sock_file_send, sock_file_recv, fd_send, fd_recv, count,
    		 sending_file, receiving_file, no_of_messages;
    	struct sockaddr_in serv_addr, my_addr, cli_addr;
    	struct hostent *server;
    	struct in_addr client_ip;
    	char name[NAMELEN], buffer[BUFLEN], *comm, comm_to_server[BUFLEN],
    		 *next, msg[BUFLEN], fileToSend[NAMELEN], fileToRecv[NAMELEN],
    		 fileBuffer[BUFFILE];
    	char history[1000][BUFLEN];//retine istoricul
    	fd_set read_fds;	//multimea de citire folosita in select()
    	fd_set tmp_fds;	//multime folosita temporar 
   	int fdmax;		//valoare maxima file descriptor din multimea read_fds
   	struct timeval timeout, tim;
   	time_t current_time;
	struct tm * time_info;
	char timeString[9];
	double t1, t2;
	 
     
    	if (argc < 5) {
      	fprintf(stderr,"Usage %s client_name client_port server_address server_port\n", argv[0]);
      	exit(0);
    	}
  	
	//golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
	FD_ZERO(&read_fds);
     FD_ZERO(&tmp_fds);
 
 	//socket-ul pe care se comunica cu serverul
	sock_server = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_server < 0) 
        error("ERROR opening socket");
        
    	//retin datele serverului
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[4]));
	inet_aton(argv[3], &serv_addr.sin_addr);
	
	//retin datele mele
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(argv[2]));
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
    	//ma conectez la server
    	if (connect(sock_server,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    	//trimit mesaj serverului cu numele si portul
    	memset(buffer, 0, BUFLEN);
    	sprintf(buffer, "log %s %d", argv[1], atoi(argv[2]));
    	//numele poate avea maxim 255 caractere
    	if(strlen(argv[1]) > NAMELEN) {
    		sprintf(buffer, "Name must be maximum %d characters long", NAMELEN); 
    		error(buffer);
    	}
    	n = send(sock_server, buffer, strlen(buffer), 0);
    	if (n < 0) 
        	error("ERROR writing to socket");
     //socket-ul pe care se realizeaza comunicarea cu ceilalti clienti
     sock_client = socket(AF_INET, SOCK_STREAM, 0);
     if (sock_client < 0) 
        error("ERROR opening socket");
        
     if (bind(sock_client, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) < 0) 
              error("ERROR on binding");
     
     listen(sock_client, MAX_CLIENTS);
     
     //adaug filedescriptorii in read_fds
     FD_SET(sock_client, &read_fds);
     FD_SET(sock_server, &read_fds);
     FD_SET(0, &read_fds);
     if(sock_client > sock_server) 
     	fdmax = sock_client;
     else
     	fdmax = sock_server;
     
     //setez un timeout de 0.001 ms
     timeout.tv_sec = 0;
     timeout.tv_usec = 1;

	no_of_messages = 0;//retine numarul mesajelor primite
	shutdown = FALSE;
	sending_file = FALSE;
	receiving_file = FALSE;
	sock_file_send = -1;
	sock_file_recv = -1;
	
   	while(1) {
   		
   		//serverul a inchis conexiunea
   		if(shutdown == TRUE)
   			break;
   			
   		tmp_fds = read_fds;
   		d = select(fdmax + 1, &tmp_fds, NULL, NULL, &timeout);
   		
   		if(d < 0) {
   			error("ERROR in select");
   		}
   		//s-a iesit din timeout
   		if(d == 0) {
   			if(sending_file == TRUE) {
   				memset(buffer, 0, BUFLEN);
   				for(i = 0; i < 2; i++) {
	   				if((count = read(fd_send, buffer, BUFFILE)) > 0) {
	   					n = send(sock_file_send, buffer, count, 0);
						if (n < 0) 
							error("ERROR writing to socket");
					}
					//s-a terminat fisierul de trimis
					else {
						 printf("File sent\n");
	   					 sending_file = FALSE;
	   					 close(sock_file_send);
	   					 close(fd_send);
	   				}
	   			}
   			}
   		}
   		else {
   			for(i = 0; i <= fdmax; i++) {
   			
   				if(FD_ISSET(i, &tmp_fds)) {
   					//s-a primit o comanda de la tastatura
	   				if(i == 0) {
	   					memset(buffer, 0, BUFLEN);
						if (fgets(buffer, BUFLEN-1, stdin) == NULL)
							error("ERROR reading from stdin");
						if(buffer[strlen(buffer) - 1] == '\n')
							buffer[strlen(buffer) - 1] = '\0';
						
						memset(comm_to_server, 0, BUFLEN);	
						memcpy(comm_to_server, buffer, strlen(buffer));
						comm = strtok(buffer, " ");
						command = getCommand(comm);
						if((command < 3) || (command > 9)) {
							printf("Invalid command\n");
						}
						else {
							if((command != HISTORY) && (command != QUIT)) {
								//se trimite comanda serverului pentru obtinerea de informatii
								//despre destinatar
								n = send(sock_server, comm_to_server, strlen(comm_to_server), 0);
								if (n < 0) 
										error("ERROR writing to socket");
							}
							else if(command == HISTORY) {
								for(j = 0; j < no_of_messages; j++) {
									printf("%s\n", history[j]);
								}
							}
							else if(command == QUIT) {
								shutdown = TRUE;
								break;
							}
						}
					}
					//daca s-a primit raspunsul de la server
					else if(i == sock_server) {
						memset(buffer, 0, BUFLEN);
						memset(name, 0, NAMELEN);
						if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
							if (n == 0) {
								//conexiunea s-a inchis
								printf("Server hung up\n");
								shutdown = TRUE;
								break;
							} else {
								error("ERROR in recv");
							}
						}
						comm = strtok(buffer, "#");
						command = getCommand(comm);
						
						//serverul a raspuns comenzii
						switch(command) {
							
							case ACCEPT:
								break;
								
							case REJECT:
								shutdown = TRUE;
								break;
								
							case KICK:
								shutdown = TRUE;
								break;
								
							case INFO_CLIENT:
								next = strtok(NULL, "#");
								memcpy(name, next, strlen(next));
								//daca clientul n-a fost gasit
								next = strtok(NULL, "#");
								if(strcmp(next, "not_found") == 0) {
									printf("%s not found\n", name);
								}
								else {
									portno = atoi(next);
									next = strtok(NULL, "#");
									t1 = atof(next);
									gettimeofday(&tim, NULL);
									t2 = tim.tv_sec+(tim.tv_usec/1000000.0);
									printf("%s port %d time_connected %.2f sec\n", name, portno, t2-t1);
								}
								break;
								
							case LIST_CLIENTS:
								next = strtok(NULL, "#");
								while(next != NULL) {
									printf("%s\n", next);
									next = strtok(NULL, "#");
								}
								break;
								
							case MESSAGE:
								next = strtok(NULL, "#");
								memcpy(name, next, strlen(next));
								//daca clientul n-a fost gasit
								next = strtok(NULL, "#");
								if(strcmp(next, "not_found") == 0) {
									printf("%s not found\n", name);
								}
								else {
									memset(msg, 0, BUFLEN);
									sprintf(msg, "%s#", argv[1]);
									pos = strlen(msg);
									memset(&cli_addr, 0, sizeof(struct sockaddr_in));
									memcpy(msg + pos, next, strlen(next));
									cli_addr.sin_family = AF_INET;
									next = strtok(NULL, "#");
									inet_aton(next, &cli_addr.sin_addr);
									next = strtok(NULL, "#");
									cli_addr.sin_port = htons(atoi(next));
									//creez un nou socket pe care trimit mesajul
									sock_send = socket(AF_INET, SOCK_STREAM, 0);
									if(sock_send < 0)
										error("ERROR opening socket");
									//ma conectez la client
    									if (connect(sock_send,(struct sockaddr*) &cli_addr,sizeof(cli_addr)) < 0) 
        									error("ERROR connecting");
        								//trimit mesajul clientului
        								n = send(sock_send, msg, strlen(msg), 0);
								    	if (n < 0) 
									   	error("ERROR writing to socket");
									close(sock_send);
								}
								break;
								
							case BROADCAST:
								next = strtok(NULL, "#");
								memset(msg, 0, BUFLEN);
								sprintf(msg, "%s#", argv[1]);
								pos = strlen(msg);
								memcpy(msg + pos, next, strlen(next));
								while(1) {
									memset(&cli_addr, 0, sizeof(struct sockaddr_in));
									cli_addr.sin_family = AF_INET;
									next = strtok(NULL, "#");
									if(next == NULL)
										break;
									inet_aton(next, &cli_addr.sin_addr);
									next = strtok(NULL, "#");
									if(next == NULL)
										break;
									cli_addr.sin_port = htons(atoi(next));
									//creez un nou socket pe care trimit mesajul
									sock_send = socket(AF_INET, SOCK_STREAM, 0);
									if(sock_send < 0)
										error("ERROR opening socket");
									//ma conectez la client
    									if (connect(sock_send,(struct sockaddr*) &cli_addr,sizeof(cli_addr)) < 0) 
        									error("ERROR connecting");
        								//trimit mesajul clientului
        								n = send(sock_send, msg, strlen(msg), 0);
								    	if (n < 0) 
									   	error("ERROR writing to socket");
									close(sock_send);
								}
								break;
								
								case SEND_FILE:
									memset(fileToSend, 0, NAMELEN);
									next = strtok(NULL, "#");
									memcpy(name, next, strlen(next));
									next = strtok(NULL, "#");
									memcpy(fileToSend, next, strlen(next));
									next = strtok(NULL, "#");
									if(strcmp(next, "not_found") == 0) {
										printf("%s not found\n", name);
									}
									else {
										memset(&cli_addr, 0, sizeof(struct sockaddr_in));
										cli_addr.sin_family = AF_INET;
										inet_aton(next, &cli_addr.sin_addr);
										next = strtok(NULL, "#");
										cli_addr.sin_port = htons(atoi(next));
										memset(buffer, 0, BUFLEN);
										sprintf(buffer, "sendfile#%s#", fileToSend);
										/*creez un nou socket pe care trimit fisierul
										acest socket ramane deschis pana la trimiterea
										fisierului sau deconectarea clientului destinatie*/
										sock_file_send = socket(AF_INET, SOCK_STREAM, 0);
										if(sock_file_send < 0)
											error("ERROR opening socket");
										//ma conectez la client
	    									if (connect(sock_file_send,(struct sockaddr*) &cli_addr,sizeof(cli_addr)) < 0) 
		   									error("ERROR connecting");
		   								//trimit mesajul clientului
		   								n = send(sock_file_send, buffer, BUFFILE, 0);
		   								//deschid fisierul pentru citire
		   								fd_send = open(fileToSend, O_RDONLY);
		   								sending_file = TRUE;
									    	if (n < 0) 
										   	error("ERROR writing to socket");
									}	
									break;
							}
						}
						//a venit o noua conexiune din partea unui alt client
						else if(i == sock_client) {
							clilen = sizeof(cli_addr);
							memset(&cli_addr, 0, clilen);
							if ((newsockfd = accept(sock_client, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
								error("ERROR in accept");
							} 
							else {
								//adaug noul socket intors de accept() la multimea descriptorilor de citire
								FD_SET(newsockfd, &read_fds);
								if (newsockfd > fdmax) { 
									fdmax = newsockfd;
								}
							}
						}
						else {
							// am primit date pe unul din socketii cu care vorbesc cu clientii
							//actiunea clientului: recv()
							memset(fileBuffer, 0, BUFFILE);
							memset(msg, 0, BUFLEN);
							memset(name, 0, NAMELEN);
							memset(comm, 0, 20);
							if ((n = recv(i, fileBuffer, BUFFILE, 0)) <= 0) {
								if(n < 0) {
									error("ERROR in recv");
								}
								if(i == sock_file_recv) {
									receiving_file = FALSE;
									sock_file_recv = -1;
									close(fd_recv);
								}
								close(i); 
								FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
							}
							else if((receiving_file == TRUE) && (i == sock_file_recv)) {
								write(fd_recv, fileBuffer, n);
							}
								
					
							else { //recv intoarce >0
								next = strtok(fileBuffer, "#");
								command = getCommand(next);
								if(command == SEND_FILE) {
									memset(fileToRecv, 0, NAMELEN);
									//pe acest socket primesc fisierul
									sock_file_recv = i;
									next = strtok(NULL, "#");
									memcpy(name, next, strlen(next));
									sprintf(fileToRecv, "%s_primit", name);
									memcpy(history[no_of_messages], fileToRecv, strlen(fileToRecv));
									no_of_messages++;
									fd_recv = open(fileToRecv, O_CREAT | O_WRONLY | O_TRUNC, 0644);
									receiving_file = TRUE;
								}
								else {
									memcpy(name, next, strlen(next));
									next = strtok(NULL, "#");
									memcpy(msg, next, strlen(next));
									time(&current_time);
									time_info = localtime(&current_time);
									strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
									memset(fileBuffer, 0, BUFLEN);
									sprintf(fileBuffer, "[%s][%s] %s", timeString, name, msg);
									memcpy(history[no_of_messages], fileBuffer, strlen(fileBuffer));
									no_of_messages++;
									printf("%s\n", fileBuffer);
								}
							}
						}
					}
				}
			}
		}
		close(sock_client);
		close(sock_server);
							
    return 0;
}


