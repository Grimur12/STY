#include "parseint.h"

/*
 * Returns the value of c or -1 on error
 */
int convertDecimalDigit(char c)
{
    (void)c;
    // First check if the char c is a single decimal digit so from 0-9
    if (c >= '0' && c <= '9')
    {
        // 1 converted to int is 49, 2 = 50, 3 = 51, 0 = 48, which means we can take the difference of the converted number and 0 to get the real number
        return (int) c - (int) '0';
    }
    // If it does not receive a valid char it returns -1 as intended
    return -1;
}

/*
 * Parses a non-negative integer, interpreted as octal when starting with 0,
 * decimal otherwise. Returns -1 on error.
 */
int parseInt(char *string)
{
    (void)string;
    // A valid octal number should have a leading zero so we will start by filtering those  that start with 0
    // converting them as octals, otherwise we call convertDecimalDigit which returns -1 if not valid so we dont need to check if its a decimal or not
    int result = 0; // final number to return
    if (string[0] == '0')
    {
        static int octalbase = 8; // To convert the octal
        while (*string != '\0') // Check for the null terminator to signify the end of the string
        // To convert octal to decimal, they represent numbers that have eight as the base
        // (547) would be 5*8^2 + 4*8^1 + 7*8^0 
        {
            int decimalConversion = convertDecimalDigit(*string);
            int octal = decimalConversion * (octalbase**);
            result += octal;
        }
        return result
    }
    else 
    {
        return convertDecimalDigit(string)
    }
    return -1;
}
