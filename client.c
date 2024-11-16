// Usage: ./client <IP_address> <port_number>

// The client attempts to connect to a server at the specified IP address and port number.
// The client should simultaneously do two things:
//     1. Try to read from the socket, and if anything appears, print it to the local standard output.
//     2. Try to read from standard input, and if anything appears, print it to the socket. 

// Remember that you can use a pthread to accomplish both of these things simultaneously.

/*
	Group Members
	Thomas Macas
	Charlie Wells
	Henry Barsanti
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

int globalFD;

void sh ( int signum ){ //this handles control C quitting, sends a pseudo quit message to other users
	FILE* sock = fdopen(globalFD, "w");
	fprintf(sock, "quit\n");
	fflush(sock);
	printf("Unexpected Quit\n");
	close(globalFD);
    exit(EXIT_FAILURE);
}

void* getMessages(void* arg){ // receives messages as a thread
	//printf("Started getter\n");
	char* message;
	int sockFD = *((int *)arg);
	FILE* sock = fdopen(sockFD, "r+");
	char outputBuffer[1024];
	while(1){
		message = fgets(outputBuffer, sizeof(outputBuffer), sock);
		//printf("Got here");
		if (message == NULL) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				//printf("here1");
				continue;
			} 
			else {
				printf("Server shut down unexpectedly\n");
				exit(EXIT_FAILURE);
			}
		}
		else{
			printf("%s", message);
			fflush(sock);
		}
	}
}

void* sendMessages(void* arg){ //sends messages as a thread
	//printf("Started sender\n");

	int sockFD = *((int *)arg);
	FILE* sock = fdopen(sockFD, "w");
	char inputBuffer[1024];
	while(1){
		if(fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL){
			continue;
		}
		printf("You: %s", inputBuffer);
	
		fprintf(sock, "%s\n", inputBuffer );
		fflush(sock);

		if(strncmp(inputBuffer, "quit", 4) == 0 && strlen(inputBuffer) == 5){ //handles quits
			shutdown(sockFD, SHUT_WR); 
            close(sockFD);
			exit(EXIT_FAILURE);
		}
	}
}


int main(int argc, char* argv[]) { //take in port and ip

	if (argc != 3){ 
		printf("Usage <port number> <IP address>\n");
		return 1;
	}
	if (atoi(argv[1])<1024 || atoi(argv[1]) > 65535){
		printf("not usuable port number, please use: 1024-65535\n");
		return 1;
	}


	int PORT_NUM = atoi(argv[1]); //Any random number 10000-60000
	char* CONNECT_ADDRESS = argv[2]; 

	signal(SIGINT, sh);

	// STEP 1 - Create socket endpoint
	int sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if( sockFD == -1 ){
		perror("Could not create socket");
		return -1;
	}	

	globalFD = sockFD;

	// STEP 2 - Connect to sever
	struct sockaddr_in address;
	memset(&address, 0, sizeof(struct sockaddr_in) );

	address.sin_family = AF_INET;
	address.sin_port   = htons( PORT_NUM );
	//address.sin_addr.s_addr = INADDR_ANY;
	inet_aton( CONNECT_ADDRESS, &address.sin_addr );

	int ret = connect( sockFD, (struct sockaddr*)&address, sizeof(struct sockaddr_in) );
	if( ret == -1 ){
		perror("Could not connect");
		return -1;
	}

	printf("Successfully connected.\n");

	FILE* sock = fdopen( sockFD, "r+" );

	pthread_t getThread;
	pthread_t sendThread;

	if(pthread_create(&getThread, NULL, getMessages, (void*)&sockFD) != 0){ //create threads
		perror("Couldnt create getter thread");
	}

	if(pthread_create(&sendThread, NULL, sendMessages, (void*)&sockFD) != 0){
		perror("Couldnt create sender thread");
	}

	pthread_join(sendThread, NULL);
	pthread_join(getThread, NULL);


	return 0;

}
