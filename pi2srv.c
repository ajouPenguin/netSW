#include "pi2srv.h"
#include "pktcheck.h"
#include "listControl.h"

char *servAddr = "192.168.222.10";
int   servPort = 9700;

void *pi2sand(void *arg) {
	int sock, cur_idx;
	struct sockaddr_in sockAddr;
	struct sockaddr_in fromAddr;
	unsigned int fromSize;
	char buf[BUFSIZ], *url, *cookie;
	unsigned int bufLen;
	int bytesRcvd, byteSent;
	char *temp, url2[BUFSIZ], cookie2[BUFSIZ], status[BUFSIZ], reason[BUFSIZ];

	struct list_item_t *item;
	struct thread_arg_t *t;

	t = (struct thread_arg_t *) arg;

	cur_idx = t->cur_idx;
	url = t->url;
	cookie = t->cookie;

	free(t);

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(servAddr);
	sockAddr.sin_port = htons(servPort);

	/* send to server*/
	memset(buf, 0, BUFSIZ);

	sprintf(buf, "{\"url\":\"%s\",\"cookie\":\"%s\"}", url, cookie);

	bufLen = strlen(buf);
	buf[bufLen] = 0;

	puts(buf);

	if((byteSent = sendto(sock, buf, bufLen, 0, (struct sockaddr *) &sockAddr, sizeof(sockAddr))) < 0){
		perror("sendto error");
		return;
	}

	/* recv from server */
	memset(buf, 0, BUFSIZ);
	fromSize = sizeof(fromAddr);

	if(bytesRcvd = recvfrom(sock, buf, BUFSIZ, 0, (struct sockaddr *) &fromAddr, &fromSize) < 0){
		perror("recvfrom error");
		return;
	}
	printf("[RECV] %s:%d-%s%d\n",
		inet_ntoa(remtAddr.sin_addr),
		ntohs(remtAddr.sin_port));
	//buf[bytesRcvd] = 0;
	printf("SHIT:%d\n", bytesRcvd);
	printf("Receive message: %s\n", buf);
	puts(buf);


	sscanf(buf, "{\"url\":\"%[^\"]\",\"cookie\":\"%[^\"]\",\"status\":\"%[^\"]\",\"reason\":\"%[^\"]\"}", url2, cookie2, status, reason);

	if (strcmp(url, url2) != 0 || strcmp(cookie, cookie2) != 0) {
		fprintf(stderr, "data corrupted\r\n");
		return;
	}


	puts(url);
	puts(cookie);
	puts(status);
	puts(cookie);
/*
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
*/
	//process_by_list(cur_idx, status);
}

int main() {
	struct thread_arg_t *arg = (struct thread_arg_t *) malloc(1 * sizeof(struct thread_arg_t));
	arg->cur_idx = 0;
	arg->url = strdup("http://eteastasdfasdfasdf/asdfasdfasdf");
	arg->cookie = strdup("asdfasdfasdfasdfasdf");
	pi2sand(arg);
	return 0;
}
