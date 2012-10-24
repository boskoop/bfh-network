/* 
 * File:   main.c
 * Author: burgc5
 *
 * Created on 28 September 2012, 12:51
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * 
 */
int main() {

    int fahr;
    for (fahr = 0; fahr <= 300; fahr = fahr + 20) {
        printf("%3d %6.1f\n", fahr, (5.0 / 9.0)*(fahr - 32));
    }


    return (EXIT_SUCCESS);

}

