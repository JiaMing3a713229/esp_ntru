#include <Arduino.h>
#include <time.h>
extern "C"{
  #include "NtruCrypto.h"
};

struct NTRU nt;   //16.2

void setup() {
  
  Serial.begin(115200);
  int coef_f[NTRU_N] = {1, 1, -1};
  int coef_g[NTRU_N] = {1, -1, 0};
  init_nt(&nt, NTRU_N, NTRU_p, NTRU_q); //18.8
  key_gen(&nt, coef_f, coef_g);

  
  unsigned long start = millis();
  for(int i = 0; i < (1 << NTRU_N); ++i){
        int *cx = nt.encrypt(&nt, i);
        int m = nt.decrypt(&nt, cx);
        if(m != i){
            printf("decrypted error !\r\n");
            break;
        }
  }
  printf("cost %ld ms \r\n", millis() - start);

  
}

void loop() {
  
}
