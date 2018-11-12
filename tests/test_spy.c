#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "../src/spy.h"
#include "../utils/minunit.h"

#define PATH_DIR "./tmp/"
#define PATH_FILE "./tmp/tmp.txt"

void * wr_file()
{
	sleep(1);
	int fd = open(PATH_FILE, O_CREAT | O_WRONLY);

	if (fd < 0) {
		perror("Open on tests fail: ");
	}

	char test_line[] = "data for spy tests";
	write(fd, test_line, 20);
	close(fd);

	return NULL;
}

static char * test_start_spy()
{
	char expected_stdout[] = "Path: \t\t\tName: \tSize:\n";
	int old_stdout = dup(1);
	int p[2];
	pipe(p);
	dup2(p[1], 1);

	char buf[1024];
	char *ptr = buf;

	pthread_t wr_file_thread;
	pthread_create(&wr_file_thread, NULL, wr_file, NULL);

	int
		result   = start_spy(PATH_DIR, 0),
		expected = 0;

	do {
		read(p[0], ptr, 1);
		ptr++;
	} while(ptr[-1] != '\n');
	fflush(stdout);

	mu_assert("---> ERROR, start_spy return not zero", expected == result);
	mu_assert("---> ERROR, stdout not equals expected", strcmp(expected_stdout, buf) == 0);
	pthread_join(wr_file_thread, NULL);
	*ptr = 0;

	dup2(old_stdout, 1);
	close(old_stdout);
	close(p[0]);
	close(p[1]);

	return 0;
}

static char * test_spy_dir()
{
	char expected_stdout[] = "Path: \t\t\tName: \tSize:\n";
	int old_stdout = dup(1);
	int p[2];
	pipe(p);
	dup2(p[1], 1);

	char buf[1024];
	char *ptr = buf;

	pthread_t wr_file_thread;
	pthread_create(&wr_file_thread, NULL, wr_file, NULL);

	int fd, wd, result, expected = 0;

	fd 	   = inotify_init();
	wd     = inotify_add_watch(fd, PATH_DIR, IN_MODIFY | IN_CREATE | IN_DELETE);
	result = spy_dir(fd);

	do {
		read(p[0], ptr, 1);
		ptr++;
	} while(ptr[-1] != '\n');
	fflush(stdout);

	mu_assert("---> ERROR, spy_dir return not zero", expected == result);
	mu_assert("---> ERROR, stdout not equals expected", strcmp(expected_stdout, buf) == 0);

	inotify_rm_watch(fd, wd);
	close(fd);
	pthread_join(wr_file_thread, NULL);
	*ptr = 0;

	dup2(old_stdout, 1);
	close(old_stdout);
	close(p[0]);
	close(p[1]);

	return 0;
}

static char * test_scan_dir()
{
	int
		result   = scan_dir(PATH_DIR),
		expected = 0;

	mu_assert("---> ERROR, scan_dir return not zero", expected == result);

	return 0;
}

static char * test_check_filesize()
{
	int
		result   = check_filesize(PATH_FILE),
		expected = 0;

	mu_assert("---> ERROR, check_filesize return not zero", expected == result);

	return 0;
}

int tests_run = 0;

static char * all_tests()
{
	mu_run_test(test_start_spy);
	mu_run_test(test_spy_dir);
	mu_run_test(test_scan_dir);
	mu_run_test(test_check_filesize);

	return 0;
}

int main()
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
