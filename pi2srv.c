#include "pi2srv.h"
#include "pktcheck.h"
#include "listControl.h"

char *servAddr = "192.168.222.251";
int   servPort = 9700;

void *pi2sand(void *arg) {
	int sock, cur_idx;
	struct sockaddr_in servAddr;
	struct sockaddr_in fromAddr;
	unsigned int fromSize;
	char buf[BUFSIZ], *url, *cookie;
	unsigned int bufLen;
	int bytesRcvd, byteSent;
	char *temp, url2[BUFSIZ], cookie2[BUFSIZ], status[BUFSIZ], reason[BUFSIZ];

	struct list_item_t *item;
	struct thread_arg_t *t;

	cur_idx = t->cur_idx;
	url = t->url;
	cookie = t->cookie;

	free(t);

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(servAddr);
	servAddr.sin_port = htons(servPort);

	/* send to server*/
	memset(buf, 0, BUFSIZ);

	sprintf(buf, "{\"url\":\"%s\",\"cookie\" :\"%s\"}", url, cookie);

	bufLen = strlen(buf);
	buf[bufLen] = 0;

	if((byteSent = sendto(sock, buf, bufLen, 0, (struct sockaddr *) &servAddr, sizeof(servAddr))) < 0){
		perror("sendto error");
		return;
	}

	/* recv from server */
	memset(buf, 0, BUFSIZ);
	fromSize = sizeof(fromAddr);

	if(bytesRcvd = recvfrom (sock, buf, BUFSIZ, 0,(struct sockaddr *) &fromAddr, &fromSize) < 0){
		perror("recvfrom error");
		return;
	}
	buf[bytesRcvd] = 0;
	printf("Receive message: %s\n", buf);

	if (strcmp(url, url2) != 0 || strcmp(cookie, cookie2) != 0) {
		fprintf(stderr, "data corrupted\r\n");
		return;
	}

	sscanf(buf, "{\"url\":\"%[^\"]\",\"cookie\":\"%[^\"]\",\"status\":\"%[^\"]\",\"reason\":\"%[^\"]\"}");

	item = select_item(url, cookie);

	if (strcmp(status, "WHITE") == 0) {
		update_status(item->idx, "WHITE");
	} else if (strcmp(status, "BLACK") == 0) {
		update_status(item->idx, "BLACK");
		update_reason(item->idx, reason);
	}

	free(item);
	free(url);
	free(cookie);

	process_by_list(cur_idx, status);
}
