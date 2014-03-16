#include "utils.h"

int main(int argc, char* argv[]) {

     int sockfd, newsockfd, portno, clilen, pos;
     char buffer[BUFLEN], aux_buffer[BUFLEN], comm[20], *cli_to_remove, 
     	*next, recv_comm[BUFLEN], name[NAMELEN], message[BUFLEN], file[NAMELEN];
     struct sockaddr_in serv_addr, cli_addr;
     int n, i, j, no_of_clients, free_pos, current, command, shutdown;
     info_clients client[MAX_CLIENTS];
	 
     fd_set read_fds;	//multimea de citire folosita in select()
     fd_set tmp_fds;	//multime folosita temporar 
     int fdmax;		//valoare maxima file descriptor din multimea read_fds
     struct timeval tim;

     if (argc < 2) {
         fprintf(stderr,"Usage : %s port\n", argv[0]);
         exit(1);
     }

	//initial nu este niciun client conectat
	for(i = 0; i < MAX_CLIENTS; i++) {
		client[i].has_name = FALSE;
		client[i].socket = -1;
	}
	no_of_clients = 0;

	//golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
     FD_ZERO(&read_fds);
     FD_ZERO(&tmp_fds);
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     
     portno = atoi(argv[1]);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("ERROR on binding");
     
     listen(sockfd, MAX_CLIENTS);

     //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
     FD_SET(sockfd, &read_fds);
     fdmax = sockfd;
	shutdown = FALSE;
	//adaugam file descriptor-ul pentru stdin
	FD_SET(0, &read_fds);
	 
	// main loop
	while (1) {
		tmp_fds = read_fds;
		
		if(shutdown == TRUE)
			break;
			 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
	
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				//daca serverul a primit comanda de la tastatura
				if(i == 0) {
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN-1, stdin);
					if(buffer[strlen(buffer) - 1] == '\n')
						buffer[strlen(buffer) - 1] = '\0';
					memset(comm, 0, 20);
					next = strtok(buffer, " ");
					memcpy(comm, next, strlen(next));
					command = getCommand(comm);
					if(command == -1) {
						printf("Invalid command\n");
					}
					else {
						if(command == STATUS) {
							//afisez toti clientii existenti
							for(j = 0; j < MAX_CLIENTS; j++) {
								//daca clientul exista si are un nume de identificare
								if((client[j].socket != -1) && (client[j].has_name == TRUE)) {
									printf("%s IP = %s PORT = %d\n", client[j].name,
												 inet_ntoa(client[j].ip), client[j].port);
								}
							}
						}
						if(command == KICK) {
							//gasesc numele clientului de eliminat
							cli_to_remove = strtok(NULL, " ");
							if(cli_to_remove == NULL) {
								printf("Invalid kick command. Command format : kick client_name\n");
							}
							else {
								j = findByName(client, cli_to_remove);
								//daca numele clientului nu exista
								if(j == -1) {
									printf("Client does not exist. Try again\n");
								}
								else {
									memset(buffer, 0, BUFLEN);
									sprintf(buffer, "kick");
									//anunt clientul ca va fi eliminat
									n = send(client[j].socket, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
									//elimin clientul din multimea de file descriptori
									FD_CLR(client[j].socket, &read_fds);
									close(client[j].socket);
									client[j].has_name = FALSE;
									client[j].socket = -1;
								}
							}
						}
						if(command == QUIT) {
							printf("Server will shutdown\n");
							//inchid conexiunile cu clientii
							for(j = 0; j < MAX_CLIENTS; j++) {
								if(client[j].socket != -1)
									close(client[j].socket);
							}
							shutdown = TRUE;
							break;
						}
					}
				}

				else if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						//retin informatiile despre clientul nou conectat
						//severul nu mai permite nicio conectare
						if(no_of_clients == MAX_CLIENTS) {
							printf("Connection denied. Server is full\n");
							break;
						}
						free_pos = findFree(client);
						
						client[free_pos].socket = newsockfd;
						client[free_pos].port = cli_addr.sin_port;
						client[free_pos].ip = cli_addr.sin_addr;
						gettimeofday(&tim, NULL);
             				client[free_pos].log_time = tim.tv_sec+(tim.tv_usec/1000000.0);
						
						FD_SET(newsockfd, &read_fds);
						no_of_clients++;
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("Server: socket %d hung up\n", i);
							current = findBySocket(client, i);
							client[current].has_name = FALSE;
							client[current].socket = -1;
							close(i); 
							FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care
							no_of_clients--; 
						} else {
							error("ERROR in recv");
						}
					} 
					
					else { //recv intoarce >0
						memset(recv_comm, 0, BUFLEN);
						memset(name, 0, NAMELEN);
						memset(comm, 0, 20);
						//salvez comanda
						memcpy(recv_comm, buffer, strlen(buffer));
						next = strtok(buffer, " ");
						memcpy(comm, next, strlen(next));
						command = getCommand(comm);
						
						switch(command) {
						
							case LOG:
								next = strtok(NULL, " ");
								memcpy(name, next, strlen(next));
								next = strtok(NULL, " ");
								portno = atoi(next);
								if(findByName(client, name) != -1) {
									memset(buffer, 0, BUFLEN);
									sprintf(buffer, "reject");
									//anunt clientul ca nu a fost acceptat
									n = send(i, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
									//elimin clientul din multimea de file descriptori
									FD_CLR(i, &read_fds);
									close(i);
									current = findBySocket(client, i);
									client[current].has_name = FALSE;
									client[current].socket = -1;
								}
								else {
									memset(buffer, 0, BUFLEN);
									sprintf(buffer, "accept");
									//anunt clientul ca a fost acceptat
									n = send(i, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
									current = findBySocket(client, i);
									memcpy(client[current].name, name, strlen(name));
									client[current].has_name = TRUE;
									client[current].port = portno;
									
								}
								break;
								
							case LIST_CLIENTS:
								memset(buffer, 0, BUFLEN);
								memcpy(buffer, recv_comm, strlen(recv_comm));
								pos = strlen(recv_comm);
								//copiez numele clientilor in buffer
								for(j = 0; j < MAX_CLIENTS; j++) {
									if((client[j].socket != -1) && (client[j].has_name == TRUE)) {
										memset(aux_buffer, 0, BUFLEN);
										sprintf(aux_buffer, "#%s", client[j].name);
										memcpy(buffer + pos, aux_buffer, strlen(aux_buffer));
										pos += strlen(aux_buffer);
									}
								}
								//trimit clientului lista cu clientii conectati
								n = send(i, buffer, strlen(buffer), 0);
								if (n < 0) 
									error("ERROR writing to socket");
								break;
								
							case INFO_CLIENT:
								next = strtok(NULL, " ");
								memcpy(name, next, strlen(next));
								j = findByName(client, name);
								//nu exista un client cu acest nume
								if(j == -1) {
									memset(buffer, 0, BUFLEN);
									sprintf(buffer, "infoclient#%s#not_found", name);
									n = send(i, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
								}
								else {
									memset(buffer, 0, BUFLEN);
									sprintf(buffer, "infoclient#%s#%d#%f", 
										client[j].name, client[j].port, client[j].log_time);
									n = send(i, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
								}
								break;
								
							case MESSAGE:
								next = strtok(NULL, " ");
								memcpy(name, next, strlen(next));
								j = findByName(client, name);
								memset(buffer, 0, BUFLEN);
								memset(message, 0, BUFLEN);
								//nu exista un client cu acest nume
								if(j == -1) {
									sprintf(buffer, "message %s not_found", name);
									n = send(i, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
								}
								else {
								
									pos = strlen(comm) + strlen(name) + 2;
									memcpy(message, recv_comm + pos, strlen(recv_comm) - pos);
									sprintf(buffer, "%s#%s#%s", comm, name, message);
									pos = strlen(buffer);
									sprintf(buffer + pos, "#%s#%d", inet_ntoa(client[j].ip), client[j].port);
									n = send(i, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
								}
								break;
								
							case BROADCAST:
								memset(message, 0, BUFLEN);
								pos = strlen(comm) + 1;
								//salvez mesajul
								memcpy(message, recv_comm + pos, strlen(recv_comm) - pos);
								memset(buffer, 0, BUFLEN);
								sprintf(buffer, "%s#%s", comm, message);
								pos = strlen(buffer);
								//copiez adresele ip si porturile clientilor
								for(j = 0; j < MAX_CLIENTS; j++) {
									if((client[j].socket != -1) && (client[j].has_name == TRUE) 
														   		&& client[j].socket != i) {
										memset(aux_buffer, 0, BUFLEN);
										sprintf(aux_buffer, "#%s#%d", inet_ntoa(client[j].ip), client[j].port);
										memcpy(buffer + pos, aux_buffer, strlen(aux_buffer));
										pos += strlen(aux_buffer);
									}
								}
								n = send(i, buffer, strlen(buffer), 0);
								if (n < 0) 
									error("ERROR writing to socket");
								break;
								
							case SEND_FILE:
								memset(file, 0, NAMELEN);
								next = strtok(NULL, " ");
								memcpy(name, next, strlen(next));
								next = strtok(NULL, " ");
								memcpy(file, next, strlen(next));
								j = findByName(client, name);
								memset(buffer, 0, BUFLEN);
								if(j == -1) {
									sprintf(buffer, "sendfile %s %s not_found", name, file);
									n = send(i, buffer, strlen(buffer), 0);
									if (n < 0) 
										error("ERROR writing to socket");
								}
								else {
									memset(aux_buffer, 0, BUFLEN);
									sprintf(buffer, "%s#%s#%s", comm, name, file);
									sprintf(aux_buffer, "#%s#%d", inet_ntoa(client[j].ip), client[j].port);
									pos = strlen(buffer);
									memcpy(buffer + pos, aux_buffer, strlen(aux_buffer));
								}
								n = send(i, buffer, strlen(buffer), 0);
								if (n < 0) 
									error("ERROR writing to socket");
								break;
						}
					}	
				} 
			}
		}
     }
     close(sockfd);
   
     return 0; 
}

