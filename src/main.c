#include <stdio.h>
#include <stdlib.h>
#include "spy.h"

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Не передан путь до директории\n");

		return 1;
	}

	int result;

	result = start_spy((char *)argv[1], 1);

	if (result != 0) {
		printf("Error start spy\n");
	}

	return 0;
}