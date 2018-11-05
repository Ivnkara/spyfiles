#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "spy.h"
#include "helpers.h"

int start_spy(char * path, int daemon)
{
	int spy_dir_result,
		fd, wd;

	scan_dir(path);

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
				if (event->mask & IN_ISDIR) {
					printf("-----> Директория модифицирована\n");
				} else {
					printf("-----> Файл модифицирован\n");
					modify_file(event->name);
				}
				break;
			case IN_CREATE:
				if (event->mask & IN_ISDIR) {
					printf("-----> Директория создана\n");
				} else {
					printf("-----> Файл создан\n");
				}
				break;
			case IN_DELETE:
				if (event->mask & IN_ISDIR) {
					printf("-----> Директория удалена\n");
				} else {
					printf("-----> Файл удалён\n");
				}
				break;
		}
	}

	null_buffer((char *)buf, BUF_LEN);

	return 0;
}

int modify_file(char * filename)
{
	printf("%s\n", filename);
	return 0;
}

int scan_dir(char * path)
{
	DIR *dir;
	struct dirent *ent;
	char * pathfile = calloc(64, 1);

	if ((dir = opendir(path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			pathfile = strcpy(pathfile, path);
			strcat(pathfile, ent->d_name);
			printf("%s\n", pathfile);
			null_buffer(pathfile, 64);
  		}
		closedir(dir);
	} else {
		perror("Opendir fail: ");
		return -1;
	}

	return 0;
}
