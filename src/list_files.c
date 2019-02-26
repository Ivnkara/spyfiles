#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "spy.h"
#include "helpers.h"
#include "list_files.h"

/**
 * Список директорий для отслеживания файлов
 */
char * list_dirs[MAX_SCAN_LIST];

/**
 * Количество директорий для отслеживания
 */
int count_list_dirs;

/**
 * Массив структур с информацией отслеживаемых файлов 
 */
struct scan_list list_files[PATH_MAX];

/**
 * Переменная с количеством элементов в массие list_files
 */
int count_list_files;

/**
 * Инициализирует переменную scan_list
 *
 * @param Список аргументов cli
 * @param Количество аргументов
 */
int init_scan_list(char ** array_dirs, int argc)
{
	int i, j;
	DIR * dir;

	for (i = 0, j = 0; i < argc; ++i) {
		if((dir = opendir(array_dirs[i])) != NULL)  {
			list_dirs[j] = calloc(PATHNAME_SIZE, sizeof(char));

			strcpy(list_dirs[j], array_dirs[i]);
			closedir(dir);

			++j;
		}
	}

	count_list_dirs = j;

	return 0;
}

/**
 * Сканирует директорию и записывает список файлов (путь, имя, размер)
 * в специальный массив list_files структур scan_list,
 * после чего распичатывает список файлов и директорий
 *
 * @param  Список аргументов cli
 * @param  Количество аргументов
 * @param  Печатать ли результат сканирования
 * @param  Инициализировать ли список сканироваемых директорий
 * @return 0 - успех, -1 - ошибка
 */
int scan_dir(char ** array_dirs, int argc, int print_result, int init_list)
{
	DIR * dir;
	struct dirent * ent;
	char * pathfile = calloc(PATHNAME_SIZE, sizeof(char));
	count_list_files = 0;
	int g = 0;

	if (init_list) {
		init_scan_list(array_dirs, argc);
	}

	for (int i = 0; i < count_list_dirs; ++i) {
		if ((dir = opendir(list_dirs[i])) != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				g++;
				if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
					continue;
				}

				pathfile = strcpy(pathfile, list_dirs[i]);

				strcat(pathfile, ent->d_name);
				add_file_to_list(pathfile, ent->d_name);
				bzero(pathfile, PATHNAME_SIZE);
	  		}

			closedir(dir);
		} else {
			perror("Opendir fail: ");

			return -1;
		}
	}

	free(pathfile);

	if (print_result) {
		print_scan_list(list_files, count_list_files);
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
int check_file(char * filename)
{
	struct stat sb;

	for (int i = 0; i < count_list_files; ++i) {
		if (strcmp(list_files[i].name, filename) == 0) {
			stat(list_files[i].path, &sb);

			if (list_files[i].size < sb.st_size) {
				uint sub = sb.st_size - list_files[i].size; 
				printf("-----> Размер файла %s увеличился на %d байт\n", filename, sub);
				print_changes_file(list_files[i].path, sub);
			}
		 } 
	}

	return 0;
}

/**
 * Добавляет файл к списку отслеживаемых файлов
 *
 * @param  char * pathfile
 * @return int
 */
int add_file_to_list(char * pathfile, char * filename)
{
	struct stat sb;

	stat(pathfile, &sb);

	list_files[count_list_files].path = calloc(PATHNAME_SIZE, sizeof(char));
	list_files[count_list_files].name = calloc(FILENAME_SIZE, sizeof(char));

	strcpy(list_files[count_list_files].path, pathfile);
	strcpy(list_files[count_list_files].name, filename);
	list_files[count_list_files].size = sb.st_size;

	bzero((char *)&sb, sizeof(sb));
	count_list_files++;

	return 0;
}

/**
 * Добавляет файл к списку для отслеживания и распечатывает эту информацию
 *
 * @param  char * filename Имя нового файла
 * @return int
 */
int add_file_to_scan(char * filename)
{
	printf("-:-:-:-:-:-: Был создан и добавлен к списку файл под именем %s :-:-:-:-:-:-\n", filename);

	char * pathfile = calloc(PATHNAME_SIZE, sizeof(char));

	strcpy(pathfile, list_dirs[0]);
	strcat(pathfile, filename);

	add_file_to_list(pathfile, filename);

	return 0;
}

/**
 * Удаляет файл из списка для отслеживания и распечатывает эту информацию
 *
 * @param  char * filename Имя удаляемого файла
 * @return int
 */
int remove_file_to_scan(char * filename)
{
	printf("-:-:-:-:-:-: Был удалён файл из списка отслеживани и директории %s :-:-:-:-:-:-\n", filename);
	int which, i;

	for (i = 0; i < count_list_files; ++i) {
		if (strcmp(filename, list_files[i].name) == 0) {
			which = i;
		}
	}

	for (i = which; i < count_list_files - 1; ++i) {
		list_files[i] = list_files[i + 1];
	}

	count_list_files--;

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
 