//CIOTEC MARIAN-STEFAN 333CA - TEMA 4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"

#define NMAX 10000
#define MAXNODES 100

#define SONDA 0
#define ECOU 1

#define START 0
#define FINAL 1
#define INFO 2
#define BROADCAST 3

//intoarce 1 daca vectorul contine valoarea cautata
int contains(int *v, int size, int value) {
	int i;
	for(i = 0; i < size; i++) {
		if(v[i] == value) {
			return 1;
		}
	}
	return 0;
}

void bubble_sort(int *v, int size) {
	int aux, i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			if(v[i] < v[j]) {
				aux = v[i];
				v[i] = v[j];
				v[j] = aux;
			}
		}
	}
}

struct Message {
	//tipul mesajului
	int type;
	//toplogia
	int topology[NMAX];
	//nodurile vizitate
	int visited[MAXNODES];
};

struct Communication {
	//tipul mesajului
	int type;
	//sursa mesajului
	int source;
	//destinatia mesajului
	int dest;
	/*in cazul mesajelor initialelor retine nodurile care
	  au mai primit acest mesaj */
	int seen[MAXNODES];
	//informatia mesajului
	char payload[100];
};

struct MyQueue {
	int elem[MAXNODES];
	int front;
	int back;
};

int main(int argc, char **argv) {
	int rank, size;
	int i, j, k;
	MPI_Status status;
	MPI_Request request;
	//primul proces de la care s-a primit sondaj
	int prim, transm;
	//vector de legaturi
	int *leg;
	//matricile care retin topologia
	int *top;
	//numarul de ecouri care se asteapta sa fie primit
	int nr_ecouri = -1;
	int visited_nodes[MAXNODES];
	FILE *file_top, *file_msg;
	char line[100];
	char *token;
	MPI_Aint offset[3], info_offset[7];
	MPI_Datatype my_mpi_struct, my_info_struct;
	struct Message mesaj, mesaj_nou;
	struct Communication info, info_nou, info_to_send[MAXNODES];
	int no_to_send = 0, no_of_lines = 0;
	int no_cycle_nodes = 0, *cycle_nodes;
	//numarul de vecini si vecinii din arborele minim de acoperire
	int no_of_neighbors = 0, *neighbors;
	//tabela de rutare - retine hop=ul urmator pana la destinatie
	int *next_hop, info_next_hop;
	
	//init
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	leg = (int*) malloc(size * sizeof(int));
	top = (int*) malloc(size * size * sizeof(int));
	next_hop = (int*) malloc(size * sizeof(int));
	cycle_nodes = (int*) malloc(size * sizeof(int));
	neighbors = (int*) malloc(size * sizeof(int));
	//creare tip nou de date MPI folosit in comunicatia
	//pentru stablirea topologiei in etapa 1
	int blocklen[3] = {1, NMAX, MAXNODES};
	MPI_Datatype oldtypes[3] = {MPI_INT, MPI_INT, MPI_INT};
	offset[0] = (char*)(&mesaj.type) - (char*)(&mesaj);
	offset[1] = (char*)(&mesaj.topology[0]) - (char*)(&mesaj);
	offset[2] = (char*)(&mesaj.visited[0]) - (char*)(&mesaj);
	MPI_Type_create_struct(3, blocklen, offset, oldtypes, &my_mpi_struct);
	MPI_Type_commit(&my_mpi_struct);
	//creare tip nou de date MPI folosit in comunicatia
	//prin mesaje dintre noduri in etapa 2
	int info_blocklen[5] = {1, 1, 1, MAXNODES, 100};
	MPI_Datatype info_oldtypes[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_CHAR};
	info_offset[0] = (char*)(&info.type) - (char*)(&info);
	info_offset[1] = (char*)(&info.source) - (char*)(&info);
	info_offset[2] = (char*)(&info.dest) - (char*)(&info);
	info_offset[3] = (char*)(&info.seen[0]) - (char*)(&info);
	info_offset[4] = (char*)(&info.payload[0]) - (char*)(&info);
	MPI_Type_create_struct(5, info_blocklen, info_offset, info_oldtypes,
				 &my_info_struct);
	MPI_Type_commit(&my_info_struct);
	
	for(i = 0; i < size; i++) {
		leg[i] = 0;
	}
	for(i = 0; i < size * size; i++) {
		top[i] = 0;
		mesaj.topology[i] = 0;
	}
	mesaj.topology[0] = 10;
	mesaj.type = 0;
	if((file_top = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Process %d cannot open input file %s\n", 
			rank, argv[1]);
		exit(1);
	}
	//cititre din fisier topologie
	i = 0;
	while(fgets(line, 100, file_top) != NULL) {
		if(i == rank) {
			break;
		}
		i++;
	}
	token = strtok(line, " ");
	while(token) {
		i = atoi(token);
		if(i != rank) {
			leg[i] = 1;
			nr_ecouri++;
			top[rank * size + i] = 1;
			top[i * size + rank] = 1;
		}
		token = strtok(NULL, " ");
	}
	top[rank * size + rank] = 1;
	/**********************************************************************
				      Etapa 1
				Stabilirea topologiei
	**********************************************************************/
	
	/*In urma acestui pas nodul 0 va avea imaginea completa a topologiei*/
		
	//Nodul 0 este initiatorul
	if(rank == 0) {
		//trimit sonda nodurilor vecine
		mesaj.type = SONDA;
		for(i = 0; i < size; i++) {
			if(leg[i]) {
				MPI_Send(&mesaj, 1, my_mpi_struct, i, 0, 
					 MPI_COMM_WORLD);
			}
		}
		//primesc mesajele ecou finale
		for(i = 0; i < size; i++) {
			if(leg[i]) {
				MPI_Recv(&mesaj_nou, 1, my_mpi_struct, i, 0,
					MPI_COMM_WORLD, &status);
				for(j = 0; j < size * size; j++) {
					top[j] = top[j] | mesaj_nou.topology[j];
				}
			}
		}
		
	}
	else {

		MPI_Recv(&mesaj_nou, 1, my_mpi_struct, MPI_ANY_SOURCE, 0, 
			 MPI_COMM_WORLD, &status);
		prim = status.MPI_SOURCE;
		//trimit sonda celorlalte noduri vecine in afara de prim
		mesaj.type = SONDA;
		for(i = 0; i < size; i++) {
			if(leg[i] && (i != prim)) {
				MPI_Isend(&mesaj, 1, my_mpi_struct, i, 0, 
					   MPI_COMM_WORLD, &request);
				
			}
		}
		while(nr_ecouri > 0) {
			MPI_Recv(&mesaj_nou, 1, my_mpi_struct, MPI_ANY_SOURCE,
				 0, MPI_COMM_WORLD, &status);
			transm = status.MPI_SOURCE;
			if(mesaj_nou.type == SONDA) {
				cycle_nodes[no_cycle_nodes] = transm;
				no_cycle_nodes++;
				mesaj.type = ECOU;
				for(i = 0; i < size * size; i++) {
					mesaj.topology[i] = 0;
				}
				MPI_Isend(&mesaj, 1, my_mpi_struct, transm, 0,
					  MPI_COMM_WORLD, &request);
			}
			else if(mesaj_nou.type == ECOU) {
				nr_ecouri--;
				for(j = 0; j < size * size; j++) {
					top[j] = top[j] | mesaj_nou.topology[j];
				}
				
			}
			
		}
		mesaj.type = ECOU;
		for(i = 0; i < size * size; i++) {
			mesaj.topology[i] = top[i];
		}
		MPI_Send(&mesaj, 1, my_mpi_struct, prim, 0, MPI_COMM_WORLD);
	}
	/*In urma acestui pas toate nodurile vor avea imaginea completa a topologiei*/
	
	//Nodul 0 este initiatorul
	if(rank == 0) {
		mesaj.type = ECOU;
		for(i = 0; i < size * size; i++) {
			mesaj.topology[i] = top[i];
		}
		mesaj.visited[0] = 1;
		//marchez si unde va ajunge mesajul
		for(i = 1; i < size; i++) {
			if(leg[i]) {
				mesaj.visited[i] = 1;
			}
		}
		for(i = 1; i < size; i++) {
			if(leg[i]) {
				MPI_Send(&mesaj, 1, my_mpi_struct, i,
					 0, MPI_COMM_WORLD);
			}
		}
	}
	else {
		MPI_Recv(&mesaj_nou, 1, my_mpi_struct, MPI_ANY_SOURCE, 0,
			 MPI_COMM_WORLD, &status);
		for(j = 0; j < size * size; j++) {
			top[j] = top[j] | mesaj_nou.topology[j];
		}
		mesaj.type = ECOU;
		for(i = 0; i < size; i++) {
			visited_nodes[i] = mesaj_nou.visited[i];
			mesaj.visited[i] = mesaj_nou.visited[i];
		}
		mesaj.visited[rank] = 1;
		for(i = 0; i < size; i++) {
			if(leg[i]) {
				mesaj.visited[i] = 1;
			}
		}
		for(i = 0; i < size * size; i++) {
			mesaj.topology[i] = top[i];
		}
		for(i = 0; i < size; i++) {
			if(leg[i] && (visited_nodes[i] == 0)) {
				MPI_Send(&mesaj, 1, my_mpi_struct, i,
					 0, MPI_COMM_WORLD);
			}
		}
	}
		
	//calcul tabela de rutare
	struct MyQueue queue;
	int *marked = (int*) malloc(size * sizeof(int));
	int *parent = (int*) malloc(size * sizeof(int));
	int *path = (int*) malloc(size * sizeof(int));
	int source = rank;
	int node, dest;
	source = rank;
	queue.front = 0;
	queue.back = 0;
	for(j = 0; j < size; j++) {
		marked[j] = 0;
		parent[j] = -1;
	}
	queue.elem[queue.back] = source;
	queue.back++;
	marked[source] = 1;
	while(queue.front < queue.back) {
		node = queue.elem[queue.front];
		queue.front++;
		for(j = 0; j < size; j++) {
			if((j != source) && (top[(node*size)+j] == 1)) {
				if(!marked[j]) {
					marked[j] = 1;
					parent[j] = node;
					queue.elem[queue.back] = j;
					queue.back++;
				}
			}
		}
	}
	next_hop[source] = source;
	for(j = 0; j < size; j++) {
		dest = j;
		if(dest != source) {
			k = 0;
			while(parent[dest] != -1) {
				path[k] = dest;
				k++;
				dest = parent[dest];
			}
			next_hop[j] = path[k-1];
		}
	}
	//afisare
	for(k = 0; k < size; k++) {
		MPI_Barrier(MPI_COMM_WORLD);
		if(k == rank) {
			printf("ID %d\n", rank);
			if(rank == 0) {
				for(j = 0; j < size * size; j++) {
					printf("%d ", top[j]);
					if(((j + 1) % size) == 0) {
						printf("\n");
					}
				}
			}
			for(i = 0; i < size; i++) {
				printf("%d %d\n", i, next_hop[i]);
			}
			usleep(10000);
		}
	}
	/**********************************************************************
					ETAPA 2
				COMUNICATIE INTRE PROCESE
	***********************************************************************/
	
	//citire din fisierul de mesaje
	if((file_msg = fopen(argv[2], "r")) == NULL) {
		fprintf(stderr, "Process %d cannot open input file %s\n", 
			rank, argv[2]);
		exit(1);
	}
	fgets(line, 100, file_msg);
	no_of_lines = atoi(line);
	for(i = 0; i < no_of_lines; i++) {
		fgets(line, 100, file_msg);
		token = strtok(line, " ");
		j = atoi(token);
		if(rank == j) {
			info_to_send[no_to_send].source = j;
			token = strtok(NULL, " ");
			if(strcmp(token, "B") == 0) {
				info_to_send[no_to_send].type = BROADCAST;
				info_to_send[no_to_send].dest = BROADCAST;
			}
			else {
				info_to_send[no_to_send].type = INFO;
				info_to_send[no_to_send].dest = atoi(token);
			}
			token = strtok(NULL, "\n");
			strcpy(info_to_send[no_to_send].payload, token);
			no_to_send++;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	/**********Initierea comunicarii cu mesaje de broadcast***************/
	
	info.type = START;
	info.source = rank;
	info.dest = BROADCAST;
	for(i = 0; i < size; i++) {
		if(i != rank) {
			if(leg[i]) {
				//mesajul urmeaza sa fie vazut de legaturile 
				//procesului curent
				info.seen[i] = 1;
			}
			else {
				info.seen[i] = 0;
			}
		}
		else {
			//mesajul a fost vazut de rank-ul curent
			info.seen[i] = 1;
		}
	}
	//trimit mesajul de start tuturor legaturilor directe
	for(i = 0; i < size; i++) {
		if(leg[i]) {
			MPI_Isend(&info, 1, my_info_struct, i, 0,
				  MPI_COMM_WORLD, &request);
		}
	}
	//astept size - 1 mesaje de start din partea celorlalte noduri
	for(i = 0; i < size - 1; i++) {
		MPI_Recv(&info_nou, 1, my_info_struct, MPI_ANY_SOURCE,
			 0, MPI_COMM_WORLD, &status);
		info.type = info_nou.type;
		info.source = info_nou.source;
		info.dest = info_nou.dest;
		for(j = 0; j < size; j++) {
			visited_nodes[j] = info_nou.seen[j];
			info.seen[j] = info_nou.seen[j];
			if(leg[j]) {
				info.seen[j] = 1;
			}
		}
		info.seen[rank] = 1;
		for(j = 0; j < size; j++) {
			//daca mesajul nu a mai fost vazut 
			//de acel nod legatura i-l trimit
			if(leg[j] && (!visited_nodes[j])) {
				MPI_Isend(&info, 1, my_info_struct, j,
					  0, MPI_COMM_WORLD, &request);
			}
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	/**********************************************************************/
	//incepe comunicarea propriu-zisa intre procese
	//fiecare proces trimite mesajele proprii la next_hop
	for(i = 0; i < no_to_send; i++) {
		if(info_to_send[i].type == INFO) {
			info_next_hop = 
				next_hop[info_to_send[i].dest];
			MPI_Isend(&info_to_send[i], 1, my_info_struct,
				  info_next_hop, 0, MPI_COMM_WORLD, &request);
		}
		else if(info_to_send[i].type == BROADCAST) {
			for(j = 0; j < size; j++) {
				if(j != rank) {
					if(leg[j]) {
						info_to_send[i].seen[j] = 1;
					}
					else {
						info_to_send[i].seen[j] = 0;
					}
				}
				else {
					info_to_send[i].seen[j] = 1;
				}
			}
			//trimit mesajul de start tuturor legaturilor directe
			for(k = 0; k < size; k++) {
				if(leg[k]) {
					MPI_Isend(&info_to_send[i], 1, my_info_struct,
						  k, 0, MPI_COMM_WORLD, &request);
				}
			}
		}
		
	}
	//anunt toate nodurile ca am terminat de trimis toate mesajele
	info.type = FINAL;
	info.source = rank;
	info.dest = BROADCAST;
	for(i = 0; i < size; i++) {
		if(i != rank) {
			if(leg[i]) {
				info.seen[i] = 1;
			}
			else {
				info.seen[i] = 0;
			}
		}
		else {
			info.seen[i] = 1;
		}
	}
	for(i = 0; i < size; i++) {
		if(leg[i]) {
			MPI_Isend(&info, 1, my_info_struct, i, 0,
				  MPI_COMM_WORLD, &request);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	//astept sa primesc mesaje de la celelalte procese
	//cand primesc din partea tutoror celorlalte noduri mesaje de final
	//inseamna ca s-a incheiat comunicatia si etapa 2
	int no_of_final_msg = 0;
	while(no_of_final_msg < (size - 1)) {
		MPI_Recv(&info_nou, 1, my_info_struct, MPI_ANY_SOURCE, 0,
			  MPI_COMM_WORLD, &status);
		transm = status.MPI_SOURCE;
		if((info_nou.type == FINAL) || (info_nou.type == BROADCAST)) {
			if(info_nou.type == FINAL) {
				no_of_final_msg++;
			}
			else if(info_nou.type == BROADCAST) {
				strcpy(info.payload, info_nou.payload);
				printf("BROADCAST ID %d %s from %d source %d\n", 
					rank, info_nou.payload, transm, info_nou.source);
			}
			info.type = info_nou.type;
			info.source = info_nou.source;
			info.dest = info_nou.dest;
			for(j = 0; j < size; j++) {
				visited_nodes[j] = info_nou.seen[j];
				info.seen[j] = info_nou.seen[j];
				if(leg[j]) {
					info.seen[j] = 1;
				}
			}
			info.seen[rank] = 1;
			for(j = 0; j < size; j++) {
				//daca mesajul nu a mai fost vazut 
				//de acel nod legatura i-l trimit
				if(leg[j] && (!visited_nodes[j])) {
					MPI_Isend(&info, 1, my_info_struct, j, 0,
						  MPI_COMM_WORLD, &request);
				}
			}
		}
		else if(info_nou.type == INFO) {
			info.source = info_nou.source;
			info.dest = info_nou.dest;
			if(info_nou.dest == rank) {
				printf("DEST ID %d %s from %d source %d\n", 
					rank, info_nou.payload, transm, info_nou.source);
			}
			else {
				info.type = info_nou.type;
				strcpy(info.payload, info_nou.payload);
				//urmatorul nod caruia trebuie sa trimit mesajul
				info_next_hop = next_hop[info.dest];
				printf("INTER ID %d %s from %d next_hop %d source %d to %d\n", 
					rank, info.payload, transm, info_next_hop,
					info.source, info.dest);
				MPI_Isend(&info, 1, my_info_struct, info_next_hop,
					  0, MPI_COMM_WORLD, &request);
				
			}
			
		}	
	}
	usleep(10000);
	/**********************************************************************
					ETAPA 3
				  ALEGEREA LIDERULUI
	***********************************************************************/
	//calculez arborele minim de acoperire
	for(i = 0; i < size; i++) {
		if(leg[i]) {
			if(no_cycle_nodes > 0) {
				if(!contains(cycle_nodes, no_cycle_nodes, i)) {
					neighbors[no_of_neighbors] = i;
					no_of_neighbors++;
				}
			}
			else {
				neighbors[no_of_neighbors] = i;
				no_of_neighbors++;
			}
		}
	}
	//aplic algoritmul unda arbore pentru alegerea liderului
	//retine id-urile tuturor proceselor
	int *ids = (int*) malloc(size * sizeof(int));
	int *ids_nou = (int*) malloc(size * sizeof(int));
	int r, q;
	//retine de la cine am primit mesaj
	int *rec = (int*) malloc(size * sizeof(int));
	//initial cunosc doar id-ul propriu
	for(i = 0; i < size; i++) {
		if(i == rank) {
			ids[i] = rank;
		}
		else {
			ids[i] = -1;
		}
		rec[i] = 0;
	}
	r = no_of_neighbors;
	while(r > 1) {
		MPI_Recv(ids_nou, size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		r--;
		transm = status.MPI_SOURCE;
		rec[transm] = 1;
		for(i = 0; i < size; i++) {
			if(ids_nou[i] != -1) {
				ids[i] = ids_nou[i];
			}
		}
	}
	for(i = 0; i < no_of_neighbors; i++) {
		if(rec[neighbors[i]] == 0) {
			q = neighbors[i];
			MPI_Send(ids, size, MPI_INT, neighbors[i], 0, MPI_COMM_WORLD);
		}
	}
	MPI_Recv(ids_nou, size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	for(i = 0; i < size; i++) {
		if(ids_nou[i] != -1) {
			ids[i] = ids_nou[i];
		}
	}
	for(i = 0; i < no_of_neighbors; i++) {
		if(neighbors[i] != q) {
			MPI_Send(ids, size, MPI_INT, neighbors[i], 0, MPI_COMM_WORLD);
		}
	}
	bubble_sort(ids, size);
	MPI_Barrier(MPI_COMM_WORLD);
	printf("%d %d %d\n", rank, ids[0], ids[1]);
	
	MPI_Type_free(&my_mpi_struct);
	MPI_Type_free(&my_info_struct);
	free(top); free(leg); free(next_hop); free(path);
	free(ids); free(ids_nou); free(rec); free(marked);
	free(parent);			
	MPI_Finalize();
	return 0;
}
	
	
	
