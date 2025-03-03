#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define USR_MAX 64
#define TEXT_MAX 256
#define packed __attribute__((packed))

struct request_text {
  unsigned char request_type; //always equal 177, arbitrary
  char username[USR_MAX];
  char message[TEXT_MAX];
} packed;

char recBuff[BUFF_SIZE];

void parseIncoming(char* data);
void cooked_terminal(void);
int raw_terminal(void);

int main(int argc, char** argv){
  if (argc != 2){
    fprintf(stdout, "Usage: ./chat <username>\n");
    exit(EXIT_SUCCESS);
  }
  if (strlen(argv[1]) > USR_MAX){
    fprintf(stdout, "Username must be no greater than 64 characters long\n");
    exit(EXIT_SUCCESS);
  } 

  if (raw_terminal() != 0){
    fprintf(stdout, "Fatal input error\n");
    exit(EXIT_FAILURE);
  }
  atexit(cooked_terminal);

  //Connect to server
  int sockFile = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockFile == -1){
    fprintf(stdout, "Fatal error, unable to create socket: %d\n", errno);
    exit(EXIT_FAILURE);
  }
  struct hostent *host = gethostbyname("127.0.0.1");
  if (host == NULL){
    fprintf(stdout, "gethostbyname error\n");
    exit(EXIT_FAILURE);
  }

  int flags = fcntl(sockFile, F_GETFL, 0);
  fcntl(sockFile, F_SETFL, flags | O_NONBLOCK);

  //Server info
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(4747);
  servAddr.sin_addr = *((struct in_addr*)host->h_addr);
  bzero(&(servAddr.sin_zero), 8);
  int servSize;

  //Login
  struct request_text login;
  login.request_type = 177;
  strcpy(login.username, argv[1]);
  //strncpy(login.username, argv[1], USERNAME_MAX);
  char tmp[26] = "[has joined the chatroom]";
  strcpy(login.message, tmp);
  if (sendto(sockFile, &login, sizeof(login), 0, (struct sockaddr*)&servAddr, sizeof(struct sockaddr)) == -1){
    fprintf(stdout, "Error logging in: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  char* sendBuff = (char*)malloc(sizeof(char) * TEXT_MAX);
  bzero(sendBuff, TEXT_MAX);
  int sendPos = 0;

  socklen_t servLen = sizeof(servAddr);
  char incoming = -1;

  struct pollfd polls[2];
  polls[0].fd = sockFile;
  polls[1].fd = 0;
  polls[0].events = POLLIN;
  polls[1].events = POLLIN;
  int p;

  fprintf(stdout, "> ");
  //main loop
  while(true){
    if ((p = poll(polls, 2, 1000)) == -1) {
	    fprintf(stdout, "Poll error: %d\n", errno);
	    errno = 0; 
    }
    else if (polls[1].revents & POLLIN){
    incoming = fgetc(stdin);
    if (incoming != -1){
	if (incoming == 127){
	  fprintf(stdout, "\r\x1b[2K");
	  sendBuff[--sendPos] = 0;
	} else if (sendPos < TEXT_MAX && incoming != 10)
	sendBuff[sendPos++] = incoming;
    }
    if (incoming == 10){ //newline - send message
	fprintf(stdout, "\r\x1b[2K");
	struct request_text req;
	req.request_type = 0x000000b1;
	//strncpy(req.message, sendBuff, TEXT_MAX);
	strcpy(req.username, argv[1]);
	strcpy(req.message, sendBuff);
	if (sendto(sockFile, &req, sizeof(req), 0, (struct sockaddr*)&servAddr, sizeof(struct sockaddr)) == -1){
	  fprintf(stdout, "Error sending message: %d\n", errno);
	  errno = 0;
	}
	bzero(sendBuff, TEXT_MAX);
	sendPos = 0;
	fprintf(stdout, "\r\x1b[2K");
    }
    incoming = -1;
    fprintf(stdout, "\r> %s", sendBuff);
    fflush(stdout);
    } else if (polls[0].revents & POLLIN){
    servSize = recvfrom(sockFile, recBuff, BUFF_SIZE, 0, (struct sockaddr*)&servAddr, &servLen);
    if (servSize > 0) {
      //recBuff[servSize] = '\0';
      parseIncoming(recBuff);
      fprintf(stdout, "\r> %s", sendBuff);
    }}
    fflush(stdout);
  }
  if (sendBuff != NULL) free(sendBuff);
  exit(EXIT_SUCCESS);
}

void parseIncoming(char* data){
  unsigned char head = data[0];
  fprintf(stdout, "\r\x1b[2K");
  //printf("head: %d\n", head);
  switch (head){
    case 177: //display text
    {
      char usr[USR_MAX];
      strcpy(usr, data+1);
      char msg[TEXT_MAX];
      strncpy(msg, data+65);
      fprintf(stdout, "%s says: %s\n", usr, msg);
    }
    break;
    default: //invalid
      fprintf(stdout, "[Client]: ERROR: Invalid packet recieved!\n");
    break;
  }
}

struct termios origTerm;

void cooked_terminal(){
  tcsetattr(STDIN_FILENO, TCSANOW, &origTerm);
}

int raw_terminal(){
  struct termios t;
  if (tcgetattr(STDIN_FILENO, &t) != 0) return -1;
  //copy current termios to revert back to
  origTerm = t; 
  //turn of echoing and line-based input
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;
  t.c_lflag = t.c_lflag & ~(ECHO) & ~(ICANON);
  tcsetattr(STDIN_FILENO, TCSADRAIN, &t);
  return 0;
}
