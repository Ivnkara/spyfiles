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
 *
 * @return int inotify_fd
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
 *
 * TODO: нужно сделать чтобы сканирование происходило после каждого события, а не только при начале слежки
 *
 * @param  char * path       Путь до директории за которой нужно следить
 * @param  int    inotify_fd Путь до директории за которой нужно следить
 * @return int               0 - при успешном завершении, -1 - при ошибке
 */
int get_watch_wd(char * path, int inotify_fd)
{
	int wd = inotify_add_watch(inotify_fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);

	if (wd < 0) {
		perror("Error inotify add watch: ");

		return -1;
	}

	return wd;
}

/**
 * Инициирует структуру inotify_event, и отслеживает события в директории
 *
 * @param  int inotify_fd Дескриптор inotify_init
 * @return int    		  0 - успех, -1 - ошибка
 */
struct inotify_event * get_event(int inotify_fd)
{
	char buf[BUF_LEN];
	int len = read(inotify_fd, buf, BUF_LEN);

	if (len < 0 ) {
		perror("Error read inotify_fd");

		return NULL;
	}

	struct inotify_event * event = (struct inotify_event * ) &buf;

	return event;
}

/**
 * Обработка события
 *
 * @param  struct inotify_event * event событие
 * @return int                          0 - успех, -1 - ошибка
 */
int prepare_event(struct inotify_event * event)
{
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
					// делаем что-нибудь
				}
		}
	}

	return 0;
}
