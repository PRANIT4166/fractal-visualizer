#include "mandelbrot.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define pixel(buffer, x, y, W) buffer[((size_t)y * (size_t)W) + (size_t)x]

margins_val_mpfr* create_margins_mpfr(const var_prop* prop) {

    if (!prop) return NULL;

    margins_val_mpfr* margins = malloc(sizeof(margins_val_mpfr));
    if (!margins) return NULL;
    
    mpfr_inits2(prop->precision, margins->dx, margins->x_max, margins->x_min, margins->x,
                                 margins->dy, margins->y_max, margins->y_min, margins->y, NULL); // FIXED
    return margins;
}
status clear_margins(margins_val_mpfr *margins)
{
    mpfr_clears(margins->dx, margins->x_max, margins->x_min, margins->x,
                margins->dy, margins->y_max, margins->y_min, margins->y, NULL);
    
    return SUCCESS;
}
void destroy_margins_mpfr(margins_val_mpfr* margins) {

    if (margins) {
        clear_margins(margins);
        free(margins);
    }
}

status init_pixel(pixel_prop *pixel, const var_prop* prop)
{
    mpfr_init2(pixel->smooth_iter, prop->precision);
    return SUCCESS;
}

status clear_pixel(pixel_prop *pixel)
{
    // to be called later
    mpfr_clear(pixel->smooth_iter);
    return SUCCESS;
}

status run_smooth(pixel_prop *pixel, const int max_level, mpfr_t *val, const int level, const var_prop *prop, temp_var *temps)
{

    if(level<max_level)
    {
        if(mpfr_cmp_ui(*val, 1) <=0 ) return ERR_MATH;
        mpfr_log(temps->temp1, *val, prop->round_mode);          // log(|z|) -> t1
        mpfr_log(temps->temp2, temps->temp1, prop->round_mode);  // log(log(|z|)) ->t2

        mpfr_set_si(temps->temp3, level + 1, prop->round_mode);     // n+1 ->t3
        mpfr_div_d(temps->temp4, temps->temp2, log(2.0), prop->round_mode); // log(log(|z|)) / log(2) ->t4
        mpfr_sub(temps->temp1, temps->temp3, temps->temp4, prop->round_mode);

        mpfr_set(pixel->smooth_iter, temps->temp1, prop->round_mode);
    }
    else mpfr_set_si(pixel->smooth_iter, level, prop->round_mode);

    return SUCCESS;
}

status mandelbrot_iter(var_c *z, const var_c *c, const int max_level, pixel_prop *pixel, const var_prop *prop, temp_var *temps)
{
    if(!z ||!c || !pixel || !prop || !temps) return NULL_POINTER;

    int level = 0;

    mpfr_t val;
    mpfr_init2(val,prop->precision);

    while(level < max_level)
    {

        sqr_c(z,z,prop,temps);
        add_c(z,z,c,prop);
        abs_c(val,z,prop,temps);
        if(mpfr_cmp_ui(val, 2) > 0) break;
        level++;
    }

    pixel->escape_level = level;

    status smooth_stat = run_smooth(pixel, max_level, &val, level, prop, temps);

    if(smooth_stat != SUCCESS) return FAILED_AT_ITER;

    mpfr_clear(val);

    return SUCCESS;
}


status transform(const margins_val *margins, margins_val_mpfr *new_margins, const var_prop *prop, temp_var *temps)
{
    mpfr_set_str(new_margins->y_max, margins->y_max, 10, prop->round_mode);
    mpfr_set_str(new_margins->x_max, margins->x_max, 10, prop->round_mode);
    mpfr_set_str(new_margins->x_min, margins->x_min, 10, prop->round_mode);
    mpfr_set_str(new_margins->y_min, margins->y_min, 10, prop->round_mode);

    mpfr_sub(temps->temp1, new_margins->x_max, new_margins->x_min, prop->round_mode);
    mpfr_sub(temps->temp2, new_margins->y_max, new_margins->y_min, prop->round_mode);

    mpfr_div_ui(new_margins->dx, temps->temp1, margins->width, prop->round_mode);
    mpfr_div_ui(new_margins->dy, temps->temp2, margins->height, prop->round_mode);

    return SUCCESS;
}

status mandelbrot_comp(pixel_prop *map, const margins_val *margins, const int max_level)
{
    if(!map || !margins) return NULL_POINTER;

    var_prop* prop = NULL;
    var_c* z = NULL;
    var_c* c = NULL;
    temp_var* temps = NULL;
    margins_val_mpfr* new_margins = NULL;
    status status_ = SUCCESS;

    //------------------ init all -------------------------------------
    prop = create_var_prop(256, MPFR_RNDN);
    if (!prop) { fprintf(stderr, "ERROR: Failed to create var_prop workspace.\n"); status_ = ERR_NO_MEM; goto cleanup; }

    z = create_var_c(prop);
    if (!z) { fprintf(stderr, "ERROR: Failed to create complex variable 'z'.\n"); status_ = ERR_NO_MEM; goto cleanup; }

    c = create_var_c(prop);
    if (!c) { fprintf(stderr, "ERROR: Failed to create complex variable 'c'.\n"); status_ = ERR_NO_MEM; goto cleanup; }

    temps = create_temp_var(prop);
    if (!temps) { fprintf(stderr, "ERROR: Failed to create temp_var workspace.\n"); status_ = ERR_NO_MEM; goto cleanup; }

    new_margins = create_margins_mpfr(prop);
    if (!new_margins) { fprintf(stderr, "ERROR: Failed to create margins_mpfr.\n"); status_ = ERR_NO_MEM; goto cleanup; }

    // Transform coordinates
    if (transform(margins, new_margins, prop, temps) != SUCCESS) {
        fprintf(stderr, "ERROR: Failed during coordinate transformation.\n");
        status_ = ERR_UNKNOWN;
        goto cleanup;
    }

    printf("Margin Transformation Complete\n");

    for (unsigned int i = 0; i < margins->width * margins->height; ++i) {
        init_pixel(&map[i], prop);
    }

    for (unsigned int py = 0; py < margins->height; py++) {
        for (unsigned int px = 0; px < margins->width; px++) {

            if(set_var_c_zero(z) != SUCCESS) return ERR_UNKNOWN;

            mpfr_mul_ui(temps->temp1, new_margins->dx, px, prop->round_mode);
            mpfr_mul_ui(temps->temp2, new_margins->dy, py, prop->round_mode);
            mpfr_add(new_margins->x, new_margins->x_min, temps->temp1, prop->round_mode);
            mpfr_sub(new_margins->y, new_margins->y_max, temps->temp2, prop->round_mode);
            
            set_var_c_ano(c, new_margins->x, new_margins->y, prop);

            status status_final = mandelbrot_iter(z, c, max_level, &pixel(map, px, py, margins->width), prop, temps);
            if (status_final != SUCCESS) {
                fprintf(stderr, "ERROR: Iteration failed at pixel (%u, %u).\n", px, py);
                status_ = status_final;
                goto cleanup;
            }
        }
    }
cleanup:

    if (prop) {
        for (unsigned int i = 0; i < margins->width * margins->height; ++i) {
            clear_pixel(&map[i]);
        }
    }
    destroy_var_prop(prop);
    destroy_var_c(z);
    destroy_var_c(c);
    destroy_temp_var(temps);
    destroy_margins_mpfr(new_margins);

    return status_;
}