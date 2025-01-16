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

    // Base case, if string is empty its invalid
    if (*string == '\0')
    {
        return -1;
    }

    int n = 0; // We will use this to count the length of the string.
    char *temp = string; // Using this to find out the length of the string for octal calculation
    while (*temp != '\0')
    {
        n++; 
        temp++;
    }

    // A valid octal number should have a leading zero so we will start by filtering those  that start with 0
    int result = 0; // final number to return

    if (string[0] == '0') // If octal...
    {
        static int octalbase = 8; // To convert the octal

        for (int i = 1; i < n; i++) // We know how long the string is so iterate over it to get each individual char, we will start at 1, assuming that the leading 0 is not relevant and only there to tell us that its an octal number
        // To convert octal to decimal, they represent numbers that have eight as the base
        // (547) would be 5*8^2 + 4*8^1 + 7*8^0, we will call this 8^x power multiplier
        {
            int powerMultiplier = 1;
            for (int j = 0; j < (n-i-1); j++) // j < (n - i - 1) because if octal is 564, n = 3, and we want 5 * 8^2 down to 8^0 for the last number.. 
            {
                powerMultiplier *= octalbase; // We do this to get the correct 8^x to multiply the converted digit with
            }
            int decimalConversion = convertDecimalDigit(string[i]);

            if (decimalConversion < 0 || decimalConversion >= 8) // octal numbers are on the range 0 - 7, if the converted number is not, then this is an invalid number
            {
                return -1;
            }

            int octal = decimalConversion * powerMultiplier;
            result += octal;
        }
        return result;
    }
    else // Else its a decimal
    {
        static int decimalBase = 10; // We need to multiply the number with 10 because its decimal
        for (int i = 0; i < n; i++)
        {
            int decimalConversion = convertDecimalDigit(string[i]); // No further needs to check if its valid, convertDecimalDigit does that
            if (decimalConversion == -1 )
            {
                return -1; // Adding the return statement for invalid decimals
            }
            result = result * decimalBase + decimalConversion;
            // 931, i = 0, result = 9, i = 1, result = 90 + 3 = 93, i = 2, result = 930 + 1 = 931.
        }
        return result;
    }
    return -1;
}
