#ifndef HELPERS_H
#define HELPERS_H

#include "list_files.h"

void print_event(struct inotify_event * event);
void print_scan_list(struct scan_list * list, int count);

#endif
