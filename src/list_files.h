#ifndef LIST_FILES_H
#define LIST_FILES_H

#include <linux/limits.h>

#define PATHNAME_SIZE 256
#define FILENAME_SIZE 64

int scan_dir(char * path, int print_result);
int check_filesize(char * filename);

void print_changes_file(char * pathfile, int bytes);

struct scan_list {
	char * path, * name;
	int size;
};

extern struct scan_list list[PATH_MAX];
extern int count_scan_list;

#endif /* LIST_FILES_H_ENDIF */
