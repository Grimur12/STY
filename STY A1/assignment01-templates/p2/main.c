#include "testlib.h"
#include "parseint.h"

int main()
{
    test_start("parseint.c");

    test_equals_int(convertDecimalDigit('0'), 0);
    test_equals_int(parseInt("010"), 8);
    test_equals_int(parseInt("01"), 1);
    test_equals_int(parseInt("0"), 0);
    test_equals_int(parseInt("563"), 563);
    test_equals_int(parseInt("0563"), 371);
    test_equals_int(parseInt("10"), 10);
    test_equals_int(parseInt("?"), -1);

    return test_end();
}
