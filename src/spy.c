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
	int fd, wd;

	fd = inotify_init();

	if (fd < 0) {
		perror("Error inotify_init");

		return -1;
	}

	scan_dir(path);

	wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);

	printf("Spy starting...\n");

	do {
		spy_dir(fd);

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
	check_filesize(filename);
	return 0;
}


int scan_dir(char * path)
{
	DIR *dir;
	struct dirent *ent;
	struct stat sb;
	char * pathfile = calloc(PATHNAME_SIZE, sizeof(char));

	count_scan_list = 0;

	if ((dir = opendir(path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}

			pathfile = strcpy(pathfile, path);

			strcat(pathfile, ent->d_name);
			stat(pathfile, &sb);

			list[count_scan_list].path = calloc(PATHNAME_SIZE, sizeof(char));
			list[count_scan_list].name = calloc(FILENAME_SIZE, sizeof(char));

			strcpy(list[count_scan_list].path, pathfile);
			strcpy(list[count_scan_list].name, ent->d_name);
			list[count_scan_list].size = sb.st_size;

			clean_buffer(pathfile, PATHNAME_SIZE);
			clean_buffer((char *)&sb, sizeof(sb));

			count_scan_list++;
  		}

		closedir(dir);
	} else {
		perror("Opendir fail: ");

		return -1;
	}

	print_scan_list(list, count_scan_list);

	return 0;
}

int check_filesize(char * filename)
{
	struct stat sb;

	for (int i = 0; i < count_scan_list; ++i) {
		if (strcmp(list[i].name, filename) == 0) {
			stat(list[i].path, &sb);

			if (list[i].size < sb.st_size) {
				uint sub = sb.st_size - list[i].size; 
				printf("Размер файла увеличился на %d байт\n", sub);
				print_changes_file(list[i].path, sub);
			}
		 } 
	}

	return 0;
}

void print_changes_file(char * pathfile, int bytes)
{
	int df = open(pathfile, O_RDONLY);
	char * buffer = calloc(bytes, sizeof(char));

	if (df < 0) {
		perror("open file in print_changes_file: ");
	}

	lseek(df, -bytes, SEEK_END);

	int count = read(df, buffer, bytes);

	printf("%d\n", count);

	if (count > 0) {
		printf("\n%s\n", buffer);
	}

	close(df);
}
