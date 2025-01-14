#include "greet.h"
#include <stdio.h>

void greet(int32_t times) {
	(void) times;
	scanf("%d", &times); // "%d" for integers, and &times to access the memory location of times to write to
	for (int i = 1; i <= times; i++) 
	{
		printf("%d Hello World!\n", i);
	}

}