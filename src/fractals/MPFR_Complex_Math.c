#include "MPFR_Complex_Math.h"
#include <stdio.h>
#include <stdlib.h>
// -------- Error codes -----------

const char* strerror(status err){
    switch (err)
    {
    case SUCCESS:
        return "SUCCESS";
    case NULL_POINTER:
        return "NULL POINTER ISSUE";
    case ERR_INIT:
        return "INITIALISATION FAILED";
    case ERR_NO_MEM:
        return "MALLOC FAILED";
    case ERR_MATH:
        return "MATH ERROR";   
    case FAILED_AT_ITER:
        return "Failed at Iteration";
    case ERR_TRUNCATED:  
        return "STRING BUFFER TRUNCATED";
    default:
        return "ERR_UNKNOWN";
    }
}

// --------- WS initialising -----

var_prop* create_var_prop(const mpfr_prec_t prec, const mpfr_rnd_t rnd){

    if (prec <= 0) return NULL;

    var_prop* prop = malloc(sizeof(var_prop));
    if (!prop) return NULL;

    prop->precision = prec;
    prop->round_mode = rnd;

    return prop;
}
void destroy_var_prop(var_prop* prop) {
    if (prop) free(prop);
}
temp_var* create_temp_var(const var_prop* prop) {

    if (!prop) return NULL;

    temp_var* temps = malloc(sizeof(temp_var));
    if (!temps) return NULL;

    mpfr_init2(temps->temp1, prop->precision);
    mpfr_init2(temps->temp2, prop->precision);
    mpfr_init2(temps->temp3, prop->precision);
    mpfr_init2(temps->temp4, prop->precision);

    return temps;
}
void destroy_temp_var(temp_var* temps) {

    if (temps) {
        mpfr_clear(temps->temp1);
        mpfr_clear(temps->temp2);
        mpfr_clear(temps->temp3);
        mpfr_clear(temps->temp4);
        free(temps);
    }
}

// ----------- complex number initiliasing -------------

var_c* create_var_c(const var_prop* prop){

    if (!prop) return NULL;

    var_c* z = malloc(sizeof(var_c));
    if (!z) return NULL;

    mpfr_init2(z->r, prop->precision);
    mpfr_init2(z->im, prop->precision);

    return z;   
}   
void destroy_var_c(var_c* z) {

    if (z) {
        mpfr_clear(z->r);
        mpfr_clear(z->im);
        free(z);
    }
}
status set_var_c(var_c *z_dst, const var_c *z_src, const var_prop *prop){

    if(!z_dst || !z_src || !prop) return NULL_POINTER;

    mpfr_set(z_dst->r, z_src->r, prop->round_mode);
    mpfr_set(z_dst->im, z_src->im, prop->round_mode);

    return SUCCESS;

}
status set_var_c_ano(var_c *z_dst, const mpfr_t real, const mpfr_t img, const var_prop *prop){

    if(!z_dst || !prop) return NULL_POINTER;

    mpfr_set(z_dst->r, real, prop->round_mode);
    mpfr_set(z_dst->im, img, prop->round_mode);

    return SUCCESS;
}
status set_var_c_str(var_c *z_dst, const char* real, const char* img, const var_prop *prop){

    if(!z_dst || !prop || !real || !img) return NULL_POINTER;

    mpfr_set_str(z_dst->r, real, 10, prop->round_mode);
    mpfr_set_str(z_dst->im, img, 10, prop->round_mode);

    return SUCCESS;
}
status set_var_c_zero(var_c *z){

    if(!z) return NULL_POINTER;

    mpfr_set_zero(z->r, 0);
    mpfr_set_zero(z->im, 0);

    return SUCCESS;
}

//----------- math ---------------------------
status add_c(var_c *z_dst, const var_c *z1, const var_c *z2, const var_prop *prop){

    if(!z_dst || !z1 || !z2 || !prop) return NULL_POINTER;

    mpfr_add(z_dst->r, z1->r, z2->r, prop->round_mode);
    mpfr_add(z_dst->im, z1->im, z2->im, prop->round_mode);

    return SUCCESS;

}
status sub_c(var_c *z_dst, const var_c *z1, const var_c *z2, const var_prop *prop){

    if(!z_dst || !z1 || !z2 || !prop) return NULL_POINTER;

    mpfr_sub(z_dst->r, z1->r, z2->r, prop->round_mode);
    mpfr_sub(z_dst->im, z1->im, z2->im, prop->round_mode);

    return SUCCESS;
}
status mult_c(var_c *z_dst, const var_c *z1, const var_c *z2, const var_prop *prop, temp_var *temps){

    if(!z_dst || !z1 || !z2 || !prop || !temps) return NULL_POINTER;

    mpfr_mul(temps->temp1, z1->r, z2->r, prop->round_mode);
    mpfr_mul(temps->temp2, z1->im, z2->im, prop->round_mode);
    mpfr_mul(temps->temp3, z1->r, z2->im, prop->round_mode);
    mpfr_mul(temps->temp4, z1->im, z2->r, prop->round_mode);

    mpfr_sub(z_dst->r, temps->temp1, temps->temp2, prop->round_mode);
    mpfr_add(z_dst->im, temps->temp3, temps->temp4, prop->round_mode);

    return SUCCESS;    
}
status sqr_c(var_c *z_dst, const var_c *z_src, const var_prop *prop, temp_var *temps){

    if(!z_dst || !z_src || !prop || !temps) return NULL_POINTER;

    mpfr_sqr(temps->temp1, z_src->r, prop->round_mode);
    mpfr_sqr(temps->temp2, z_src->im, prop->round_mode);
    mpfr_mul(temps->temp3, z_src->r, z_src->im, prop->round_mode);
    mpfr_mul_ui(temps->temp4, temps->temp3, 2, prop->round_mode);

    mpfr_sub(z_dst->r, temps->temp1, temps->temp2, prop->round_mode);
    mpfr_set(z_dst->im, temps->temp4, prop->round_mode);

    return SUCCESS;
}
status conj_c(var_c *z_dst, const var_c *z_src, const var_prop *prop){

    if(!z_dst || !z_src || !prop) return NULL_POINTER;

    mpfr_set(z_dst->r, z_src->r, prop->round_mode);
    mpfr_neg(z_dst->im, z_src->im, prop->round_mode);

    return SUCCESS;
}
status abs_c(mpfr_t dst, const var_c *z_src, const var_prop *prop, temp_var *temps){

    if(!dst || !z_src || !prop || !temps) return NULL_POINTER;

    mpfr_sqr(temps->temp1, z_src->r, prop->round_mode);
    mpfr_sqr(temps->temp2, z_src->im, prop->round_mode);
    mpfr_add(temps->temp3, temps->temp1, temps->temp2, prop->round_mode);

    mpfr_sqrt(dst, temps->temp3, prop->round_mode);

    return SUCCESS;
} 

// ---------- debugs ------------------------
status print_c(const var_c *z, const var_prop *prop){

    if(!z || !prop) return NULL_POINTER;

    mpfr_printf("(%Rg, %Rg)\n", z->r, z->im);   

    return SUCCESS;
}
status print_c_prec(const var_c *z, const var_prop *prop, const int digits){

    if(!z || !prop) return NULL_POINTER;
    if(digits<=0) return ERR_MATH;

    // mpfr_printf("testing 1 with : %.70Rg , %.70Rg \n", z->r, z->im);
    mpfr_printf("(%.*Rg, %.*Rg)\n", digits, z->r, digits, z->im);


    return SUCCESS;
}
status c_to_str(char *buf, size_t n, const var_c *z, const var_prop *prop, const int digits){

    if(!buf || !z || !prop) return NULL_POINTER;
    if(n == 0 || digits <= 0) return ERR_MATH;

    // mpfr_snprintf returns number of chars that WOULD have been written, or negative on error.
    int need = mpfr_snprintf(buf, n, "(%.*Rg, %.*Rg)", digits, z->r, digits, z->im);
    if (need < 0) return ERR_UNKNOWN;

    // ADD THIS CHECK: If the needed space is >= the buffer size, the string was cut off.
    if ((size_t)need >= n) return ERR_TRUNCATED;

    return SUCCESS;
}



