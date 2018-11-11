#include <stdio.h>
#include <sys/inotify.h>

#include "spy.h"

/**
 * Распечатывает переданное событие inotify
 *
 * @param struct inotify_event * event
 */
void print_event(struct inotify_event * event) {
	printf("Name:   %s\n", event->name);
	printf("WD:     %d\n", event->wd);
	printf("Mask:   %d\n", event->mask);
	printf("Cookie: %d\n", event->cookie);
	printf("Length: %d\n", event->len);
}

/**
 * Распечатывает массив структур list хранящий список файлов в отслеживаемой директории
 *
 * @param struct scan_list * list  Адрес массива структур
 * @param int                count Количество структур в массиве
 */
void print_scan_list(struct scan_list * list, int count) {
	printf("Path: \t\t\tName: \tSize:\n");
	printf("----------------------------------------------------\n");

	for (int i = 0; i < count; ++i) {
		printf("%s \t%s \t%d\n", list[i].path, list[i].name, list[i].size);
	}

	printf("\n\n");
}
