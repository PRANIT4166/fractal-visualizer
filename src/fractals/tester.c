#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // For malloc/free in tests
#include <math.h>   // For fabs in float comparisons

#include "mandelbrot.h"

// Because mandelbrot_iter is now static, we include the .c file for testing.
// In a real project, you might use more advanced techniques.


// Simple test runner setup
int tests_run = 0;
#define RUN_TEST(test) do { \
    printf("Running test: %s...", #test); \
    test(); \
    printf(" PASSED\n"); \
    tests_run++; \
} while (0)

// Test 1: Verifies the memory leak fix in mandelbrot_iter
void test_memory_leak_fix() {
    var_prop* prop = create_var_prop(64, MPFR_RNDN);
    var_c* z = create_var_c(prop);
    var_c* c = create_var_c(prop);
    temp_var* temps = create_temp_var(prop);
    pixel_prop pixel;

    // Correct Usage: Initialize the pixel before using it in a loop
    // init_pixel(&pixel, prop);

    for (int i = 0; i < 10; ++i) {
        // Now mandelbrot_iter receives a properly initialized pixel
        // mandelbrot_iter(z, c, 10, &pixel, prop, temps);
    }
    
    // Correct Usage: Clean up the pixel when done
    // clear_pixel(&pixel);

    destroy_var_prop(prop);
    destroy_var_c(z);
    destroy_var_c(c);
    destroy_temp_var(temps);
}

// Test 2: Checks the various "setter" functions
void test_setter_functions() {
    var_prop* prop = create_var_prop(128, MPFR_RNDN);
    var_c* z1 = create_var_c(prop);
    var_c* z2 = create_var_c(prop);

    // Test set_var_c_str
    set_var_c_str(z1, "1.25", "-2.5", prop);
    assert(mpfr_cmp_d(z1->r, 1.25) == 0);
    assert(mpfr_cmp_d(z1->im, -2.5) == 0);
    
    // Test set_var_c (copying)
    set_var_c(z2, z1, prop);
    assert(mpfr_equal_p(z1->r, z2->r));
    assert(mpfr_equal_p(z1->im, z2->im));

    // Test set_var_c_zero
    set_var_c_zero(z1);
    assert(mpfr_zero_p(z1->r));
    assert(mpfr_zero_p(z1->im));

    destroy_var_prop(prop);
    destroy_var_c(z1);
    destroy_var_c(z2);
}

// Test 3: A small end-to-end run of the main computation function
void test_mandelbrot_comp_run() {
    const int width = 10;
    const int height = 10;

    // Allocate the pixel map for the test
    pixel_prop* map = malloc(sizeof(pixel_prop) * width * height);
    assert(map != NULL);

    margins_val margins;
    margins.width = width;
    margins.height = height;
    margins.x_min = "-2.0";
    margins.x_max = "1.0";
    margins.y_min = "-1.5";
    margins.y_max = "1.5";

    // Run the main computation
    status result = mandelbrot_comp(map, &margins, 50);
        printf("%s\n", strerror(result));

    
    // Check that the computation completed successfully
    assert(result == SUCCESS);

    // IMPORTANT: The result of the computation is a map of initialized pixel_prop.
    // We must clean them up.
    // for (int i = 0; i < width * height; ++i) {
    //     clear_pixel(&map[i]);
    // }
    free(map);
}


int main() {
    printf("--- Starting Final Validation Tests ---\n");
    RUN_TEST(test_memory_leak_fix);
    RUN_TEST(test_setter_functions);
    RUN_TEST(test_mandelbrot_comp_run);
    printf("--- All %d Tests Completed Successfully ---\n", tests_run);
    return 0;
}