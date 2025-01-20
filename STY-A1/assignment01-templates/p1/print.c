// grimurg21@ru.is  - sigurdurn21@ru.is
#include "print.h"
#include <inttypes.h>
#include <stdio.h> 

void print_line(int64_t number, char *string)
{
    (void) number;
    (void) string;
    // inttypes.h defines PRI for fprintf() and fwprintf() family of functions
    // We want to print out the 64 bit signed int number,
    // PRIdN, d to represent the int and N will be the size of the int, in our case 64 bit
    // PRId64.
    printf("%" PRId64 " %s\n", number, string);
    // After printing out the int we have one whitespace before printing out the string and then a newline to finish
    
}
