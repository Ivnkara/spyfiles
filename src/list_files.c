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
char * scan_list[MAX_SCAN_LIST];

/**
 * Массив структур с информацией отслеживаемых файлов 
 */
struct scan_list list[PATH_MAX];

/**
 * Переменная с количеством элементов в массие list
 */
int count_scan_list;

/**
 * Инициализирует переменную scan_list
 *
 * @param Список аргументов cli
 * @param Количество аргументов
 */
int init_scan_list(char const * argv[], int argc)
{
	for (int i = 1; i <= argc; ++i) {
		scan_list[i - 1] = calloc(PATHNAME_SIZE, sizeof(char));

		strcpy(scan_list[i - 1], argv[i]);
	}

	return 0;
}

/**
 * Сканирует директорию и записывает список файлов (путь, имя, размер)
 * в специальный массив list структур scan_list,
 * после чего распичатывает список файлов и директорий
 *
 * @param  Список аргументов cli
 * @param  Количество аргументов
 * @param  Печатать ли результат сканирования
 * @return 0 - успех, -1 - ошибка
 */
int scan_dir(char const * argv[], int argc, int print_result)
{
	DIR * dir;
	struct dirent *ent;
	char * pathfile = calloc(PATHNAME_SIZE, sizeof(char));
	count_scan_list = 0;

	init_scan_list(argv, argc);

	if ((dir = opendir(scan_list[0])) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}

			pathfile = strcpy(pathfile, scan_list[0]);

			strcat(pathfile, ent->d_name);
			add_file_to_list(pathfile, ent->d_name);
			bzero(pathfile, PATHNAME_SIZE);
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
int check_file(char * filename)
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
 * Добавляет файл к списку отслеживаемых файлов
 *
 * @param  char * pathfile
 * @return int
 */
int add_file_to_list(char * pathfile, char * filename)
{
	struct stat sb;

	stat(pathfile, &sb);

	list[count_scan_list].path = calloc(PATHNAME_SIZE, sizeof(char));
	list[count_scan_list].name = calloc(FILENAME_SIZE, sizeof(char));

	strcpy(list[count_scan_list].path, pathfile);
	strcpy(list[count_scan_list].name, filename);
	list[count_scan_list].size = sb.st_size;

	bzero((char *)&sb, sizeof(sb));
	count_scan_list++;

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

	strcpy(pathfile, scan_list[0]);
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

	for (i = 0; i < count_scan_list; ++i) {
		if (strcmp(filename, list[i].name) == 0) {
			which = i;
		}
	}

	for (i = which; i < count_scan_list - 1; ++i) {
		list[i] = list[i + 1];
	}

	count_scan_list--;

	print_scan_list(list, count_scan_list);

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
 