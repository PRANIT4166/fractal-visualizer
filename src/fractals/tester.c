#include <stdio.h>
#include <stdlib.h>
#include "MPFR_Complex_Math.h"

/*
    gcc tester.c MPFR_Complex_Math.c -lmpfr -lgmp -o tester.exe
*/ 


int main(void) {
    var_prop prop;
    temp_var temps;
    var_c z1, z2, z3;
    mpfr_t absval;
    // -------- Context Setup --------
    if (set_var_prop(&prop, 256, MPFR_RNDN) != SUCCESS) {
        printf("Failed to set properties\n");
        return EXIT_FAILURE;
    }
    if (set_temp_var(&temps, &prop) != SUCCESS) {
        printf("Failed to init temp vars\n");
        return EXIT_FAILURE;
    }
    if (init_var_c(&z1, &prop) != SUCCESS ||
        init_var_c(&z2, &prop) != SUCCESS ||
        init_var_c(&z3, &prop) != SUCCESS) {
        printf("Failed to init complex vars\n");
        return EXIT_FAILURE;
    }

    mpfr_t root2;
    mpfr_init2(root2, prop.precision);

    // Compute sqrt(2)
    mpfr_sqrt_ui(root2, 2, prop.round_mode);

    printf("Testing MPFR sqrt(2)...\n");
    mpfr_printf("sqrt(2) == %.50Rf\n", root2); // print 50 digits

    // Check difference against known high-precision value
    const char* ref_str = "1.4142135623730950488016887242096980785696718753769"; // 50 digits
    mpfr_t ref, diff;
    mpfr_init2(ref, prop.precision);
    mpfr_init2(diff, prop.precision);

    mpfr_set_str(ref, ref_str, 10, prop.round_mode);
    mpfr_sub(diff, root2, ref, prop.round_mode);

    mpfr_printf("Difference = %.5Re\n", diff);

    // Cleanup
    mpfr_clear(root2);
    mpfr_clear(ref);
    mpfr_clear(diff);



    return EXIT_SUCCESS;
   
}
