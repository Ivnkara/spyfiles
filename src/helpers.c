#include <stdio.h>
#include <sys/inotify.h>

#include "spy.h"

void null_buffer(char * buffer, int size) {
	for (int i = 0; i < size; ++i) {
		buffer[i] = 0;
	}
}

void print_event(struct inotify_event * event) {
	printf("Name:   %s\n", event->name);
	printf("WD:     %d\n", event->wd);
	printf("Mask:   %d\n", event->mask);
	printf("Cookie: %d\n", event->cookie);
	printf("Length: %d\n", event->len);
}

void print_scan_list(struct scan_list * list, int count) {
	printf("Scan list:\n");
	
	for (int i = 0; i < count; ++i) {
		printf("\tPath: %s\n", list[i].path);
		printf("\tSize: %d\n", list[i].size);
	}
}
