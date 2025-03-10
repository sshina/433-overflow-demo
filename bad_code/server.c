#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>

#define LISTEN_QUEUE_SIZE 10
#define BUFF_SIZE 1024
#define USR_MAX 64
#define TEXT_MAX 256
#define packed __attribute__((packed))

//request struct
struct request_text {
  unsigned char request_type; //always equal 177, arbitrary
  char username[USR_MAX];
  char message[TEXT_MAX];
} packed;

int sockFile;

int connectedPorts[10];
int connectedSize = 0;

int formatBuff(char* buffer){
  char b[512];
  strcpy(b, buffer);
  return 0;
}

void parseIncoming(char data[BUFF_SIZE], int port, unsigned char head){
  //First, parse 4 byte int header
  switch(head){
    case 177://say request
      {
	//check if login. this is not good, the chatroom will not connect new users
	//after 10 have logged in. but for demonstrating buffer overflows it doesnt matter
	if (strcmp("[has joined the chatroom]",(data+65)) == 0){
	  if (connectedSize < 10){
	    connectedPorts[connectedSize++] = port;
	  } 
	}
	//construct the packet
	struct request_text* res = (struct request_text*)malloc(sizeof(struct request_text));
	res->request_type = 177;
	//strncpy(res->username, username, USR_MAX);
	strcpy(res->username, (data+1));
	strcpy(res->message, (data+65));
	
	//send to all users
	struct sockaddr_in cli;
        cli.sin_family = AF_INET;
	cli.sin_addr.s_addr = inet_addr("127.0.0.1");
	for(int i = 0; i < connectedSize; ++i){
	  cli.sin_port = connectedPorts[i];  
	  sendto(sockFile, res, sizeof(struct request_text), 0, (struct sockaddr*)&cli, sizeof(struct sockaddr));
	}
        //for server display only
        printf("[%s]: %s\n", (data+1), (data+65));
      }
      break;
    default: //Invalid header
      fprintf(stdout, "ERROR INVALID HEADER: %u\n", head);
      break;
  }
}

int main(int argc, char** argv){
  if (argc != 1){
    fprintf(stdout, "Server program does not take any additional arguments, ignoring...\n");
  }
  while (argv[0]) break; //pointless
  errno = 0;
  
  //Open server
  sockFile = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockFile == -1){
    fprintf(stdout, "Fatal error, unable to create socket: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  //set up the server
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddr.sin_port = htons(4747);
  int flags = fcntl(sockFile, F_GETFL, 0);
  fcntl(sockFile, F_SETFL, flags | O_NONBLOCK);
  if (bind(sockFile, (const struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1){
    fprintf(stdout, "Fatal error binding socket: %d\n", errno);
    exit(EXIT_FAILURE);
  }
  
  //Get packets, main loop
  struct sockaddr_in cliAddr;
  socklen_t cliLen = sizeof(cliAddr);
  int cliSize;

  fd_set read_fds;
  struct timeval timeout;
  
  char buffer[512];

  while(true){
    FD_ZERO(&read_fds);
    FD_SET(sockFile, &read_fds);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int activity = select(sockFile + 1, &read_fds, NULL, NULL, &timeout);
    if ((activity < 0) && (errno != EINTR)){
      fprintf(stdout, "Select error: %d\n", errno);
    } else if (activity > 0){ //Handle incoming data
      if (FD_ISSET(sockFile, &read_fds)){
	cliSize = recvfrom(sockFile, buffer, BUFF_SIZE, 0, (struct sockaddr*)&cliAddr, &cliLen);
	if (cliSize > 0){
	  //buffer[cliSize] = '\0';
	  formatBuff(buffer);
	  parseIncoming(buffer, cliAddr.sin_port, buffer[0]);
	}else if ((cliSize < 0) && (errno != EWOULDBLOCK)){
	  fprintf(stdout, "Recieve error: %d\n", errno);
	}
      }
    }
    errno = 0;
  } 

  close(sockFile);
}
