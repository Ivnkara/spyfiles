#include <stdio.h>
#include <stdlib.h>
#include "spy.h"

int main(int argc, char const *argv[])
{
	int result;

	result = start_spy("/tmp");

	if (result != 0) {
		printf("Error start spy\n");
	}

	return 0;
}