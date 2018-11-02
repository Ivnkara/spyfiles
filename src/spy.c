#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include "spy.h"
#include "helpers.h"

int start_spy(char * path, int daemon)
{
	int spy_dir_result,
		fd, wd;

	fd = inotify_init();

	if (fd < 0) {
		perror("Error inotify_init");

		return -1;
	}

	wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);

	do {
		spy_dir_result = spy_dir(fd);

		if (spy_dir_result != 0) {
			perror("Error spy_dir");

			return -1;
		}
	} while(daemon);

	inotify_rm_watch(fd, wd);
	close(fd);

	return 0;
}

int spy_dir(int fd)
{
	char buf[BUF_LEN];
	int len = read(fd, buf, BUF_LEN);

	if (len < 0 ) {
		perror("Error read fd");

		return -1;
	}

	struct inotify_event * event = (struct inotify_event *) &buf[0];

	if (event->len) {
		print_event(event);
		switch (event->mask & (IN_MODIFY | IN_CREATE | IN_DELETE)) {
			case IN_MODIFY:
				printf("-----> Объект модифицирован\n");
				break;
			case IN_CREATE:
				printf("-----> Создан новый объект\n");
				break;
			case IN_DELETE:
				printf("-----> Удалён объект\n");
				break;
		}
	}

	null_buffer((char *)buf, BUF_LEN);

	return 0;
}
