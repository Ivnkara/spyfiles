#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../src/spy.h"
#include "../src/list_files.h"
#include "../utils/minunit.h"

char * tempdir_for_event;
char * tempfile_for_event;

char * tempdir_for_filesize;
char * tempfile_for_filesize;

void * wr_file(void * which_file)
{
	int fd;
	int type = *(int *)which_file;

	switch (type) {
	case 1:
		fd = open(tempfile_for_event, O_CREAT | O_WRONLY | O_TRUNC, 0777);
		break;
	case 2:
		fd = open(tempfile_for_filesize, O_CREAT | O_WRONLY | O_APPEND, 0777);
		break;
	}

	if (fd < 0) {
		perror("Open on tests fail: ");
	}

	char test_line[] = "data for spy tests";
	write(fd, test_line, 20);
	close(fd);

	*(int *)which_file = type;

	return which_file;
}

static char * test_get_inotify_fd()
{
	int inotify_fd = get_inotify_fd();

	mu_assert("---> ERROR, get_inotify_fd returns error", inotify_fd > 0);

	return 0;
}

static char * test_add_watch()
{
	int inotify_fd = get_inotify_fd();

	mu_assert("---> ERROR, add_watch returns error", add_watch(inotify_fd) == 0);

	return 0;
}

static char * test_get_event()
{
	char expected[] = "tmp.txt";
	int arg = 1;
	char * dirs[] = {tempdir_for_event};
	int inotify_fd = get_inotify_fd();
	char * events = calloc(BUF_LEN, sizeof(char));
	pthread_t wr_file_thread;
	void *ret;

	scan_dir(dirs, 1, 0, 1);
	add_watch(inotify_fd);
	
	pthread_create(&wr_file_thread, NULL, wr_file, &arg);
	get_event(inotify_fd, events);

	pthread_join(wr_file_thread, &ret);
	struct inotify_event * event = (struct inotify_event * ) &events[0];

	mu_assert("---> ERROR, get_event returns wrong name file", strcmp(expected, event->name) == 0);


	return 0;
}

static char * test_scan_dir()
{
	char * dirs[] = {tempdir_for_event};

	mu_assert("---> ERROR, scan_dir returns not zero", scan_dir(dirs, 1, 0, 1) == 0);

	return 0;
}

static char * test_check_file()
{
	char expected[] = "-----> Размер файла tmp.txt увеличился на 20 байт";
	int old_stdout = dup(1);
	int p[2];
	pipe(p);
	dup2(p[1], 1);
	char buf[100];
	char *ptr = buf;
	int arg = 2;
	char * dirs[] = {tempdir_for_filesize};
	
	pthread_t wr_file_thread1, wr_file_thread2;
	void * ret1, * ret2;

	pthread_create(&wr_file_thread1, NULL, wr_file, &arg);
	pthread_join(wr_file_thread1, &ret1);

	scan_dir(dirs, 1, 0, 1);

	pthread_create(&wr_file_thread2, NULL, wr_file, &arg);
	pthread_join(wr_file_thread2, &ret2);

	int result = check_file("tmp.txt");

	do {
		read(p[0], ptr, 1);
		ptr++;
	} while(ptr[-1] != '\n');

	ptr[-1] = '\0';
	fflush(stdout);
	*ptr = 0;

	dup2(old_stdout, 1);
	close(old_stdout);
	close(p[0]);
	close(p[1]);

	mu_assert("---> ERROR, start_spy return not zero", 0 == result);
	mu_assert("---> ERROR, stdout not equals expected", strcmp(expected, buf) == 0);

	return 0;
}

static char * test_print_changes_file()
{
	char expected[] = "-----> Новые данные в файле:";
	int old_stdout = dup(1);
	int p[2];
	pipe(p);
	dup2(p[1], 1);
	char buf[80];
	char *ptr = buf;
	int arg = 2;
	char * dirs[] = {tempdir_for_filesize}; 
	
	pthread_t wr_file_thread1, wr_file_thread2;
	pthread_create(&wr_file_thread1, NULL, wr_file, &arg);

	void * ret1, * ret2;
	pthread_join(wr_file_thread1, &ret1);

	scan_dir(dirs, 1, 0, 1);

	pthread_create(&wr_file_thread2, NULL, wr_file, &arg);
	pthread_join(wr_file_thread2, &ret2);

	print_changes_file(tempfile_for_filesize, 20);

	do {
		read(p[0], ptr, 1);
		ptr++;
	} while(ptr[-1] != '\n');

	ptr[-1] = '\0';
	fflush(stdout);
	*ptr = 0;

	dup2(old_stdout, 1);
	close(old_stdout);
	close(p[0]);
	close(p[1]);

	mu_assert("---> ERROR, stdout not equals expected", strcmp(expected, buf) == 0);

	return 0;
}

static char * test_add_file_to_scan()
{
	char expected[] = "-:-:-:-:-:-: Был создан и добавлен к списку файл под именем tmp_second.txt :-:-:-:-:-:-";

	mu_assert("---> ERROR, count_list_files wrong value", 1 == count_list_files);
	int old_stdout = dup(1);
	int p[2];
	pipe(p);
	dup2(p[1], 1);
	char buf[300];
	char *ptr = buf;

	int result = add_file_to_scan("tmp_second.txt");

	do {
		read(p[0], ptr, 1);
		ptr++;
	} while(ptr[-1] != '\n');

	ptr[-1] = '\0';
	fflush(stdout);

	*ptr = 0;
	dup2(old_stdout, 1);
	close(old_stdout);
	close(p[0]);
	close(p[1]);

	mu_assert("---> ERROR, stdout not equals expected", strcmp(expected, buf) == 0);
	mu_assert("---> ERROR, add_file_to_scan return not zero", 0 == result);
	mu_assert("---> ERROR, count_list_files wrong value", 2 == count_list_files);

	return 0;
}

static char * test_remove_file_to_scan()
{
	char expected[] = "-:-:-:-:-:-: Был удалён файл из списка отслеживани и директории tmp_second.txt :-:-:-:-:-:-";

	mu_assert("---> ERROR, count_list_files wrong value", 2 == count_list_files);
	int old_stdout = dup(1);
	int p[2];
	pipe(p);
	dup2(p[1], 1);
	char buf[300];
	char *ptr = buf;

	int result = remove_file_to_scan("tmp_second.txt");

	do {
		read(p[0], ptr, 1);
		ptr++;
	} while(ptr[-1] != '\n');

	ptr[-1] = '\0';
	fflush(stdout);

	*ptr = 0;
	dup2(old_stdout, 1);
	close(old_stdout);
	close(p[0]);
	close(p[1]);

	mu_assert("---> ERROR, stdout not equals expected", strcmp(expected, buf) == 0);
	mu_assert("---> ERROR, remove_file_to_scan return not zero", 0 == result);
	mu_assert("---> ERROR, count_list_files wrong value", 1 == count_list_files);

	return 0;
}

void initTmp(char * tmp1, char * tmp2)
{
	tempfile_for_event = calloc(64, sizeof(char));
	tempdir_for_event = calloc(64, sizeof(char));
	tempfile_for_filesize = calloc(64, sizeof(char));
	tempdir_for_filesize = calloc(64, sizeof(char));

	tempdir_for_event = mkdtemp(tmp1);
	tempdir_for_filesize = mkdtemp(tmp2);

	strcat(tempdir_for_event, "/");
	strcat(tempdir_for_filesize, "/");
	
	strcpy(tempfile_for_event, tempdir_for_event);
	strcat(tempfile_for_event, "tmp.txt");
	strcpy(tempfile_for_filesize, tempdir_for_filesize);
	strcat(tempfile_for_filesize, "tmp.txt");

	printf("Create temp directory for test: %s\n", tempdir_for_event);
	printf("Create path temp file for test: %s\n", tempfile_for_event);
	printf("Create temp directory for test: %s\n", tempdir_for_filesize);
	printf("Create path temp file for test: %s\n", tempfile_for_filesize);
}

void removeTmp()
{
	int res1 = unlink(tempfile_for_event);
	int res2 = unlink(tempfile_for_filesize);

	if (res1 < 0 || res2 < 0) {
		perror("Unlink fail: ");
	}

	int res3 = rmdir(tempdir_for_event);
	int res4 = rmdir(tempdir_for_filesize);

	if (res3 < 0 || res4 < 0) {
		perror("Rmdir fail: ");
	}
}

int tests_run = 0;

static char * all_tests()
{
	mu_run_test(test_get_inotify_fd);
	mu_run_test(test_add_watch);
	mu_run_test(test_get_event);
	mu_run_test(test_scan_dir);
	mu_run_test(test_print_changes_file);
	mu_run_test(test_check_file);
	mu_run_test(test_add_file_to_scan);
	mu_run_test(test_remove_file_to_scan);

	return 0;
}

int main()
{
	char tmp1[] = "/tmp/spytest1-XXXXXX";
	char tmp2[] = "/tmp/spytest2-XXXXXX";
	initTmp(tmp1, tmp2);

	char * result = all_tests();

	if (result != 0) {
		printf("%s\n", result);
	} else {
		printf("ALL TESTS PASSED\n");
	}

	printf("Tests run: %d\n", tests_run);

	removeTmp();

	return 0;
}
