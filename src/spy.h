#ifndef SPY_H
#define SPY_H

#include <linux/limits.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int get_inotify_fd();
int get_watch_wd(char * path, int inotify_fd);
int prepare_event(struct inotify_event * event);

struct inotify_event * get_event(int inotify_fd);

#endif /* SPY_H_ENDIF */
