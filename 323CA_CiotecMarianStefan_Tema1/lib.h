#ifndef LIB
#define LIB

#define HOST "127.0.0.1"
#define PORT1 10000
#define PORT2 10001
#define TYPE1		1
#define TYPE2		2
#define TYPE3		3
#define ACK_TYPE1	4
#define ACK_TYPE2	5
#define ACK_TYPE3	6
#define NACK_TYPE	7
#define ACK_T1		"ACK(TYPE1)"
#define ACK_T2		"ACK(TYPE2)"
#define ACK_T3		"ACK(TYPE3)"
#define NACK		"NACK(TYPE)"
#define MSGSIZE 	1400
#define FRAME_SIZE	1404
#define BITS_NO		8
#define PKTSIZE		1392
#define CONTROLSIZE	1390

typedef struct {
	int len;
  	char payload[MSGSIZE];
} msg;

typedef struct {
  	int type;
  	int sequence;
  	char payload[PKTSIZE];

} my_pkt;

typedef struct {
	int type;
	int sequence;
	char control_byte1;
	char control_byte2;
	char payload[CONTROLSIZE];
} control_pkt;

void init(char* remote,int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
int recv_message_timeout(msg *m, int timeout);

/*intoarce bitul din poztia pos dintr-un byte*/
int getBit(char c, int pos) {
	c >>= pos;
	int bit = c & 0x00000001;
	return bit;
}


/*compara doi bytes*/
bool equal(char a, char b) {
	int i;
	
	for(i = 0; i < 8; i++) {
		if(getBit(a, i) != getBit(b, i)) {
			return false;
		}
	}
	
	return true;
}

/*intoarce byte-ul de control al unei structuri*/
char getControlByte(char* payload, int count) {
	char byte = 0x00000000;
	int i;
	
	for(i = 0; i < count; i++) {
		byte ^= payload[i];
	}
	
	return byte;
}

#endif

