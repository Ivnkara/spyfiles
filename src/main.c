#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>

#include "spy.h"
#include "helpers.h"

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Use spy-files /path/to/spy/dir\n");

		return 1;
	}

	int inotify_fd = get_inotify_fd();

	if (inotify_fd < 0) {
		fprintf(stderr, "Error get inotify fd\n");

		return 1;
	}

	int scan_dir_result = scan_dir((char *)argv[1], 1);

	if (scan_dir_result < 0) {
		fprintf(stderr, "Error scan dir\n");

		return 1;
	}

	int add_watch_result = get_watch_wd((char *)argv[1], inotify_fd);

	if (add_watch_result < 0) {
		fprintf(stderr, "Error get watch wd\n");

		return 1;
	}

	struct inotify_event * event;

	while (1) {
		scan_dir((char *)argv[1], 0);
		event = get_event(inotify_fd);
		prepare_event(event);
		bzero(event, EVENT_SIZE);
	}

	return 0;
}