#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "spy.h"
#include "helpers.h"

struct scan_list list[PATH_MAX];
int count_scan_list;

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

	return 0;
}

/**
 * Сканирует директорию и записывает список файлов (путь, имя, размер)
 * в специальный массив list структур scan_list,
 * после чего распичатывает список файлов и директорий
 *
 * @param  char * path Путь    до сканируемой директории
 * @param  int    print_result печатать ли результат сканирования
 * @return int                 0 - успех, -1 - ошибка
 */
int scan_dir(char * path, int print_result)
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

			bzero(pathfile, PATHNAME_SIZE);
			bzero((char *)&sb, sizeof(sb));

			count_scan_list++;
  		}

		closedir(dir);
	} else {
		perror("Opendir fail: ");

		return -1;
	}

	free(pathfile);

	if (print_result) {
		print_scan_list(list, count_scan_list);
	}

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
	bytes += 1;
	int df = open(pathfile, O_RDONLY);
	char * buffer = calloc(bytes, sizeof(char));

	if (df < 0) {
		perror("Error open file in print_changes_file: ");
	}

	lseek(df, -bytes, SEEK_END);

	int count = read(df, buffer, bytes);

	if (count > 0) {
		printf("-----> Новые данные в файле:\n");
		printf("***********************************************************************************************\n");
		printf("%s", buffer);
		printf("***********************************************************************************************\n");
		printf("\nSpy continue...\n\n");
	}

	free(buffer);
	close(df);
}
