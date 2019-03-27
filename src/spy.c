#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <time.h>
#include <unistd.h>

#include "helpers.h"
#include "list_files.h"
#include "spy.h"

/**
 * Получить дескриптор inotify
 */
int get_inotify_fd()
{
	int fd = inotify_init();

	if (fd < 0) {
		perror("Error inotify_init");

		return -1;
	}

	return fd;
}

/**
 * Функция начинает следить за директорией
 */
int add_watch(int inotify_fd)
{
	int wd, i;

	for (i = 0; i < count_list_dirs; ++i) {
		wd = inotify_add_watch(inotify_fd, list_dirs[i], IN_MODIFY | IN_CREATE | IN_DELETE);

		if (wd < 0) {
			perror("Error inotify add watch: ");

			return -1;
		}
	}

	return 0;
}

/**
 * Инициирует структуру inotify_event, и отслеживает события в директории
 */
int get_event(int inotify_fd, char * buf)
{
	int i = 0;
	int len = read(inotify_fd, buf, BUF_LEN);

	if (len < 0 ) {
		perror("Error read inotify_fd");

		return 0;
	}

	int count = 0;
	
	while (i < len) {
		struct inotify_event * event = (struct inotify_event * ) &buf[i];
		i += EVENT_SIZE + event->len;
		count++;
	}

	return count;
}

/**
 * Обработка события
 */
int prepare_event(int count, char * events)
{
	int bytes = 0;
	for (int i = 0; i < count; ++i) {
		struct inotify_event * event = (struct inotify_event * ) &events[bytes];
		
		if (event->len) {
			switch (event->mask & (IN_MODIFY | IN_CREATE | IN_DELETE)) {
			case IN_MODIFY:
				if (event->mask | IN_ISDIR) {
					check_file(event->name);
				}
				break;
			case IN_CREATE:
				if (event->mask | IN_ISDIR) {
					add_file_to_scan(event->name);
				}
				break;
			case IN_DELETE:
				if (event->mask | IN_ISDIR) {
					remove_file_to_scan(event->name);
				}
			}
		}
		
		bytes += EVENT_SIZE + event->len;
	}

	return 0;
}
