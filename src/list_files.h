#ifndef LIST_FILES_H
#define LIST_FILES_H

#include <linux/limits.h>

#define PATHNAME_SIZE 256
#define FILENAME_SIZE 64

struct scan_list {
	char * path, * name;
	int size;
};

// extern char * scan_path;
extern struct scan_list list[PATH_MAX];
extern int count_scan_list;

int scan_dir(char * path, int print_result);
int check_file(char * filename);
int add_file_to_scan(char * filename);

void print_changes_file(char * pathfile, int bytes);

#endif /* LIST_FILES_H_ENDIF */
