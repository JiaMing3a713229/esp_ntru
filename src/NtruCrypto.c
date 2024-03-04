#include "NtruCrypto.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


/*
** @brief this function is used to get gcd(a, b)
** return : the number of gcd(a,b)
*/
static int gcdOf(int a, int b)
{

    if (b > a)
    {
        int c = a;
        a = b;
        b = c;
    }

    if (b == 0)
    {
        return a;
    }
    return gcdOf(b, (a % b));
}

static int invOfnum(int num, int mod_t)
{
    num = (num < 0) ? num + mod_t : num;
    if ((num - mod_t) > 0)
    {
        num %= mod_t;
    }

    if (gcdOf(num, mod_t) != 1)
    {   
        printf("not exist inverse of %d mod %d \r\n", num, mod_t);
        return 0;
    }
    int mod_tmp = mod_t;
    int q_ = 0;
    int r_ = 0;
    int d1 = 0;
    int d2 = 1;
    int d = -1;

    while (num != 0)
    {
        q_ = mod_t / num;
        r_ = mod_t % num;
        mod_t = num;
        num = r_;
        d = d1 - (q_ * d2);
        d1 = d2;
        d2 = d;
    }

    return (d1 > 0) ? (d1) : (d1 + mod_tmp);
}

static int *dec2bin(int number, int bits)
{

    int index = bits - 1;

    int *binary_array = (int *)malloc(bits * sizeof(int));
    if (binary_array == NULL)
    {
        printf("error with allocate heap \r\n");
        return NULL;
    }

    while (number > 0 && index >= 0)
    {
        binary_array[index] = number % 2;
        number /= 2;
        index--;
    }

    // for(int i = 0; i < bits; ++i){
    //     printf("%d ", *(binary_array + i));
    // }
    // printf("\r\n");

    return binary_array;
}

int dec2arr(int number, int arr[])
{   
    
    // printf("max_number:%d \r\n", MAX_NUMBER);
    if (number > MAX_NUMBER)
    {
        printf("error with number \r\n");
        return -1;
    }else if(number == 0){
        memset(&arr[0], 0, sizeof(int) * NTRU_N);
        return 1;
    }

    int bits = 0;
    int temp = number;
    while (temp > 0)
    {
        bits++;
        temp /= 2;
    }
    int *bin_array = dec2bin(number, bits);

    int index = bits - 1;

    for (int i = 0; i < bits; ++i)
    {
        arr[index - i] = *(bin_array + i);
    }

    return 1;
}



///////////////////////////////////////////////////////////////////////////////////////////
static void print_poly(struct PolyObj *self)
{   
    struct PolyObj *ptr = self;
    if(ptr == NULL){
        printf("error with print_poly memory allocate \r\n");
    }
    printf("Polynomail %10s: ", ptr->poly_name);
    for (int i = 0; i < ptr->degree; ++i)
    {   
        printf("%3d ", *(ptr->coef + i));
    }
    printf("%3d \t", *(ptr->coef + (ptr->degree)));
    printf("degree: %3d \r\n", self->degree);
}

static void _free_poly(struct PolyObj *self)
{
    free(self->coef);
    self->coef = NULL;

}

int init_poly(struct PolyObj *self, const char *name, int coef[])
{
    // int degree = (sizeof(coef) / sizeof(coef[0]));
    int degree = (NTRU_N - 1);
    while (coef[degree] == 0 && degree > 0)
    {
        degree--;
    }

    int sizeof_poly = NTRU_N;
    struct PolyObj *ptr = self;
    ptr->coef = (int *)malloc(sizeof(int) * sizeof_poly);
    memset(ptr->coef, 0, sizeof(int) * sizeof_poly);
    // print_poly(self->poly);
    if (ptr->coef == NULL)
    {
        return -1;
    }

    for (int i = 0; i <= degree; ++i)
    {
        *(ptr->coef + i) = coef[i];
    }

    self->size_ = sizeof_poly;
    self->degree = degree;
    strcpy(ptr->poly_name, name);
    
    return 1;
}

int init_ring(struct PolyObj *self, const char *name)
{
    
    int coef_ring[NTRU_N + 1] = {0};
    coef_ring[0] = -1;
    coef_ring[NTRU_N] = 1;
    int degree = NTRU_N;
    self->coef = (int*)malloc(sizeof(int) * (NTRU_N + 1));
    while (coef_ring[degree] == 0 && degree > 0)
    {
        degree--;
    }

    for(int i = 0; i <= degree; ++i){
        self->coef[i] = coef_ring[i];
    }

    self->degree = degree;
    strcpy(self->poly_name, name);
    int arr_size = sizeof(coef_ring) / sizeof(coef_ring[0]);
    self->size_ = (arr_size == (NTRU_N + 1)) ? (arr_size):(-1); //檢查機制
    return 1;
}
/*
** dest 為複製目標 類型為struct Poly
** src  為參考來源 類型為struct Poly
*/
int polycpy(struct PolyObj *dest, struct PolyObj *src, char *name)
{

    if (dest == NULL || src == NULL)
    {
        printf("error with allocate memory \r\n");
        return -1;
    }

    struct PolyObj *sptr = src;
    struct PolyObj *dptr = dest;
    dptr->coef = (int *)malloc(sizeof(int) * (src->size_));   // the size of Polynomial
    if (dptr->coef == NULL || sptr->coef == NULL)
    {
        printf("error with polycpy");
        return -1;
    }

    for (int i = 0; i < sptr->size_; ++i)
    {
        *(dptr->coef + i) = *(sptr->coef + i);
    }

    strcpy(dptr->poly_name, (const char*)name);
    dptr->degree = sptr->degree;
    dptr->size_ = sptr->size_;

    return 1;
}

/*
** @brief dividend 是被除數
** @brief division 是除數
** return ret[0] 是商
** return ret[1] 是餘數
*/
static struct PolyObj *divpoly(struct PolyObj *dividend, struct PolyObj *division, int modulo_size)
{   
    
    // printf("-----------------\r\n");
    // printf("div:\r\n");
    // print_poly(dividend);
    // printf("division\r\n");
    // print_poly(division);


    struct PolyObj poly_div;
    polycpy(&poly_div, dividend, "tmp");
    struct PolyObj *div = &poly_div;

    if (dividend == NULL || division == NULL)
    {   
        printf("error with dividend or division  \r\n");
        return NULL;
    } // 例外處理

    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj) * 2);
    struct PolyObj *d_ptr = div;
    struct PolyObj *s_ptr = division;
    
    int q_degree = ((dividend->degree) - (division->degree));
    int q_coef[NTRU_N];
    memset(q_coef, 0, sizeof(q_coef));

    // int r_degree = (division->poly->degree - 1);
    int r_coef[NTRU_N];
    memset(r_coef, 0, sizeof(r_coef));

    int inv_divsion = invOfnum(division->coef[division->degree], modulo_size);
    int q_index = q_degree;
    while(d_ptr->degree >= s_ptr->degree){

        int q = (d_ptr -> coef[d_ptr->degree] * inv_divsion) % modulo_size;
        for(int i = 0; i <= s_ptr->degree; i++){
            d_ptr->coef[d_ptr->degree - i] -= (q * s_ptr->coef[s_ptr->degree - i]);
            d_ptr->coef[d_ptr->degree - i] %= modulo_size;
            d_ptr->coef[d_ptr->degree - i] = CENTERED_ZERO(d_ptr->coef[d_ptr->degree - i], modulo_size);
        }


        q_coef[q_index] = CENTERED_ZERO(q, modulo_size);
        q_index--;
        d_ptr->degree--;
        
    }
    
    if(d_ptr->degree < 0){
        d_ptr->degree++;
    }


    init_poly(&ret[0], "quotient", q_coef);
    polycpy(&ret[1], div, "reminder");

    // printf("quotient:\r\n");
    // print_poly(&ret[0]);
    // printf("reminder:\r\n");
    // print_poly(&ret[1]);
    // printf("---------------------\r\n");
    
    return ret;

}

static struct PolyObj* mulpoly(struct PolyObj *a, struct PolyObj *b, int modulo_size, char *name)
{

    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj)); 

    int ret_coef[NTRU_N];
    memset(&ret_coef[0], 0, sizeof(ret_coef));
    
    for(int i =0; i <= (a->degree); ++i){
        for(int j = 0; j <= (b->degree); ++j){
            ret_coef[(i + j) % NTRU_N] += (a->coef[i]) * (b->coef[j]);
            ret_coef[(i + j) % NTRU_N] %= modulo_size;
        }
    }

    for(int i = 0; i < NTRU_N; ++i){
        ret_coef[i] = CENTERED_ZERO(ret_coef[i], modulo_size);
    }
    
    
    init_poly(ret, name, ret_coef);
    return ret;
}


/*
** @return (a + b)
*/
static struct PolyObj* addpoly(struct PolyObj *a, struct PolyObj *b, int modulo_size, char *name)
{   
    
    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    int ret_coef[NTRU_N];
    memset(&ret_coef, 0, sizeof(ret_coef));

    // ret->poly->degree = (a->poly->degree > b->poly->degree) ? (a->poly->degree) : (b->poly->degree);

    if(a->degree > b->degree){
        polycpy(ret, a, name);
        for(int i = 0; i <= b->degree; ++i){
            ret->coef[i] += b->coef[i];
            ret->coef[i] %= modulo_size;
            ret->coef[i] = CENTERED_ZERO(ret->coef[i], modulo_size);
        }
    }
    else{
        polycpy(ret, b, name);
        for(int i = 0; i <= a->degree; ++i){
            ret->coef[i] += a->coef[i];
            ret->coef[i] %= modulo_size;
            ret->coef[i] = CENTERED_ZERO(ret->coef[i], modulo_size);
        }
    }

    return ret;

}

static struct PolyObj* subpoly(struct PolyObj *a, struct PolyObj *b, int modulo_size){

    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    int ret_coef[NTRU_N];
    memset(&ret_coef, 0, sizeof(ret_coef));

    for(int i = 0; i < NTRU_N; ++i){
        
        ret_coef[i] = (*(a->coef + i)) - (b->coef[i]);
        ret_coef[i] = CENTERED_ZERO(ret_coef[i], modulo_size);
        
    }
    
    init_poly(ret, "", ret_coef);
    return ret;
    
}
static int coef_sum(struct PolyObj *self)
{   
    int ret = 0;
    for(int i = 0; i < NTRU_N; ++i){
        ret += (self->coef[i]);
    }
    return ret;
}

/*
** @brief This function is used to retrieve the inverse of polynomial
** @brief the function is over (x^N - 1)
** @return Inverse of Poly (Type: struct Poly)
*/
static struct PolyObj* exgcdPoly(struct PolyObj *a, struct PolyObj *b, int modulo_size, char *name){

    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj));

    struct PolyObj *d;
    struct PolyObj tmp_a;
    struct PolyObj tmp_b;
    struct PolyObj d1;
    struct PolyObj d2;
    struct PolyObj r;
    struct PolyObj q;
    
    int d1_coef[NTRU_N];
    dec2arr(0, d1_coef);
    int d2_coef[NTRU_N];
    dec2arr(1, d2_coef);
    int r_coef[NTRU_N];
    dec2arr(1, r_coef);
    int q_coef[NTRU_N];
    dec2arr(1, q_coef);
    
    polycpy(&tmp_a, a, "tmp_a");
    polycpy(&tmp_b, b, "tmp_b");
    init_poly(&d1, "d1", d1_coef);
    init_poly(&d2, "d2", d2_coef);
    init_poly(&r, "r", r_coef);
    init_poly(&q, "q", q_coef);
    
    while(coef_sum(&tmp_a) != 0){
        
        struct PolyObj *div = divpoly(&tmp_b, &tmp_a, modulo_size);
        q = div[0];
        r = div[1];
        d = subpoly(&d1, mulpoly(&q, &d2, modulo_size, ""), modulo_size);       // d = d1 - q * d2
        polycpy(&tmp_b, &tmp_a, "tmp_b");                                       // a = b;
        polycpy(&tmp_a, &r, "tmp_a");                                           // b = r;
        polycpy(&d1, &d2, "d1");                                                // d1 = d1
        polycpy(&d2, d, "d2");                                                  // d2 = d
    }
    
    polycpy(ret, &d1, name);
    _free_poly(d);
    return ret;
    
}
/*
** @brief  This function is used to transfer num from int to PolyObj
** @return struct PolyObj
*/
static struct PolyObj *encoder(int num, char *name){

    int coef[NTRU_N] = {0};
    dec2arr(num, coef);

    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    init_poly(ret, name, coef);

    return ret;

}

static int decoder(struct PolyObj *self){

    int ret = 0;
    int *ptr = self->coef;
    for(int i = 0; i < NTRU_N; ++i){
        ret += ptr[i] * (1 << i);
    }

    return ret;

}

int key_gen(struct NTRU *nt, int *coef_f, int *coef_g){

    if(nt == NULL){
        return -1;
    }

    nt->params.Fp = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    nt->params.Fq = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    nt->params.Kp = (struct PolyObj*)malloc(sizeof(struct PolyObj));

    if(nt->poly(nt->params.fx, "fx", coef_f)){
        nt->print(nt->params.fx);
    }

    if(nt->poly(nt->params.gx, "gx", coef_g)){
        nt->print(nt->params.gx);
    }

    if(nt->ring(nt->params.ring, "ringx")){
        nt->print(nt->params.ring);
    }

    nt->params.Fp = nt->polyops.exgcdPoly(nt->params.fx, nt->params.ring, nt->params.p, "Fp");
    nt->print(nt->params.Fp);
    nt->params.Fq = nt->polyops.exgcdPoly((nt->params.fx), (nt->params.ring), (nt->params.q), "Fq");
    nt->print(nt->params.Fq);
    nt->params.Kp = nt->polyops.mulpoly(nt->params.Fq, nt->params.gx, nt->params.q, "kp");
    nt->print(nt->params.Kp);


    return 1;
}

static int* encrypt(struct NTRU *self, int num)   // length of array is N
{
    int randomval;
    int *ret = (int*)malloc(sizeof(int) * NTRU_N);
    struct PolyObj *ret_poly = (struct PolyObj*)malloc(sizeof(struct PolyObj));

    srand(time(NULL));
    randomval = rand() % MAX_NUMBER;
    randomval = rand() % MAX_NUMBER;
    // printf("randomval:%d \r\n", randomval);
    struct PolyObj *mx = encoder(num, "m");
    struct PolyObj *rx = encoder(randomval, "rx");
    
    // print_poly(mx);
    // print_poly(rx);

    ret_poly = self->polyops.mulpoly(self->params.Kp, rx, self->params.q, "");
    for(int i = 0; i < NTRU_N; ++i){               
        ret_poly->coef[i] *= self->params.p;
        ret_poly->coef[i] = CENTERED_ZERO(ret_poly->coef[i], self->params.q);
    }

    ret_poly = self->polyops.addpoly(ret_poly, mx, self->params.q, "cx");
    
    // print_poly(ret_poly);
    ret = ret_poly->coef;

    return ret;
}

static int decrypt(struct NTRU *nt,int *self){

    int iret = 0;
    struct PolyObj *ax = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    struct PolyObj *cx = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    nt->poly(cx, "cx", self);

    ax = nt->polyops.mulpoly(cx, nt->params.fx, nt->params.q, "ax");
    ret = nt->polyops.mulpoly(nt->params.Fp, ax, nt->params.p, "mx");
    // print_poly(ret);
    iret = decoder(ret);
    return iret;
}

int init_nt(struct NTRU *self, int N, int p, int q)
{

    if (self == NULL)
    {
        return -1;
    }

    self->params.N = N;
    self->params.p = p;
    self->params.q = q;

    (self->params.fx) = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    (self->params.gx) = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    (self->params.ring) = (struct PolyObj*)malloc(sizeof(struct PolyObj));

    self->modops.gcdOf = gcdOf;
    self->modops.invOfnum = invOfnum;

    self->poly = init_poly;
    self->ring = init_ring;
    self->polyops.divpoly = divpoly;
    self->polyops.mulpoly = mulpoly;
    self->polyops.exgcdPoly = exgcdPoly;
    self->polyops.addpoly = addpoly;
    self->polyops.subpoly = subpoly;

    self->print = print_poly;
    self->free = _free_poly;

    self->encrypt = encrypt;
    self->decrypt = decrypt;

    return 1;
};

void testfun(struct NTRU *nt){
}
