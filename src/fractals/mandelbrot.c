#include "mandelbrot.h"

#define pixel(buffer, x, y, W) buffer[(y*W) + x]

status init_margins(margins_val_mpfr *margins, var_prop *prop)
{
    mpfr_inits2(prop->precision, margins->dx, margins->x_max, margins->x_min, margins->x,
                                 margins->dy, margins->y_max, margins->y_min, margins->y);
    
    return SUCCESS;
}

status init_pixel()
{

}

status clear_pixel()
{

}

status clear_margins(margins_val_mpfr *margins)
{
    mpfr_clears(margins->dx, margins->x_max, margins->x_min, margins->x,
                margins->dy, margins->y_max, margins->y_min, margins->y);
    
    return SUCCESS;
}

status run_smooth(pixel_prop *pixel, const int max_level, mpfr_t *val, int level, var_prop *prop, temp_var *temps)
{
    if(mpfr_cmp_ui(val, 0) >0 ) return ERR_MATH;
    if(mpfr_cmp_ui(val, 1)>0 ) return ERR_MATH;

    if(level<max_level)
    {
        mpfr_log(temps->temp1, val, prop->round_mode);          // log(|z|) -> t1
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

    init_pixel(pixel, prop);
    pixel->escape_level = level;

    status smooth_stat = run_smooth(pixel, max_level, &val, level, prop, temps);

    if(smooth_stat != SUCCESS) return FAILED_AT_ITER;

    mpfr_clear(val);

    return SUCCESS;
}


status transform(margins_val *margins, margins_val_mpfr *new_margins, var_prop *prop, temp_var *temps)
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

status mandelbrot_comp(pixel_prop *map, const margins_val *margins, int max_level)
{
    if(!map || !margins) return NULL_POINTER;

    var_c z,c;
    temp_var temps;
    var_prop prop;
    margins_val_mpfr new_margins;

    //------------------ innit all ----------------------------

    status prop_status =  set_var_prop(&prop, 256, MPFR_RNDN);
    if ( prop_status != SUCCESS) {
        printf("Failed to set properties\n");
        return prop_status;
    }
    status c_status =  init_var_c(&c, &prop);
    if ( c_status != SUCCESS) {
        printf("Failed to set c\n");
        return c_status;
    }
    status z_status =  init_var_c(&z, &prop);
    if ( z_status != SUCCESS) {
        printf("Failed to set z\n");
        return z_status;
    }
    status temp_stat = set_temp_var(&temps, &prop);
    if ( temp_stat != SUCCESS) {
        printf("Failed to set z\n");
        return z_status;
    }

    status marg_status = init_margins(&new_margins, &prop);
    if ( marg_status != SUCCESS) {
        printf("Failed to set z\n");
        return marg_status;
    }
    // ---------------------------------------------

    if(transform(margins, &new_margins, &prop, &temps) != SUCCESS) return ERR_UNKNOWN;

    for (int py = 0; py < margins->height; py++) {
        for (int px = 0; px < margins->width; px++) {

            if(set_var_c_zero(&z) != SUCCESS) return ERR_UNKNOWN;

            mpfr_mul_ui(temps.temp1, new_margins.dx, px, prop.round_mode);
            mpfr_mul_ui(temps.temp2, new_margins.dy, py, prop.round_mode);
            mpfr_add(new_margins.x, new_margins.x_min, temps.temp1, prop.round_mode);
            mpfr_sub(new_margins.y, new_margins.y_max, temps.temp2, prop.round_mode);
            
            set_var_c_ano(&c, new_margins.x, new_margins.y, &prop);

            status final_status = mandelbrot_iter(&z, &c, max_level, &pixel(map, px, py, margins->width), &prop, &temps);
            if(final_status != SUCCESS) return FAILED_AT_ITER;
        }
    }

    clear_var_c(&c);
    clear_var_c(&z);
    clear_temp_var(&temps);
    clear_margins(&new_margins);

    return SUCCESS;
}