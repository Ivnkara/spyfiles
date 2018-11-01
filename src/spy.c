#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include "spy.h"

void null_buffer(char * buffer, int size) {
	for (int i = 0; i < size; ++i) {
		buffer[i] = 0;
	}
}

void print_event(struct inotify_event * event) {
	printf("Name:   %s\n", event->name);
	printf("WD:     %d\n", event->wd);
	printf("Mask:   %d\n", event->mask);
	printf("Cookie: %d\n", event->cookie);
	printf("Length: %d\n", event->len);
}

int start_spy(char * path)
{
	int fd, wd, len, i = 0;
	char buf[BUF_LEN];

	fd = inotify_init();

	if (fd < 0) {
		perror("Error inotify_init");

		return -1;
	}

	wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);

	while(1) {
		len = read(fd, buf, BUF_LEN);

		if (len < 0 ) {
			perror("Error read fd");

			return -1;
		}

		struct inotify_event * event = (struct inotify_event *) &buf[i];

		if (event->len) {
			print_event(event);
			if (event->mask & IN_CREATE) {
				printf("-----> Создан новый объект\n");
			} else if (event->mask & IN_DELETE) {
				printf("-----> Удалён объект\n");
			} else if (event->mask & IN_MODIFY) {
				printf("-----> Объект модифицирован\n");
			}
		}
		null_buffer((char *)buf, BUF_LEN);
		// i += EVENT_SIZE + event->len;
	}

	inotify_rm_watch(fd, wd);
	close(fd);

	return 0;
}