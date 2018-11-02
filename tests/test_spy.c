#include <stdio.h>
#include "../src/spy.h"
#include "../utils/minunit.h"

static char * test_start_spy() {
	int
		result   = start_spy("/tmp", 0),
		expected = 0;

	mu_assert("---> ERROR, start_spy return not zero", expected == result);

	return 0;
}

int tests_run = 0;

static char * all_tests() {
	mu_run_test(test_start_spy);

	return 0;
}

int main(int argc, char const *argv[])
{
	char * result = all_tests();

	if (result != 0) {
		printf("%s\n", result);
	} else {
		printf("ALL TESTS PASSED\n");
	}

	printf("Tests run: %d\n", tests_run);

	return 0;
}
