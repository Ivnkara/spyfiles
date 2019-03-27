#ifndef LIST_FILES_H
#define LIST_FILES_H

#include <linux/limits.h>

#define PATHNAME_SIZE 256
#define MAX_SCAN_LIST 9
#define FILENAME_SIZE 64

struct scan_list {
	char * path, * name;
	int size;
};

extern struct scan_list list_files[PATH_MAX];
extern int count_list_files;
extern char * list_dirs[MAX_SCAN_LIST];
extern int count_list_dirs;

int init_scan_list(char ** array_dirs, int argc);
int scan_dir(char ** array_dirs, int argc, int print_result, int init_list);
int check_file(char * filename);
int add_file_to_list(char * pathfile, char * filename);
int add_file_to_scan(char * filename);
int remove_file_to_scan(char * filename);

void print_changes_file(char * pathfile, int bytes);

#endif /* LIST_FILES_H_ENDIF */
