#ifndef SPY_H
#define SPY_H

#include <linux/limits.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int get_inotify_fd();
int get_watch_wd(char * path, int inotify_fd);
int prepare_event(int count, char * events);

int get_event(int inotify_fd, char * buf);

#endif /* SPY_H_ENDIF */
