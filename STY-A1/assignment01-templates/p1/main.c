#include "testlib.h"
#include "print.h"

int main()
{
    test_start("print.c");

    // Add some more test code here.
    print_line(42, "Hello World!");
    // Extra
    print_line(12, "Does this work?");
    print_line(-1, "What about this?");

    return test_end();
}
