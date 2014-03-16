#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "lib.h"

int main(int argc, char *argv[])
{
	msg r, t;
	my_pkt p;
	int res;
	int task_index, fd, no_of_frames, i, j, window_size;
	char filename[50], file[50];
	int frame_expected = 0, frame_received, nr = 0;

	task_index = atoi(argv[1]);	
	printf("[RECEIVER] Receiver starts.\n");
	printf("[RECEIVER] Task index=%d\n", task_index);
		
	init(HOST, PORT2);
	
	if((task_index == 0) || (task_index == 1) || (task_index == 2)) {
	
		memset(&p, 0, sizeof(my_pkt));
		memset(&t, 0, sizeof(msg));
		
		/*astept frame-ul cu numele fisierului si marimea ferestrei*/
		recv_message(&r);
		p = *((my_pkt*) r.payload);
		if(p.type != TYPE1) {
			memset(&p, 0, sizeof(my_pkt));
			p.type = NACK_TYPE;
			p.sequence = -1;
			memcpy(p.payload, NACK, strlen(NACK) + 1);
			printf("[RECEIVER] Send NACK for type %d\n", TYPE1);
		} else {
			memcpy(file, p.payload, r.len - 2 * sizeof(int));
			/*retin dimensiunea ferestrei*/
			memcpy(&window_size, &p.sequence, sizeof(int));
			printf("[RECEIVER] window size %d\n", window_size);
			/*formez numele fisierului receiverului*/
			sprintf(filename, "recv_%s", file);
			printf("[RECEIVER] Filename: %s\n", filename);
			 /*deschid fisierul*/
			fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
			memset(&p, 0, sizeof(my_pkt));
			p.type = ACK_TYPE1;
			p.sequence = -1;
			memcpy(p.payload, ACK_T1, strlen(ACK_T1));
			printf("[RECEIVER] Send ACK for type %d\n", TYPE1);
		}
		t.len = 2 * sizeof(int) + strlen(p.payload);
		memcpy(t.payload, &p, t.len);
		res = send_message(&t);
		if(res < 0) {
			perror("[RECEIVER] Error sending ACK");
			return -1;
				
		}
	
		/*astept frame-ul cu numarul de cadre */
		recv_message(&r);
		p = *((my_pkt*) r.payload);
		if(p.type != TYPE2) {
			memset(&p, 0, sizeof(my_pkt));
			p.type = NACK_TYPE;
			p.sequence = -1;
			memcpy(p.payload, NACK, strlen(NACK));
			printf("[RECEIVER] Send NACK for type %d\n", TYPE2);
		} else {
			memcpy(&no_of_frames, &p.sequence, sizeof(int));
			printf("[RECEIVER] Number of frames %d\n", no_of_frames);
			memset(&p, 0, sizeof(my_pkt));
			p.type = ACK_TYPE2;
			p.sequence = -1;
			memcpy(p.payload, ACK_T2, strlen(ACK_T2));
			printf("[RECEIVER] Send ACK for type %d\n", TYPE2);
		}
		t.len = 2 * sizeof(int) + strlen(p.payload);
		memcpy(t.payload, &p, t.len);
		res = send_message(&t);
		if(res < 0) {
			perror("[RECEIVER] Error sending ACK");
			return -1;
				
		}
	}
	

	/*Task-ul 0*/
	if(task_index == 0) {
		for(i = 0; i < no_of_frames; i++){
			if (recv_message(&r) < 0) {
				perror("[RECEIVER] Error receive message");
				return -1;
			}else{
				p = *((my_pkt *) r.payload);
				write(fd, p.payload, r.len - 2 * sizeof(int));
				memset(&t, 0, sizeof(msg));
				memset(&p, 0, sizeof(my_pkt));
				p.type = TYPE3;
				p.sequence = i;
				memcpy(p.payload, ACK_T3, strlen(ACK_T3));
				t.len = 2 * sizeof(int) + strlen(p.payload);
				memcpy(t.payload, &p, t.len);
				printf("[RECEIVER] ACK %d\n", i);
				send_message(&t);
			}
		}
	}
		
	/*Task-ul 1 - Go-Back-N*/
	if(task_index == 1) {
		/*astept frame-urile*/
		while(frame_expected < no_of_frames) {
			res = recv_message(&r);
			memset(&p, 0, sizeof(my_pkt));
			p = *((my_pkt *) r.payload);
			/*daca am primit cadrul asteptat*/
			if(p.sequence == frame_expected) {
				write(fd, p.payload, r.len - 2 * sizeof(int));
				memset(&p, 0, sizeof(my_pkt));
				p.type = TYPE3;
				p.sequence = frame_expected;
				memcpy(p.payload, ACK_T3, strlen(ACK_T3));
				frame_expected++;
				memset(&t, 0, sizeof(msg));
				t.len = 2 * sizeof(int) + strlen(p.payload);
				memcpy(t.payload, &p, t.len);
				res = send_message(&t);
				printf("[RECEIVER] ACK %d\n", p.sequence);
				if(res < 0) {
					perror("[RECEIVER] Error sending ACK");
					return -1;
				}
			}	
		}
			
	}
	
	/*Task-ul 2 - Selective repeat */
	
	if(task_index == 2) {
		int frame_ack[no_of_frames];
		int frames_written = 0;
		msg buf_frame[window_size];
		
		for(i = 0; i < no_of_frames; i++) {
			frame_ack[i] = NACK_TYPE;
		}
		
		while(frames_written < no_of_frames) {
			/*astept frame-urile sa soseasca*/
			memset(&p, 0, sizeof(my_pkt));
			res = recv_message(&r);

			memcpy(&p, r.payload, r.len);
			frame_received = p.sequence;
			
			/*daca am primt frame-ul corespunzator*/
			if(frame_expected == frame_received) {
				nr = 0;
				frame_ack[frame_received] = ACK_TYPE3;
				j = frame_received;
				memcpy(&buf_frame[frame_received % window_size], &r, r.len + sizeof(int));
				while(frame_ack[j] == ACK_TYPE3) {
					p = *((my_pkt *) buf_frame[j % window_size].payload);
					write(fd, p.payload, buf_frame[j % window_size].len - 2 * sizeof(int));
					j++;
					frames_written++;
					nr++;
				}
				frame_expected += nr;
			}
			else {
				memcpy(&buf_frame[frame_received % window_size], &r, r.len + sizeof(int));
				frame_ack[frame_received] = ACK_TYPE3;
			}
			/*trimit ACK pentru frame-ul primit*/
			memset(&p, 0, sizeof(my_pkt));
			p.type = TYPE3;
			p.sequence = frame_received;
			memcpy(p.payload, ACK_T3, strlen(ACK_T3));
			memset(&t, 0, sizeof(msg));
			t.len = 2 * sizeof(int) + strlen(p.payload);
			memcpy(t.payload, &p, t.len);
			res = send_message(&t);
			printf("[RECEIVER] ACK %d\n", p.sequence);

			if(res < 0) {
				perror("[RECEIVER] Error sending ACK");
				return -1;
			}
			
			
		}
	}
	
	/*Task-ul 3 - Detectia erorilor*/
	if(task_index == 3) {
		bool arrived = false;
		char aux_byte, byte;
		control_pkt p;
		int *vector_ack;
		int frames_written = 0;
		no_of_frames = 0;
		msg* buf_frame;
		memset(&p, 0, sizeof(control_pkt));
		memset(&t, 0, sizeof(msg));
		
		
		while(!arrived) {
			/*astept frame-ul cu numele fisierului si marimea ferestrei*/
			recv_message(&r);
			p = *((control_pkt*) r.payload);
			
			if(equal(p.control_byte1, p.control_byte2)) {
	
				aux_byte = p.control_byte1;
				/*formez byte-ul de control pentru mesajul ajuns*/
				p.control_byte1 = p.control_byte2 = 0x00000000;
				byte = getControlByte((char *) &p.type, r.len);
				
				/*daca mesajul a ajuns corect*/
				if(equal(aux_byte, byte)) {
					if(p.type != TYPE1) {
						memset(&p, 0, sizeof(control_pkt));
						p.type = NACK_TYPE;
						p.sequence = -1;
						p.control_byte1 = p.control_byte2 = 0x00000000;
						memcpy(p.payload, NACK, strlen(NACK) + 1);
						printf("[RECEIVER] Sent NACK for type %d\n", TYPE1);
					} else {
						arrived = true;
						memcpy(file, p.payload, r.len - 2 * sizeof(char) - 2 * sizeof(int));
						/*retin dimensiunea ferestrei*/
						memcpy(&window_size, &p.sequence, sizeof(int));
						printf("[RECEIVER] window size %d\n", window_size);
						buf_frame = (msg*) malloc(window_size * sizeof(msg));
						/*formez numele fisierului receiverului*/
						sprintf(filename, "recv_%s", file);
						printf("[RECEIVER] Filename: %s\n", filename);
						 /*deschid fisierul*/
						fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
						memset(&p, 0, sizeof(control_pkt));
						p.type = ACK_TYPE1;
						p.sequence = -1;
						p.control_byte1 = p.control_byte2 = 0x00000000;
						memcpy(p.payload, ACK_T1, strlen(ACK_T1));
						printf("[RECEIVER] Sent ACK for type %d\n", TYPE1);
					}
					t.len = 2 * sizeof(char) + 2 * sizeof(int) + strlen(p.payload);
					memcpy(t.payload, &p, t.len);
					res = send_message(&t);
					if(res < 0) {
						perror("[RECEIVER] Error sending ACK");
						return -1;
				
					}
				}
			}
		}
	
		arrived = false;
		/*astept frame-ul cu numarul de cadre */
		while(!arrived) {
			recv_message(&r);
			p = *((control_pkt *) r.payload);
			if(equal(p.control_byte1, p.control_byte2)) {
	
				aux_byte = p.control_byte1;
				/*formez byte-ul de control pentru mesajul ajuns*/
				p.control_byte1 = p.control_byte2 = 0x00000000;
				byte = getControlByte((char *) &p.type, r.len);
				
				/*daca mesajul a ajuns corect*/
				if(equal(aux_byte, byte)) {
					if(p.type != TYPE2) {
						memset(&p, 0, sizeof(control_pkt));
						p.type = NACK_TYPE;
						p.sequence = -1;
						p.control_byte1 = p.control_byte2 = 0x00000000;
						memcpy(p.payload, NACK, strlen(NACK));
						printf("[RECEIVER] Send NACK for type %d\n", TYPE2);
					} else {
						no_of_frames = p.sequence;
						vector_ack = (int*) malloc (no_of_frames * sizeof (int));
						printf("[RECEIVER] Number of frames %d\n", no_of_frames);
						memset(&p, 0, sizeof(control_pkt));
						p.type = ACK_TYPE2;
						p.sequence = -1;
						p.control_byte1 = p.control_byte2 = 0x00000000;
						memcpy(p.payload, ACK_T2, strlen(ACK_T2));
						printf("[RECEIVER] Send ACK for type %d\n", TYPE2);
						arrived = true;
					}
					t.len = 2 * sizeof(char) + 2 * sizeof(int) + strlen(p.payload);
					memcpy(t.payload, &p, t.len);
					res = send_message(&t);
					if(res < 0) {
						perror("[RECEIVER] Error sending ACK");
						return -1;
				
					}
				}
			}
		}
			
		
		for(i = 0; i < no_of_frames; i++) {
			vector_ack[i] = NACK_TYPE;
			
		}
		frame_expected = 0;
		frame_received = -1;
		
		while(frames_written < no_of_frames) {
			
			/*astept frame-urile sa soseasca*/
			memset(&p, 0, sizeof(control_pkt));
			res = recv_message(&r);

			p = *((control_pkt *) r.payload);
			
			/*verific daca mesajul a ajuns corect*/
			/*in primul rand trebuie ca byte-ul de control sa nu fie corupt*/
			if(equal(p.control_byte1, p.control_byte2)) {
	
				aux_byte = p.control_byte1;
				/*formez byte-ul de control pentru mesajul ajuns*/
				p.control_byte1 = p.control_byte2 = 0x00000000;
				byte = getControlByte((char *) &p.type, r.len);
				
				/*daca mesajul a ajuns corect*/
				if(equal(aux_byte, byte)) {
					
					frame_received = p.sequence;
					/*daca am primt frame-ul corespunzator*/
					/*trimit ACK pentru frame-ul primit*/
					memset(&p, 0, sizeof(control_pkt));
					p.type = TYPE3;
					p.sequence = frame_received;
					p.control_byte1 = p.control_byte2 = 0x00000000;
					memcpy(p.payload, ACK_T3, strlen(ACK_T3));
					memset(&t, 0, sizeof(msg));
					t.len = 2 * sizeof(char) + 2 * sizeof(int) + strlen(p.payload);
					memcpy(t.payload, &p, t.len);
					res = send_message(&t);
					printf("[RECEIVER] ACK %d\n", p.sequence);

					if(res < 0) {
						perror("[RECEIVER] Error sending ACK");
						return -1;
					}
					
					if(frame_expected == frame_received) {
					
						nr = 0;
						vector_ack[frame_received] = ACK_TYPE3;
						j = frame_received;
						memcpy(&buf_frame[frame_received % window_size], &r, r.len + sizeof(int));

						while(vector_ack[j] == ACK_TYPE3) {
							p = *((control_pkt *) buf_frame[j % window_size].payload);
							write(fd, p.payload, buf_frame[j % window_size].len -
										2 * sizeof(char) - 2 * sizeof(int));
							j++;
							frames_written++;
							nr++;
						}
						frame_expected += nr;
					}
					else {
						memcpy(&buf_frame[frame_received % window_size], &r, r.len + sizeof(int));
						vector_ack[frame_received] = ACK_TYPE3;
					}

				}
			}
			
			
		}
	}
			
	printf("[RECEIVER] All done.\n");
	close(fd);
	return 0;
}
