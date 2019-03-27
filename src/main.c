#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>

#include "spy.h"
#include "helpers.h"

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Use spy-files /path/to/spy/dir1 [/path/to/spy/dir2 ...]\n");

		return 1;
	}

	int i, j, all_dirs, count;
	int inotify_fd = get_inotify_fd();
	char ** array_dirs = calloc(1024, sizeof(char));
	char * events = calloc(BUF_LEN, sizeof(char));

	if (inotify_fd < 0) {
		fprintf(stderr, "Error get inotify fd\n");

		return 1;
	}

	for (i = 1, j = 0; i < argc; ++i, ++j) {
		array_dirs[j] = strdup(argv[i]);
	}

	all_dirs = j;

	if (scan_dir(array_dirs, all_dirs, 1, 1) < 0) {
		fprintf(stderr, "Error scan dir\n");

		return 1;
	}

	if (add_watch(inotify_fd) < 0) {
		fprintf(stderr, "Error get watch wd\n");

		return 1;
	}

	while (1) {
		scan_dir(array_dirs, all_dirs, 0, 0);
	
		count = get_event(inotify_fd, events);
	
		prepare_event(count, events);
		bzero(events, BUF_LEN);
	}

	exit(EXIT_SUCCESS);
}