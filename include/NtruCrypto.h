#ifndef _NTRUCRYPTO_H_
#define _NTRUCRYPTO_H_

#define NTRU_N 3                                            /*  N is prime   */
#define NTRU_p 3                                            /* gcd(N, q) = 1 */
#define NTRU_q 23                                          /* gcd(N, p) = 1 */

#define MAX_NUMBER ({                                                       \
    int max_number = 1;                                                     \
    max_number = (1 << NTRU_N);                                             \
    max_number;                                                             \
}) 

#define CENTERED_ZERO(NUMBER, MODULO)                                       \
({                                                                          \
    int num = NUMBER % MODULO;                                              \
    if (num > MODULO / 2) {                                                 \
        num -= MODULO;                                                      \
    } else if (num <= -MODULO / 2) {                                        \
        num += MODULO;                                                      \
    }                                                                       \
    num;                                                                    \
})                                                            

#define sizeof_poly(POLY) ((&POLY) -> poly -> size_)

struct PolyObj{

    char poly_name[10];
    int *coef;
    int size_;
    int degree;

};

struct NTRU{

    int *(*encrypt)(struct NTRU *self, int num);
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
    
    struct modOps{
        int (*gcdOf)(int a, int b);
        int (*invOfnum)(int num, int mod_size);
        
    }modops;
    
    struct Polyops{
        
        struct PolyObj* (*mulpoly)(struct PolyObj *a, struct PolyObj *b, int modulo_size, char *name);
        struct PolyObj* (*divpoly)(struct PolyObj *dividend, struct PolyObj *division, int mod_size); /* dividend / division */
        struct PolyObj* (*exgcdPoly)(struct PolyObj *a, struct PolyObj *b, int modulo_size, char *name);
        struct PolyObj* (*addpoly)(struct PolyObj *a, struct PolyObj *b, int modulo_size, char *name);
        struct PolyObj* (*subpoly)(struct PolyObj *a, struct PolyObj *b, int modulo_size);
    }polyops;

    int (*poly)(struct PolyObj *self, const char *name, int coef[]);
    int (*ring)(struct PolyObj *self, const char *name);

    void (*print)(struct PolyObj *self);
    void (*free)(struct PolyObj *self);
};

typedef struct PolyObj *poly_nt;



int init_nt(struct NTRU *self, int N, int p, int q);
int polycpy(struct PolyObj *dest, struct PolyObj *src, char *name);
int key_gen(struct NTRU *nt, int *coef_f, int *coef_g);
void testfun(struct NTRU *nt);


#endif 