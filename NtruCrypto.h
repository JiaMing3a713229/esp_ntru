#ifndef _NTRUCRYPTO_H_
#define _NTRUCRYPTO_H_
#include <stdint.h>
#define NTRU_N 9                                            /*  N is prime   */
#define NTRU_p 3                                            /* gcd(N, q) = 1 */
#define NTRU_q 251                                          /* gcd(N, p) = 1 */

#define VALIDATION_MODE 1
#define ESP32_MODULO 1
#define _get_max_mun(NUM1, NUM2)({                                          \
    int ret = 0;                                                            \
    if(NUM1 >= NUM2){                                                       \
        ret = NUM1;                                                         \
    }                                                                       \
    else{                                                                   \
        ret = NUM2;                                                         \
    }                                                                       \
    ret;                                                                    \
})          

#define MAX_NUMBER ({                                                       \
    int max_number = 1;                                                     \
    max_number = (1 << NTRU_N);                                             \
    max_number;                                                             \
}) 

#define CENTERED_ZERO(NUMBER, MODULO)                                       \
({                                                                          \
    int num = NUMBER % MODULO;                                              \
    if (num >= ((MODULO + 1) / 2)) {                                        \
        num -= MODULO;                                                      \
    } else if (num <= ((-MODULO - 1) / 2)) {                                \
        num += MODULO;                                                      \
    }                                                                       \
    num;                                                                    \
})                                                            

#define sizeof_poly(POLY) ((&POLY) -> poly -> size_)

struct PolyObj{

    char poly_name[10];
    int *coef;
    int buf_size;
    int degree;

};

struct NTRU{

    int (*key_gen)(struct NTRU *nt, int *coef_f, int coef_g);
    int *(*encrypt)(struct NTRU *self, int num, int randnum);
    int (*decrypt)(struct NTRU *nt,int *self);
    struct Parameter{
        int N;
        int p;
        int q;
        struct PolyObj *fx;
        struct PolyObj *gx;
        struct PolyObj *ring;
        struct PolyObj *Fp;
        struct PolyObj *Fq;
        struct PolyObj *Kp;
    }params;

    uint8_t key_gen_flag;

    int (*poly)(struct PolyObj *self, const char *name, int coef[]);
    int (*ring)(struct PolyObj *self, const char *name);

    void (*print)(struct PolyObj *self);
    void (*println)(struct PolyObj *self);
    void (*free)(struct PolyObj *self);
};

int init_nt(struct NTRU *self, int N, int p, int q);

#endif 