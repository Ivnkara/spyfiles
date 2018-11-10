#ifndef SPY_H
#define SPY_H

#include <linux/limits.h>

#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
#define PATHNAME_SIZE 256
#define FILENAME_SIZE 64

int start_spy(char * path, int daemon);
int spy_dir(int fd);
int scan_dir(char * path);
int check_filesize(char * filename);

void print_changes_file(char * pathfile, int bytes);

struct scan_list {
	char * path, * name;
	int size;
};

extern struct scan_list list[PATH_MAX];
extern int count_scan_list;

#endif
