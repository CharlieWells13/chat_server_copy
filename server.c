// Usage: ./server <port_number>

// This server should listen on the specified port, waiting for incoming connections.
// After a client connects, add it to a list of currently connected clients.
// If any message comes in from *any* connected client, then it is repeated to *all*
//    other connected clients.
// If reading or writing to a client's socket fails, then that client should be remove from the linked list. 

// Remember that blocking read calls will cause your server to stall. Instead, set your
// your sockets to be non-blocking. Then, your reads will never block, but instead return
// an error code indicating there was nothing to read- this error code can be either
// EAGAIN or EWOULDBLOCK, so make sure to check for both. If your read call fails
// with that error, then ignore it. If it fails with any other error, then treat that
// client as though they have disconnected.

// You can create non-blocking sockets by passing the SOCK_NONBLOCK argument to both
// the socket() function, as well as the accept4() function.


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
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


struct Client{ //to be saved in out linked list
	int socket;
	char name [32];
	struct Client* next;
};

struct Client* head = NULL;

void broadcast(struct Client* sender, char* message){ //sends messages out to other clients
	struct Client* it = head;
	while(it != NULL){
		if(it != sender){
			FILE* sock = fdopen(it->socket, "w");
			if(sock == NULL){
				printf("User disconnected");
			}
			else{
				//printf("Called broadcast on user %s\n", it->name);
				fprintf(sock, "%s: %s", sender->name, message);
				fflush(sock);
				//printf("%s: %s", sender->name, message);
			}
		}
		it = it->next;
	}
}

void addClient(int socket, int number){ //adds client to list and assigns them names
	struct Client* newHead = (struct Client*)malloc(sizeof(struct Client));
	newHead->socket = socket;
	newHead->next = head;
	sprintf(newHead->name, "User %d", number);
	broadcast(newHead, "Connected to the server\n");
	head = newHead;

}

void removeClient(struct Client* clientToRemove) { // rmeoves client from linked list and frees malloced memory
    if (clientToRemove == NULL) {
        return;
    }
    if (clientToRemove == head) {
        head = head->next;
    } 
	else {
        struct Client* prev = head;
        while (prev->next != NULL && prev->next != clientToRemove) {
            prev = prev->next;
        }
        if (prev->next == NULL) {
            return;
        }
        prev->next = clientToRemove->next;
    }
    close(clientToRemove->socket);
    free(clientToRemove);
}

int main(int argc, char* argv[]) { //take in port number

	if (argc != 2){
		printf("Usage <port number>\n");
		return 1;
	}
	if (atoi(argv[1])<1024 || atoi(argv[1]) > 65535){
		printf("not usuable port number, please use: 1024-65535\n");
		return 1;
	}

	int PORT_NUM = atoi(argv[1]); 

	// STEP 1 - Create socket endpoint
	int listenFD = socket(AF_INET, SOCK_STREAM, 0);
	if( listenFD == -1 ){
		perror("Could not create socket");
		return -1;
	}	

	if (fcntl(listenFD, F_SETFL, O_NONBLOCK) < 0){ //add nonblocker
		perror("failed to nonblocker");
		exit(EXIT_FAILURE);
	}

	// STEP 2 - Bind address to socket
	struct sockaddr_in address;
	memset(&address, 0, sizeof(struct sockaddr_in) );

	address.sin_family = AF_INET;
	address.sin_port   = htons( PORT_NUM );
	address.sin_addr.s_addr = INADDR_ANY;

	int ret = bind( listenFD, (struct sockaddr*)&address, sizeof(struct sockaddr_in) );
	if( ret == -1 ){
		perror("Could not bind address");
		return -1;
	}

	// STEP 3 - Set socket to listening mode
	// After this point, the OS will accept incoming connections and
	// wait for our program to handle them. The number of waiting connections
	// that are allowed is the size of the backlog in the second argument.
	ret = listen( listenFD, 5 );
	if( ret == -1 ){
		perror("Could not set socket to listening mode");
		return -1;
	}

	int visitors = 0;

	// Step 4 - Accept incoming connections on this socket. This is a blocking
	// call that causes this program to wait here until a connection attempt
	// is made.
	char* message;
	socklen_t addressSize = sizeof(address);
	while(1){	
		sleep(.1); //every 100ms
		int clientFD = accept4( listenFD, (struct sockaddr*)&address, &addressSize, SOCK_NONBLOCK);
		if (clientFD < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                //printf("no user connect\n");
				//continue;
            } 
			else {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }
        }
		else{
			visitors++;
			printf("Visitor %d connected\n", visitors);
			addClient(clientFD, visitors);
		}

		struct Client* it = head;
		
		while(it != NULL) { //iterating thru every client
			//printf("iterated to %s\n", it->name);
			FILE* sock = fdopen(it->socket, "r+");
			if (sock == NULL) {
				perror("failed to\n");
				it = it->next;
				continue;
			}
			
			char buffer[1024];

			message = fgets(buffer, sizeof(buffer), sock);
			
			if (message == NULL) {
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					//printf("no message came in\n");
					//sleep(1);
					it = it->next;
					continue;
				} 
        	}
			else if(strncmp(message, "name ", 5) == 0){ // handles name changes on server side
				char old_name[32];
				strcpy(old_name, it->name);

				char *newline = strchr(message + 5, '\n');
				if (newline != NULL) {
					*newline = '\0';
				}
				strcpy(it->name, message + 5); 
				char brodMess[1024];
				sprintf(brodMess, "%s changed name to %s\n", old_name, it->name);
				broadcast(it, brodMess);
			}
			else if(strncmp(message, "quit", 4) == 0 && strlen(message) == 5){ //handles quits on server side
				broadcast(it,"Has left the server\n");
				shutdown(it->socket, SHUT_RDWR);
				removeClient(it);
			}
			else{
				broadcast(it, message);

				printf("%s: %s", it->name, buffer);
			}
			it = it->next;
		}

	}
	return 0;
}

