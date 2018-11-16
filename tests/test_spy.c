#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "../src/spy.h"
#include "../utils/minunit.h"

char * tempdir_for_event;
char * tempfile_for_event;

char * tempdir_for_filesize;
char * tempfile_for_filesize;

int inotify_fd;

void * wr_file(void * which_file)
{
	sleep(1);
	int fd;

	if (*which_file == 1) {
		fd = open(tempfile_for_event, O_CREAT | O_WRONLY | O_TRUNC, 0777);
	} else if (*which_file == 2) {
		fd = open(tempfile_for_filesize, O_CREAT | O_WRONLY | O_TRUNC, 0777);
	}

	if (fd < 0) {
		perror("Open on tests fail: ");
	}

	char test_line[] = "data for spy tests";
	write(fd, test_line, 32);
	close(fd);

	return NULL;
}

static char * test_get_inotify_fd()
{
	inotify_fd = get_inotify_fd();

	mu_assert("---> ERROR, get_inotify_fd returns error", inotify_fd > 0);

	return 0;
}

static char * test_get_watch_wd()
{
	mu_assert("---> ERROR, get_watch_wd returns error", get_watch_wd(tempdir_for_event, inotify_fd) > 0);

	return 0;
}

static char * test_get_event()
{
	char expected[] = "tmp.txt";

	int arg = 1;
	pthread_t wr_file_thread;
	pthread_create(&wr_file_thread, NULL, wr_file, &arg);

	struct inotify_event * event = get_event(inotify_fd);

	mu_assert("---> ERROR, get_event returns wrong name file", strcmp(expected, event->name) == 0);

	pthread_join(wr_file_thread, NULL);

	return 0;
}

static char * test_scan_dir()
{
	mu_assert("---> ERROR, scan_dir returns not zero", scan_dir(tempdir_for_event, 0) == 0);

	return 0;
}

static char * test_check_filesize()
{
	char expected[] = "-----> Размер файла tmp.txt увеличился на 19 байт\n";
	int old_stdout = dup(1);
	int p[2];
	pipe(p);
	dup2(p[1], 1);

	char buf[1024];
	char *ptr = buf;

	int arg = 2;
	pthread_t wr_file_thread;
	pthread_create(&wr_file_thread, NULL, wr_file, &arg);

	int result = check_filesize(tempdir_for_filesize);

	do {
		read(p[0], ptr, 1);
		ptr++;
	} while(ptr[-1] != '\n');

	fflush(stdout);

	printf("%s\n", buf);

	mu_assert("---> ERROR, start_spy return not zero", 0 == result);
	mu_assert("---> ERROR, stdout not equals expected", strcmp(expected, buf) == 0);

	pthread_join(wr_file_thread, NULL);
	*ptr = 0;

	dup2(old_stdout, 1);
	close(old_stdout);
	close(p[0]);
	close(p[1]);

	return 0;
}

int tests_run = 0;

static char * all_tests()
{
	mu_run_test(test_get_inotify_fd);
	mu_run_test(test_get_watch_wd);
	mu_run_test(test_get_event);
	mu_run_test(test_scan_dir);
	mu_run_test(test_check_filesize);

	return 0;
}

int main()
{
	char tmp[] = "/tmp/spytest-XXXXXX";

	tempfile_for_event = calloc(64, sizeof(char));
	tempdir_for_event = calloc(64, sizeof(char));
	tempfile_for_filesize = calloc(64, sizeof(char));
	tempdir_for_filesize = calloc(64, sizeof(char));
	
	tempdir_for_event = mkdtemp(tmp);
	tempdir_for_filesize = mkdtemp(tmp);
	
	strcpy(tempfile_for_event, tempdir_for_event);
	strcat(tempfile_for_event, "/tmp.txt");
	strcpy(tempfile_for_filesize, tempdir_for_filesize);
	strcat(tempfile_for_filesize, "/tmp.txt");

	printf("Create temp directory for test: %s\n", tempdir_for_event);
	printf("Create path temp file for test: %s\n", tempfile_for_event);
	printf("Create temp directory for test: %s\n", tempdir_for_filesize);
	printf("Create path temp file for test: %s\n", tempfile_for_filesize);

	char * result = all_tests();

	if (result != 0) {
		printf("%s\n", result);
	} else {
		printf("ALL TESTS PASSED\n");
	}

	printf("Tests run: %d\n", tests_run);

	return 0;
}
