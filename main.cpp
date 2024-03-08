#include <Arduino.h>
#include <time.h>
#include "esp_system.h"
extern "C"{
  #include "NtruCrypto.h"
};

struct NTRU nt;   //16.2

void setup() {
  
  Serial.begin(115200);
  int coef_f[NTRU_N] = {-1, -1, -1, 1, 1, 1, 1, 1, -1};
  // int coef_g[NTRU_N] = {0};
  init_nt(&nt, NTRU_N, NTRU_p, NTRU_q);

  size_t origin_heap = esp_get_free_heap_size();
  size_t tmp_heap;
  int error[(1 << NTRU_N)] = {0};
  int broke_time = 0;
  int count = 0;
  int *cx = NULL;
  unsigned long start = millis();

  for(int g = 1; g <(1 << NTRU_N); g++){
    // int coef_g[NTRU_N] = {0, 1, 1, 1, 1, 0, 1, 1, 1};
    nt.key_gen(&nt, coef_f, g);

    for(int j = 0; j < (1 << NTRU_N); j++){
      for(int i = 0; i < (1 << NTRU_N); ++i){
          int *cx = nt.encrypt(&nt, i, j);
          int m = nt.decrypt(&nt, cx);
          if(m != i){
            // printf("decrypted error ! %5d r:%d \r\n", i, j);
            error[broke_time] = i;
            broke_time++;
            break;
          }
          tmp_heap = esp_get_free_heap_size();
          printf("%s, %4d | r:%3d | g:%3d | err:%3d M:%d\r\n","Val Pass", i, j, g, (m - i), (origin_heap - tmp_heap));
          free(cx);
          count++;
      }
      vTaskDelay(1 / portTICK_PERIOD_MS);
      // size_t free_heap = esp_get_free_heap_size();
      // printf("Free heap space: %d bytes\n", (origin_heap - free_heap));
    }    

    
  }
  
  
  printf(" %d time, cost %ld ms, average %f \r\n", count, millis() - start, float(millis() - start) / count);  //35ms



  // 取得剩餘的堆疊空間
  size_t free_stack = uxTaskGetStackHighWaterMark(NULL);
  printf("Free stack space: %d bytes, ", free_stack);

  // 取得剩餘的堆空間
  size_t free_heap = esp_get_free_heap_size();
  printf("Free heap space: %d bytes\n", free_heap);



  printf("---------------result-----------------\r\n");
  for(int i = 0; i < broke_time; ++i){
    printf("%d ", error[i]);
  }
  printf("\r\n");

}

void loop() {
  
}
