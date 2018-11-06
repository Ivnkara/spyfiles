#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int start_spy(char * path, int daemon);
int spy_dir(int fd);
int modify_file(char * filename);
int scan_dir(char * path);

struct scan_list {
	char * path;
	int size;
};
