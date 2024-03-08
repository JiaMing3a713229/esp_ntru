#include "NtruCrypto.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


/*
 * This function calculates the greatest common divisor (GCD) of two integers.
 * It uses the Euclidean algorithm recursively to find the GCD.
 * Parameters:
 * - a: The first integer.
 * - b: The second integer.
 * Returns:
 * The greatest common divisor of 'a' and 'b'.
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

/*
 * This function calculates the modular multiplicative inverse of a number 'num'
 * modulo 'mod_t'. It first checks if the inverse exists by verifying that 'num' and
 * 'mod_t' are coprime. If they are not, it prints an error message and returns 0.
 * Parameters:
 * - num: The number for which the inverse is to be calculated.
 * - mod_t: The modulo value.
 * Returns:
 * The modular multiplicative inverse of 'num' modulo 'mod_t'.
 */

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

    return binary_array;
}

int dec2arr(int number, int arr[])
{   
    
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

    free(bin_array);
    return 1;
}



/*
 * This function prints the polynomial represented by the given 'PolyObj' structure.
 * Parameters:
 * - self: Pointer to the 'PolyObj' structure representing the polynomial.
 * Note: Assumes 'self' is not NULL.
 */

static void println_poly(struct PolyObj *self)
{   
    struct PolyObj *ptr = self;
    if(ptr == NULL){
        printf("error with print_poly memory allocate \r\n");
    }
    printf("PolyObj %s: ", ptr->poly_name);
    for (int i = 0; i < (ptr->buf_size - 1); ++i)
    {   
        printf("%4d ", *(ptr->coef + i));
    }
    printf("%4d ", *(ptr->coef + (ptr->buf_size - 1)));
    printf("| degree: %2d \r\n", self->degree);
}

static void print_poly(struct PolyObj *self)
{   
    struct PolyObj *ptr = self;
    if(ptr == NULL){
        printf("error with print_poly memory allocate \r\n");
    }
    printf("PolyObj %s: ", ptr->poly_name);
    for (int i = 0; i < (ptr->buf_size - 1); ++i)
    {   
        printf("%4d ", *(ptr->coef + i));
    }
    printf("%4d ", *(ptr->coef + (ptr->buf_size - 1)));
    printf("| degree: %2d |", self->degree);
}

static void check_PolyObj(struct PolyObj *self){
    if(self == NULL){
        printf("Error with PolyObj memory \r\n");
    }
    
    if(self -> coef == NULL){
        printf("Error with coef memory \r\n");
    }

    println_poly(self);
    printf("degree of %s : %d, buffer size: %d \r\n", self -> poly_name, self -> degree, self -> buf_size);
}

static void _free_poly(struct PolyObj *self)
{
    free(self->coef);
    free(self);

}

int init_poly(struct PolyObj *self, const char *name, int coef[])
{

    int degree = NTRU_N - 1;
    while (coef[degree] == 0 && degree > 0)
    {
        degree--;
    }

    struct PolyObj *ptr = self;
    ptr->coef = (int *)malloc(sizeof(int) * (NTRU_N));
    memset(ptr->coef, 0, sizeof(int) * (NTRU_N));
    if (ptr->coef == NULL)
    {
        return -1;
    }

    for (int i = 0; i <= degree; ++i)
    {
        *(ptr->coef + i) = coef[i];
    }

    self->buf_size = NTRU_N;
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
    self->buf_size = (arr_size == (NTRU_N + 1)) ? (arr_size):(-1); //檢查機制
    return 1;
}

/*
 * This function copies the contents of one polynomial represented by 'src'
 * to another polynomial represented by 'dest', and assigns a given name to 'dest'.
 * Parameters:
 * - dest: Pointer to the destination 'PolyObj' structure where the polynomial will be copied.
 * - src: Pointer to the source 'PolyObj' structure from which the polynomial will be copied.
 * - name: Pointer to the string containing the name to be assigned to the destination polynomial.
 * Returns:
 * - 1 if the copy operation is successful.
 * - -1 if there's an error with memory allocation or if either 'dest' or 'src' is NULL.
 */

int polycpy(struct PolyObj *dest, struct PolyObj *src, char *name)
{

    if (dest == NULL || src == NULL)
    {
        printf("error with allocate memory \r\n");
        return -1;
    }

    if(dest->coef == NULL | src->coef == NULL){
        printf("%s None initialize memory \r\n", dest->poly_name);
        return -1;
    }

    if(dest->buf_size != src->buf_size){
        dest->buf_size = src->buf_size;
        dest->coef = realloc(dest->coef, sizeof(int) * dest->buf_size);
    }

    for(int i = 0; i < src->buf_size; ++i){
        dest->coef[i] = src->coef[i];
    }

    strcpy(dest->poly_name, (const char*)name);
    dest->degree = src->degree;
    dest->buf_size = src->buf_size;
    return 1;
}

static struct PolyObj* mulpoly(struct PolyObj *a, struct PolyObj *b, int modulo_size, char *name)
{

    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj)); 

    int ret_coef[NTRU_N];
    memset(&ret_coef, 0, sizeof(ret_coef));
    
    for(int i =0; i <= (a->degree); ++i){
        for(int j = 0; j <= (b->degree); ++j){
            ret_coef[(i + j) % NTRU_N] += ((a->coef[i]) * (b->coef[j]));
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
    init_poly(ret, "", ret_coef);

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
    ret->buf_size = _get_max_mun(a->buf_size, b->buf_size);
    int ret_coef[ret->buf_size];
    memset(&ret_coef, 0, sizeof(ret_coef));

    for(int i = 0; i < ret->buf_size; ++i){
        
        ret_coef[i] = a->coef[i] - b->coef[i];
        ret_coef[i] = CENTERED_ZERO(ret_coef[i], modulo_size);
        
    }
    init_poly(ret, "", ret_coef); 
    return ret;
    
}


/*
** @brief dividend 是被除數
** @brief division 是除數
** return ret[0] 是商
** return ret[1] 是餘數
*/
static struct PolyObj *divpoly(struct PolyObj *dividend, struct PolyObj *division, int modulo_size)
{   
    // printf("***********div************\r\n");
    // print_poly(dividend);
    // print_poly(division);
    int init_coef[NTRU_N] = {0};
    struct PolyObj *ret = (struct PolyObj*)malloc((sizeof(struct PolyObj)) * 2);
    struct PolyObj *tmp_dptr = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    struct PolyObj *tmp_sptr = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    init_poly(tmp_dptr, "tmp_dpter", init_coef);
    init_poly(tmp_sptr, "tmp_spter", init_coef);

    polycpy(tmp_dptr, dividend, "tmp_dptr");
    polycpy(tmp_sptr, division, "tmp_sptr");

    int inv_s = invOfnum(tmp_sptr->coef[tmp_sptr->degree], modulo_size);
    int deg_q = (tmp_dptr->degree - tmp_sptr->degree);

    int q_arr[NTRU_N];
    memset(q_arr, 0, sizeof(q_arr));

    while(tmp_dptr->degree >= tmp_sptr->degree){

        int q_coef = (tmp_dptr->coef[tmp_dptr->degree]) * inv_s;
        for(int i = 0; i <= tmp_sptr->degree; ++i){
            tmp_dptr->coef[i + deg_q] -= (tmp_sptr->coef[i] * q_coef);
            tmp_dptr->coef[i + deg_q] = CENTERED_ZERO(tmp_dptr->coef[i + deg_q], modulo_size);
        }
        // print_poly(tmp_dptr);
        if(deg_q >= 0){
            q_arr[deg_q] = CENTERED_ZERO(q_coef, modulo_size);
            deg_q--;  
        }
        
        while((tmp_dptr->coef[tmp_dptr->degree]) == 0){
            tmp_dptr->degree--;
        }

        
    }

    if(tmp_dptr->degree < 0){
        tmp_dptr->degree++;
    }

    init_poly(&ret[0], "quotient", q_arr);
    init_poly((ret + 1), "reminder", init_coef);
    polycpy(&ret[1], tmp_dptr, "reminder");
    
    _free_poly(tmp_dptr);
    _free_poly(tmp_sptr);
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
    struct PolyObj d;
    struct PolyObj tmp_a;
    struct PolyObj tmp_b;
    struct PolyObj d1;
    struct PolyObj d2;
    struct PolyObj r;
    struct PolyObj q;
    
    
    int init_coef[NTRU_N] = {0};
    int d2_coef[NTRU_N] = {0};
    dec2arr(1, d2_coef);
    int q_coef[NTRU_N] = {0};
    dec2arr(1, q_coef);
    
    
    init_poly(&tmp_a, "tmp_a", init_coef);
    init_poly(&tmp_b, "tmp_b", init_coef);
    init_poly(&d1, "d1", init_coef);
    init_poly(&d2, "d2", d2_coef);
    init_poly(&r, "r", init_coef);
    init_poly(&q, "q", q_coef);
    init_poly(&d, "d", init_coef);

    polycpy(&tmp_a, a, "tmp_a");
    polycpy(&tmp_b, b, "tmp_b");

    // for(int i = 0; i < 3; i++){
    while(coef_sum(&r) != 1){

        struct PolyObj *div, *ptr_mul, *ptr_sub;
        div = divpoly(&tmp_b, &tmp_a, modulo_size);
        polycpy(&q, (div), "q");
        polycpy(&r, (div + 1), "r");

        ptr_mul = mulpoly(&q, &d2, modulo_size, "");
        ptr_sub = subpoly(&d1, ptr_mul, modulo_size);          // d = d1 - q * d2
        polycpy(&d, ptr_sub, "d");
        // println_poly(&d);
        // printf("-----------------------------------------\r\n");
        // print_poly(&tmp_b);
        // print_poly(&tmp_a);
        // print_poly(&q);
        // println_poly(&r);


        polycpy(&tmp_b, &tmp_a, "tmp_b");                                       // a = b;
        polycpy(&tmp_a, &r, "tmp_a");                                           // b = r;
        polycpy(&d1, &d2, "d1");                                                // d1 = d1
        polycpy(&d2, &d, "d2");                                                  // d2 = d

        // print_poly(&d);
        
        // printf("-----------------------------------------\r\n");
        // print_poly(mulpoly(a, &d1, modulo_size, ""));
        // print_poly(mulpoly(a, &d2, modulo_size, ""));

        _free_poly(ptr_mul);
        _free_poly(ptr_sub);
        free(div[1].coef);
        free(div[0].coef);
        free(div);
    }

    init_poly(ret, "", init_coef);
    polycpy(ret, &d, name);

    free(d.coef);
    free(tmp_a.coef);
    free(tmp_b.coef);
    free(d1.coef);
    free(d2.coef);
    free(r.coef);
    free(q.coef);

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

static int check_key(struct NTRU *nt){

    int ret = 1;
    struct PolyObj *val_Fp = mulpoly(nt->params.Fp, nt->params.fx, nt->params.p, "val_Fp");
    struct PolyObj *val_Fq = mulpoly(nt->params.Fq, nt->params.fx, nt->params.q, "val_Fq");
    struct PolyObj *tar = (struct PolyObj *)malloc(sizeof(struct PolyObj));

    int tar_coef[NTRU_N] = {0};
    dec2arr(1, tar_coef);
    init_poly(tar, "target", tar_coef);

#ifdef VALIDATION_MODE
    // println_poly(Fp);
    // println_poly(Fq);
#endif

    struct PolyObj *ptr_sub_fp = subpoly(tar, val_Fp, nt->params.p);
    struct PolyObj *ptr_sub_fq = subpoly(tar, val_Fq, nt->params.q);
    if(coef_sum(ptr_sub_fp) != 0){
        printf("Generating Kp failed \r\n");
        ret = -1;
    }
    if(coef_sum(ptr_sub_fq) != 0){
        printf("Generating Kq failed \r\n");
        ret = -1;
    }

    printf("Generating Key Successed \r\n");


    _free_poly(val_Fp);
    _free_poly(val_Fq);
    _free_poly(ptr_sub_fp);
    _free_poly(ptr_sub_fq);
    _free_poly(tar);

    return ret;
}

int key_gen(struct NTRU *nt, int *coef_f, int g_num){

    int coef_g[NTRU_N] = {0};
    dec2arr(g_num, coef_g);

    if(nt == NULL){
        return -1;
    }

    if(nt->params.Kp != NULL){
        free(nt->params.Kp);
        free(nt->params.Kp->coef);
        free(nt->params.gx->coef);
    }
    printf("-------------------key Generator step-------------------\r\n");

    nt->poly(nt->params.gx, "gx", coef_g);

    if(!nt->key_gen_flag){

        if(nt->poly(nt->params.fx, "fx", coef_f)){
            // nt->println(nt->params.fx);
        }

        if(nt->ring(nt->params.ring, "ringx")){
            // nt->println(nt->params.ring);
        }

        nt->params.Fp = exgcdPoly(nt->params.fx, nt->params.ring, nt->params.p, "Fp");
        nt->params.Fq = exgcdPoly((nt->params.fx), (nt->params.ring), (nt->params.q), "Fq");
        nt->key_gen_flag = 1;
    }

    nt->params.Kp = mulpoly(nt->params.Fq, nt->params.gx, nt->params.q, "kp");

    

    nt->println(nt->params.fx);
    nt->println(nt->params.ring);
    nt->println(nt->params.Fp);
    nt->println(nt->params.Fq);
    nt->println(nt->params.Kp);
    nt->println(nt->params.gx);

    if(check_key(nt) == -1){
        return -1;
    }
    printf("-------------------key-Generator-step-End---------------\r\n");
    return 1;
}

static int* encrypt(struct NTRU *self, int num, int randnum)   // length of array is N
{
    int randomval;
    int *ret = (int*)malloc(sizeof(int) * NTRU_N);
    struct PolyObj *ret_poly = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    
    struct PolyObj *mx = encoder(num, "m");
    struct PolyObj *rx = encoder(randnum, "rx");
    
    ret_poly = mulpoly(self->params.Kp, rx, self->params.q, "");
    for(int i = 0; i < NTRU_N; ++i){               
        ret_poly->coef[i] *= self->params.p;
        ret_poly->coef[i] = CENTERED_ZERO(ret_poly->coef[i], self->params.q);
    }

    struct PolyObj *ret_poly_ = addpoly(ret_poly, mx, self->params.q, "cx");

#ifdef VALIDATION_MODE
    // print_poly(mx);
    // println_poly(ret_poly);
#endif


    for(int i = 0; i < ret_poly->buf_size; ++i){
        ret[i] = ret_poly_->coef[i];
    }


    _free_poly(mx);
    _free_poly(rx);
    _free_poly(ret_poly);
    _free_poly(ret_poly_);

    return ret;
}

static int decrypt(struct NTRU *nt,int *self){

    int iret = 0;
    struct PolyObj *ax = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    struct PolyObj *ret = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    struct PolyObj *cx = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    nt->poly(cx, "cx", self);

    ax = mulpoly(cx, nt->params.fx, nt->params.q, "ax");
    ret = mulpoly(nt->params.Fp, ax, nt->params.p, "mx");

#ifdef VALIDATION_MODE
    print_poly(cx);
    print_poly(ret);
#endif

    iret = decoder(ret);

    _free_poly(ax);
    _free_poly(ret);
    _free_poly(cx);

    return iret;
}



int init_nt(struct NTRU *self, int N, int p, int q)
{

    if (self == NULL)
    {
        return -1;
    }
    self->key_gen_flag = 0;
    self->params.N = N;
    self->params.p = p;
    self->params.q = q;

    (self->params.fx) = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    (self->params.gx) = (struct PolyObj*)malloc(sizeof(struct PolyObj));
    (self->params.ring) = (struct PolyObj*)malloc(sizeof(struct PolyObj));

    self->params.Fp = NULL;
    self->params.Fq = NULL;
    self->params.Kp = NULL;


    self->poly = init_poly;
    self->ring = init_ring;

    self->print = print_poly;
    self->println = println_poly;
    self->free = _free_poly;

    self->encrypt = encrypt;
    self->decrypt = decrypt;
    self->key_gen = key_gen;
    
    return 1;
};

