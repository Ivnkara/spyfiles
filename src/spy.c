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

/**
 * Функция начинает следить за директорией
 *
 * TODO: нужно сделать чтобы сканирование происходило после каждого события, а не только при начале слежки
 *
 * @param  char * path   Путь до директории за которой нужно следить
 * @param  int    daemon 1 - Окончить слежку после первых изменений, 0 - Продолжать слежку постоянно 
 * @return int           0 - при успешном завершении, -1 - при ошибке
 */
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

	printf("Spy starting...\n\n");

	do {
		spy_dir(fd);
	} while(daemon);

	inotify_rm_watch(fd, wd);
	close(fd);

	return 0;
}

/**
 * Инициирует структуру inotify_event, и отслеживает события в директории
 *
 * @param  int fd Дескриптор inotify_init
 * @return int    0 - успех, -1 - ошибка
 */
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
					// делаем что-нибудь
				} else {
					check_filesize(event->name);
				}
				break;
			case IN_CREATE:
				if (event->mask & IN_ISDIR) {
					// делаем что-нибудь
				} else {
					// делаем что-нибудь
				}
				break;
			case IN_DELETE:
				if (event->mask & IN_ISDIR) {
					// делаем что-нибудь
				} else {
					// делаем что-нибудь
				}
				break;
		}
	}

	clean_buffer((char * )buf, BUF_LEN);

	return 0;
}

/**
 * Сканирует директорию и записывает список файлов (путь, имя, размер)
 * в специальный массив list структур scan_list,
 * после чего распичатывает список файлов и директорий
 *
 * @param  char * path Путь до сканируемой директории
 * @return int         0 - успех, -1 - ошибка
 */
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

	free(pathfile);
	print_scan_list(list, count_scan_list);

	return 0;
}

/**
 * Проверяет изменился ли размер файла, при событии IN_MODIFY,
 * если да, сравнивает и вычисляет разницу в байтах
 *
 * @param  char * filename Путь до проверяемого файла
 * @return int 
 */
int check_filesize(char * filename)
{
	struct stat sb;

	for (int i = 0; i < count_scan_list; ++i) {
		if (strcmp(list[i].name, filename) == 0) {
			stat(list[i].path, &sb);

			if (list[i].size < sb.st_size) {
				uint sub = sb.st_size - list[i].size; 
				printf("-----> Размер файла %s увеличился на %d байт\n", filename, sub);
				print_changes_file(list[i].path, sub);
			}
		 } 
	}

	return 0;
}

/**
 * Выводит в stdout конец файла равный количеству переданных байт
 *
 * TODO: нужно сделать чтобы выводил именно новые данные, а не конец файла
 *
 * @param char * pathfile Путь до файла
 * @param int    bytes    Количество новых байт в файле
 */
void print_changes_file(char * pathfile, int bytes)
{
	int df = open(pathfile, O_RDONLY);
	char * buffer = calloc(bytes, sizeof(char));

	if (df < 0) {
		perror("Error open file in print_changes_file: ");
	}

	lseek(df, -bytes, SEEK_END);

	int count = read(df, buffer, bytes);

	if (count > 0) {
		printf("-----> Новые данные в файле:\n");
		printf("____________________________________________________________________________________\n");
		printf("\n%s\n", buffer);
		printf("____________________________________________________________________________________\n");
		printf("\nSpy continue...\n\n");
	}

	free(buffer);
	close(df);
}
