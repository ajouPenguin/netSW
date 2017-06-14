#ifndef __PI2SRV_H__
#define __PI2SRV_H__

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

extern char *servAddr;
extern int   servPort;

struct thread_arg_t {
	int cur_idx;
	char *url;
	char *cookie;
};

void *pi2sand(void *arg);

#endif
