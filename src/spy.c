#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

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

	printf("Spy starting...\n");

	do {
		spy_dir_result = spy_dir(fd);

		if (spy_dir_result != 0) {
			perror("Error spy_dir");

			return -1;
		}

		printf("Spy continue...\n");
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

	struct inotify_event * event = (struct inotify_event * ) &buf[0];

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

	clean_buffer((char * )buf, BUF_LEN);

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
	struct scan_list list[SCAN_LIST_SIZE];
	struct stat sb;
	char * pathfile = calloc(PATHNAME_SIZE, sizeof(char));
	int i = 0;

	if ((dir = opendir(path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}

			pathfile = strcpy(pathfile, path);

			strcat(pathfile, ent->d_name);
			stat(pathfile, &sb);

			list[i].path = calloc(PATHNAME_SIZE, sizeof(char));
			list[i].name = calloc(FILENAME_SIZE, sizeof(char));

			strcpy(list[i].path, pathfile);
			strcpy(list[i].name, ent->d_name);
			list[i].size = sb.st_size;

			clean_buffer(pathfile, PATHNAME_SIZE);
			clean_buffer((char *)&sb, sizeof(sb));

			i++;
  		}

		closedir(dir);
	} else {
		perror("Opendir fail: ");

		return -1;
	}

	print_scan_list(list, i);

	return 0;
}
