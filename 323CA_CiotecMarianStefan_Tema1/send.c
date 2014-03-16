#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "lib.h"




int main(int argc, char *argv[])
{
	msg t, r;
	my_pkt p;
	char* filename;
	int count, sent, res, timeout;
	int task_index, speed, delay;
	int bdp, window_size, no_of_frames;
	int fd, filesize;
	int  i, j, nack_no = 0, ack_total = 0, nr;
	int ack_expected = 0, buf_size = 0, index, ack_received = -1;
	char buffer[PKTSIZE];
	struct stat f_status;

	task_index = atoi(argv[1]);
	filename = argv[2];
	speed = atoi(argv[3]);
	delay = atoi(argv[4]);
	
	printf("[SENDER] Sender starts.\n");
	printf("[SENDER] Filename=%s, task_index=%d, speed=%d, delay=%d\n", filename, task_index, speed, delay);
	
	init(HOST, PORT1);
	
	bdp = ((speed * delay) * 1000) / BITS_NO;
	window_size = bdp / FRAME_SIZE;
	/*bufferul*/
	msg buf_frame[window_size];
	
	printf("[SENDER] BDP = %d bytes\n", bdp);
	printf("[SENDER] Window Size = %d frames\n", window_size);
	
	fd = open(filename, O_RDONLY);
	fstat(fd, &f_status);
	filesize = (int) f_status.st_size;
	printf("[SENDER] File size: %d\n", filesize);
	if((task_index == 0) || (task_index == 1) || (task_index == 2)) {
		no_of_frames = (filesize / PKTSIZE) + 1;
		printf("[SENDER] Gonna send %d frames\n", no_of_frames);
	}
	printf("[SERVER] File transfer begins.\n");
	
	/*calculez delay-ul*/
	if((2 * delay) > 1000) {
		timeout = 2 * delay;
	}else{
		timeout = 1000;
	}
	if((task_index == 0) || (task_index == 1) || (task_index == 2)) {	
		/*trimit numele fisierului si marimea ferestrei*/
		memset(&t, 0, sizeof(msg));
		memset(&p, 0, sizeof(my_pkt));
		
		p.type = TYPE1;
		p.sequence = window_size;
		memcpy(p.payload, filename, strlen(filename));
		t.len = 2 * sizeof(int) + strlen(filename);
		memcpy(t.payload, &p, t.len);
		do { 
			send_message(&t);
			res = recv_message_timeout(&r, timeout);
			p = *((my_pkt *) r.payload);
			printf("[SENDER] Sending filename\n");
			
		} while((res == -1) && (p.type != ACK_TYPE1));
		printf("[SENDER] Filename sent\n");
		
		/*trimit numarul de cadre*/
		memset(&t, 0, sizeof(msg));
		memset(&p, 0, sizeof(my_pkt));
		
		p.type = TYPE2;
		memcpy(&p.sequence, &no_of_frames, sizeof(int));
		t.len = 2 * sizeof(int);
		memcpy(t.payload, &p, t.len);
		do { 
			send_message(&t);
			res = recv_message_timeout(&r, timeout);
			p = *((my_pkt *) r.payload);
			
		} while((res == -1) && (p.type != ACK_TYPE2));
		printf("[SENDER] Number of frames sent\n");
	}
	
	/*Task-ul 0*/
	if(task_index == 0) {
		/*umplu legatura de date*/
		for(sent = 0; sent < no_of_frames; sent++) { 
		
			if((count = read(fd, buffer, PKTSIZE)) > 0) {
				/* cleanup msg */
				memset(&t, 0, sizeof(msg));
				memset(&p, 0, sizeof(my_pkt));
				memcpy(p.payload, buffer, count);
				p.sequence = sent;
				p.type = TYPE3;
				t.len = 2 * sizeof(int) + count;
				memcpy(t.payload, &p, t.len);
				res = send_message(&t);
				if(res < 0){
					perror("[SENDER] Send error. Exiting.\n");
					return -1;
				} else{
					printf("[SENDER] Sending frame : %d\n", sent);
				}
			}
			
			if((sent + 2) > window_size) {
				/*astept ACK*/
				res = recv_message(&r);
				p = *((my_pkt *) r.payload);
				if(res < 0){
					perror("[SENDER] ACK receiving error \n");
				}
			}
		}
		
		/*astept ACK-urile ramase*/
		for(i = 0; i < window_size - 1; i++) {
			/*astept ACK*/
			res = recv_message(&r);
			p = *((my_pkt *) r.payload);
			if(res < 0){
				perror("[SENDER] ACK receiving error \n");
			}
		}
	}
	
	
	/*Task-ul 1 - Go-Back-N*/
	if(task_index == 1){
		sent = 0;
		while(sent < no_of_frames) {
			while(buf_size < window_size) {
				if((count = read(fd, buffer, PKTSIZE)) > 0){
					memset(&t, 0, sizeof(msg));
					memset(&p, 0, sizeof(my_pkt));
					memcpy(p.payload, buffer, count);
					p.sequence = sent;
					p.type = TYPE3;
					t.len = 2 * sizeof(int) + count;
					memcpy(t.payload, &p, t.len);
					memcpy(&buf_frame[sent % window_size], &t, t.len + sizeof(int));
					send_message(&buf_frame[sent % window_size]);
					printf("[SENDER] Sending frame %d\n", p.sequence);
					buf_size++;
					sent++;
				}
			}
			
			if(buf_size == window_size) {
				/*astept ACK*/
				memset(&p, 0, sizeof(my_pkt));
				res = recv_message_timeout(&r, timeout);
				/*daca ACK ajunge*/
				if(res >= 0) {
					memcpy(&p, r.payload, r.len);
					/*daca este ACK asteptat*/
					if(p.sequence == ack_expected) {
						buf_size--;
						printf("[SENDER] Received ACK for frame %d\n", ack_expected);
						ack_expected++;
					}
					
				}
				/*daca ACK nu ajunge*/
				else {
					/*pozitia din buffer*/
					index = (ack_expected % window_size);
					for(i = 0; i < window_size; i++) {
						memset(&t, 0, sizeof(msg));
						memset(&p, 0, sizeof(my_pkt));
						memcpy(&t, &buf_frame[index % window_size], 
								buf_frame[index % window_size].len + sizeof(int));
						p = *((my_pkt *) buf_frame[index % window_size].payload);
						index++;
						send_message(&t);
						printf("[SENDER] Resending frame %d\n", p.sequence);
					}
				}
			}
		}
		
		/*daca s-au trimis toate cadrele 
		mai astept ACK pentru window_size cadre*/
		if(sent == no_of_frames) {
			while(buf_size > 0) {
				memset(&p, 0, sizeof(my_pkt));
				res = recv_message_timeout(&r, timeout);
				/*daca ACK ajunge*/
				if(res >= 0) {
					memcpy(&p, r.payload, r.len);
					/*daca este ACK asteptat*/
					if(p.sequence == ack_expected) {
						buf_size--;
						ack_expected++;
					} 
				}
				/*daca ACK nu ajunge*/
				else {
					index = (ack_expected % window_size);
					for(i = 0; i < buf_size; i++) {
						index = (ack_expected % window_size);
						for(i = 0; i < window_size; i++) {
							memset(&t, 0, sizeof(msg));
							memset(&p, 0, sizeof(my_pkt));
							memcpy(&t, &buf_frame[index % window_size], 
									buf_frame[index % window_size].len + sizeof(int));
							p = *((my_pkt *) buf_frame[index % window_size].payload);
							index++;
							send_message(&t);
							printf("[SENDER] Resending frame %d\n", p.sequence);
						}
					}
				}
			}
		}
		
	}
	
	/*Task 2 - Selective Repeat*/
	if(task_index == 2) {
		/*retine daca un frame a primit ack*/
		int frame_ack[no_of_frames];
		/*ultimul cadru citit din fisier si trimis*/
		sent = -1;
		buf_size = 0;
		/*daca n-am primit ack pentru toate frame-urile trimise*/
		while(sent < no_of_frames - 1) {
		
			while((buf_size < window_size) && (sent < no_of_frames - 1)) {
			
				if((count = read(fd, buffer, PKTSIZE)) > 0){
					memset(&t, 0, sizeof(msg));
					memset(&p, 0, sizeof(my_pkt));
					memcpy(p.payload, buffer, count);
					sent++;
					p.sequence = sent;
					p.type = TYPE3;
					t.len = 2 * sizeof(int) + count;
					memcpy(t.payload, &p, t.len);
					memcpy(&buf_frame[sent % window_size], &t, t.len + sizeof(int));
					buf_size++;
					/*frame-ul este marcat ca neconfirmat*/
					frame_ack[sent] = NACK_TYPE;
					send_message(&t);
					printf("[SENDER] Sending frame %d\n", p.sequence);
				}
			}
			
			if(buf_size == window_size) {
				/*astept ACK*/
				memset(&p, 0, sizeof(my_pkt));
				res = recv_message_timeout(&r, timeout);
				
				if(res >= 0) {
					memcpy(&p, r.payload, r.len);
					ack_received = p.sequence;
					/*daca am primit ACK-ul corespunzator*/
					if(ack_expected == ack_received) {
						frame_ack[ack_received] = ACK_TYPE3;
						ack_total++;
						j = ack_received;
						nr = 0;
						while(frame_ack[j] == ACK_TYPE3) {
							nr++;
							j++;
						}
						/*voi trimite alte nr frame-uri*/
						if((nack_no == 0) && (sent < no_of_frames - 1)) {
							buf_size -= nr;
						}
						ack_expected += nr;
					}
					/*daca am primit alt ACK*/
					else{
						frame_ack[ack_received] = ACK_TYPE3;
						/*retin cate frame-uri n-au primit ACK inaintea lui*/
						for(j = ack_expected; j < ack_received; j++) {
							if(frame_ack[j] == NACK_TYPE) {
								nack_no += 1;
							}
						}
						/*astept urmatorul ACK*/
						j = ack_received;
						nr = 0;
						while(frame_ack[j] == ACK_TYPE3) {
							nr++;
							j++;
						}
						ack_expected = ack_received + nr;
						ack_total++;
					}
					/*daca am primit ACK pentru ultimul frame trimis si 
					s-au pierdut inaintea lui frame-uri*/
					if(ack_received == sent) {
					
						for(i = sent - window_size + 1; i < sent; i++) {
							if(frame_ack[i] == NACK_TYPE) {
								ack_expected = i;
								break;
							}
						}
					
						/*retrimit doar frame-urile pierdute*/
						for(i = sent - window_size + 1; i <= sent; i++) {
							if(frame_ack[i] == NACK_TYPE) {
								send_message(&buf_frame[i % window_size]);
								printf("[SENDER] Resending frame %d\n", i);
							}
						}
						nack_no = 0;
					}
				}
				/*daca nu a ajuns niciun ACK*/
				else{
					/*retrimit toate cadrele pana la sent*/
					j = sent;
					if((j - window_size + 1) < 0) {
						j = 0;
					}
					else {
						j -= window_size + 1;
					}
					
					for(i = j; i <= sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							ack_expected = i;
							break;
						}
					}
					
					for(i = j; i <= sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							send_message(&buf_frame[i % window_size]);
							printf("[SENDER] Resending frame %d\n", i);
						}
						
					}
					nack_no = 0;
				}
			}
		}
		/*ma opresc cand am primit ACK-uri pentru toate frame-urile*/
		while(ack_total < no_of_frames) {
			/*astept ACK*/
			memset(&p, 0, sizeof(my_pkt));
			res = recv_message_timeout(&r, timeout);
				
			if(res >= 0) {
				memcpy(&p, r.payload, r.len);
				ack_received = p.sequence;
				/*daca am primit ACK-ul corespunzator*/
				if(ack_expected == ack_received) {
					frame_ack[ack_received] = ACK_TYPE3;
					ack_total++;
					j = ack_received;
					nr = 0;
					while(frame_ack[j] == ACK_TYPE3) {
						nr++;
						j++;
					}
					
					ack_expected += nr;
				}
					/*daca am primit alt ACK*/
				else{
					frame_ack[ack_received] = ACK_TYPE3;
					/*retin cate frame-uri n-au primit ACK inaintea lui*/
					for(j = ack_expected; j < ack_received; j++) {
						if(frame_ack[j] == NACK_TYPE) {
							nack_no += 1;
						}
					}
					/*astept urmatorul ACK*/
					j = ack_received;
					nr = 0;
					while(frame_ack[j] == ACK_TYPE3) {
						nr++;
						j++;
					}
					ack_expected = ack_received + nr;
					ack_total++;
				}
				/*daca am primit ACK pentru ultimul frame trimis si 
				s-au pierdut inaintea lui frame-uri*/
				if(ack_received == sent) {
					
					for(i = sent - window_size + 1; i < sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							ack_expected = i;
							break;
						}
					}
					
					/*retrimit doar frame-urile pierdute*/
					for(i = sent - window_size + 1; i <= sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							send_message(&buf_frame[i % window_size]);
							printf("[SENDER] Resending frame %d\n", i);
						}
			
					}
				}
			}
			/*daca nu a ajuns niciun ACK*/
			else{
				/*retrimit toate cadrele pana la sent*/
				j = sent;
				if((j - window_size + 1) < 0) {
					j = 0;
				}
				else {
					j -= window_size + 1;
				}
					
				for(i = j; i <= sent; i++) {
					if(frame_ack[i] == NACK_TYPE) {
						ack_expected = i;
						break;
					}
				}
					
				for(i = j; i <= sent; i++) {
					if(frame_ack[i] == NACK_TYPE) {
						send_message(&buf_frame[i % window_size]);
						printf("[SENDER] Resending frame %d\n", i);
					}
						
				}
			}
		}
		
	}
	
	/*Task 3- Detectia erorilor*/
	if(task_index == 3) {
	
		char val;
		control_pkt p;
		no_of_frames = (filesize / CONTROLSIZE) + 1;
		printf("[SENDER] Gonna send %d frames\n", no_of_frames);
		/*retine daca un frame a primit ack*/
		int frame_ack[no_of_frames];
		/*ultimul cadru citit din fisier si trimis*/
		sent = -1;
		buf_size = 0;
		
		/*trimit numele fisierului si marimea ferestrei*/
		memset(&t, 0, sizeof(msg));
		memset(&p, 0, sizeof(control_pkt));
		/*setez campurile din structura control_pkt*/
		p.type = TYPE1;
		p.sequence = window_size;
		p.control_byte1 = p.control_byte2 = 0x00000000;
		memcpy(p.payload, filename, strlen(filename));
		
		/*calculez byte-ul de control*/
		t.len = 2 * sizeof(char) + 2 * sizeof(int) + strlen(p.payload);
		val = getControlByte((char *) &p.type, t.len);
		
		memcpy(&p.control_byte1, &val, sizeof(char));
		memcpy(&p.control_byte2, &val, sizeof(char));
		
		memcpy(t.payload, &p, t.len);
		printf("[SENDER] Filename sent %s\n", p.payload);
		do { 
			send_message(&t);
			res = recv_message_timeout(&r, timeout);
			p = *((control_pkt *) r.payload);
			printf("[SENDER] Sending filename\n");
			
		} while((res == -1) || (p.type == NACK_TYPE));
		printf("[SENDER] Filename sent\n");
		
		/*trimit numarul de cadre*/
		memset(&t, 0, sizeof(msg));
		memset(&p, 0, sizeof(control_pkt));
		
		p.type = TYPE2;
		memcpy(&p.sequence, &no_of_frames, sizeof(int));
		p.control_byte1 = p.control_byte2 = 0x00000000;
		t.len = 2 * sizeof(char) + 2 * sizeof(int);
		
		/*calculez byte-ul de control*/
		val = getControlByte((char *) &p.type, t.len);
		memcpy(&p.control_byte1, &val, sizeof(char));
		memcpy(&p.control_byte2, &val, sizeof(char));
		
		memcpy(t.payload, &p, t.len);
		do { 
			send_message(&t);
			res = recv_message_timeout(&r, timeout);
			p = *((control_pkt *) r.payload);
			
		} while((res == -1) || (p.type == NACK_TYPE));
		printf("[SENDER] Number of frames sent\n");
		
		/*daca n-am primit ack pentru toate frame-urile trimise*/
		while(sent < no_of_frames - 1) {
		
			while((buf_size < window_size) && (sent < no_of_frames - 1)) {
			
				if((count = read(fd, buffer, CONTROLSIZE)) > 0){
				
					memset(&t, 0, sizeof(msg));
					memset(&p, 0, sizeof(control_pkt));
					memcpy(p.payload, buffer, count);
					sent++;
					
					p.sequence = sent;
					p.type = TYPE3;
					p.control_byte1 = p.control_byte2 = 0x00000000;
					t.len = 2 * sizeof(char) + 2 * sizeof(int) + count;
	
					/*calculez byte-ul de control*/
					val = getControlByte((char *) &p.type, t.len);
					memcpy(&p.control_byte1, &val, sizeof(char));
					memcpy(&p.control_byte2, &val, sizeof(char));
					
					memcpy(t.payload, &p, t.len);
					/*retin mesajul in buffer*/
					memcpy(&buf_frame[sent % window_size], &t, t.len + sizeof(int));
					buf_size++;
					/*frame-ul este marcat ca neconfirmat*/
					frame_ack[sent] = NACK_TYPE;
					send_message(&t);
					printf("[SENDER] Sending frame %d\n", p.sequence);
					
				}
			}
			
			if(buf_size == window_size) {
				/*astept ACK*/
				memset(&p, 0, sizeof(control_pkt));
				res = recv_message_timeout(&r, timeout);
				
				if(res >= 0) {
					memcpy(&p, r.payload, r.len);
					ack_received = p.sequence;
					/*daca am primit ACK-ul corespunzator*/
					if(ack_expected == ack_received) {
						frame_ack[ack_received] = ACK_TYPE3;
						ack_total++;
						j = ack_received;
						nr = 0;
						while(frame_ack[j] == ACK_TYPE3) {
							nr++;
							j++;
						}
						/*voi trimite alte nr frame-uri*/
						if((nack_no == 0) && (sent < no_of_frames - 1)) {
							buf_size -= nr;
						}
						ack_expected += nr;
					}
					/*daca am primit alt ACK*/
					else{
						frame_ack[ack_received] = ACK_TYPE3;
						/*retin cate frame-uri n-au primit ACK inaintea lui*/
						for(j = ack_expected; j < ack_received; j++) {
							if(frame_ack[j] == NACK_TYPE) {
								nack_no += 1;
							}
						}
						/*astept urmatorul ACK*/
						j = ack_received;
						nr = 0;
						while(frame_ack[j] == ACK_TYPE3) {
							nr++;
							j++;
						}
						ack_expected = ack_received + nr;
						ack_total++;
					}
					/*daca am primit ACK pentru ultimul frame trimis si 
					s-au pierdut inaintea lui frame-uri*/
					if(ack_received == sent) {
					
						for(i = sent - window_size + 1; i < sent; i++) {
							if(frame_ack[i] == NACK_TYPE) {
								ack_expected = i;
								break;
							}
						}
					
						/*retrimit doar frame-urile pierdute*/
						for(i = sent - window_size + 1; i <= sent; i++) {
							if(frame_ack[i] == NACK_TYPE) {
								send_message(&buf_frame[i % window_size]);
								printf("[SENDER] Resending frame %d\n", i);
							}
						}
						nack_no = 0;
					}
				}
				/*daca nu a ajuns niciun ACK*/
				else{
					/*retrimit toate cadrele pana la sent*/
					j = sent;
					if((j - window_size + 1) < 0) {
						j = 0;
					}
					else {
						j -= window_size + 1;
					}
					
					for(i = j; i <= sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							ack_expected = i;
							break;
						}
					}
					
					for(i = j; i <= sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							send_message(&buf_frame[i % window_size]);
							printf("[SENDER] Resending frame %d\n", i);
						}
						
					}
					nack_no = 0;
				}
			}
		}
		/*ma opresc cand am primit ACK-uri pentru toate frame-urile*/
		while(ack_total < no_of_frames) {
			/*astept ACK*/
			memset(&p, 0, sizeof(my_pkt));
			res = recv_message_timeout(&r, timeout);
				
			if(res >= 0) {
				memcpy(&p, r.payload, r.len);
				ack_received = p.sequence;
				/*daca am primit ACK-ul corespunzator*/
				if(ack_expected == ack_received) {
					frame_ack[ack_received] = ACK_TYPE3;
					ack_total++;
					j = ack_received;
					nr = 0;
					while(frame_ack[j] == ACK_TYPE3) {
						nr++;
						j++;
					}
					
					ack_expected += nr;
				}
					/*daca am primit alt ACK*/
				else{
					frame_ack[ack_received] = ACK_TYPE3;
					/*retin cate frame-uri n-au primit ACK inaintea lui*/
					for(j = ack_expected; j < ack_received; j++) {
						if(frame_ack[j] == NACK_TYPE) {
							nack_no += 1;
						}
					}
					/*astept urmatorul ACK*/
					j = ack_received;
					nr = 0;
					while(frame_ack[j] == ACK_TYPE3) {
						nr++;
						j++;
					}
					ack_expected = ack_received + nr;
					ack_total++;
				}
				/*daca am primit ACK pentru ultimul frame trimis si 
				s-au pierdut inaintea lui frame-uri*/
				if(ack_received == sent) {
					
					for(i = sent - window_size + 1; i < sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							ack_expected = i;
							break;
						}
					}
					
					/*retrimit doar frame-urile pierdute*/
					for(i = sent - window_size + 1; i <= sent; i++) {
						if(frame_ack[i] == NACK_TYPE) {
							send_message(&buf_frame[i % window_size]);
							printf("[SENDER] Resending frame %d\n", i);
						}
			
					}
				}
			}
			/*daca nu a ajuns niciun ACK*/
			else{
				/*retrimit toate cadrele pana la sent*/
				j = sent;
				if((j - window_size + 1) < 0) {
					j = 0;
				}
				else {
					j -= window_size + 1;
				}
					
				for(i = j; i <= sent; i++) {
					if(frame_ack[i] == NACK_TYPE) {
						ack_expected = i;
						break;
					}
				}
					
				for(i = j; i <= sent; i++) {
					if(frame_ack[i] == NACK_TYPE) {
						send_message(&buf_frame[i % window_size]);
						printf("[SENDER] Resending frame %d\n", i);
					}
						
				}
			}
		}
		
	}
	
	printf("[SENDER] Job done.\n");
	
	return 0;
}
