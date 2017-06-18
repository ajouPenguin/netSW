#include "pi2srv.h"
#include "pktcheck.h"
#include "listControl.h"

char *servAddr = "192.168.222.10";  //서버 IP, port
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

	struct list_item_t *item;  // DB에 사용하기위한 구조체
	struct thread_arg_t *t;    

	/*server에 보낼 idx, url, cookie 저장*/
	t = (struct thread_arg_t *) arg; 
	cur_idx = t->cur_idx;
	url = t->url;
	cookie = t->cookie;

	free(t);

	/*소켓을 사용하기위한 설정*/
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(servAddr);
	sockAddr.sin_port = htons(servPort);

	memset(buf, 0, BUFSIZ);

	/*보낼 형식에 맞추기위한 작업*/
	sprintf(buf, "{\"url\":\"%s\",\"cookie\":\"%s\"}", url, cookie); 

	bufLen = strlen(buf);
	buf[bufLen] = 0;

	puts(buf);

	/*전송*/
	if((byteSent = sendto(sock, buf, bufLen, 0, (struct sockaddr *) &sockAddr, sizeof(sockAddr))) < 0){ 
		perror("sendto error");
		return;
	}

	/* recv from server */
	memset(buf, 0, BUFSIZ);
	fromSize = sizeof(fromAddr);

	/*server에서 검증한 결과를 받음*/
	if(bytesRcvd = recvfrom(sock, buf, BUFSIZ, 0, (struct sockaddr *) &fromAddr, &fromSize) < 0){ 
		perror("recvfrom error");
		return;
	}

	/*받은 메시지 출력*/
	printf("[RECV] %s:%d-%s%d\n",
		inet_ntoa(remtAddr.sin_addr),
		ntohs(remtAddr.sin_port));
	printf("SHIT:%d\n", bytesRcvd);
	printf("Receive message: %s\n", buf);
	puts(buf);

	/*받은 결과를 url, cookie, state, reason 각각 파싱*/
	sscanf(buf, "{\"url\":\"%[^\"]\",\"cookie\":\"%[^\"]\",\"status\":\"%[^\"]\",\"reason\":\"%[^\"]\"}", url2, cookie2, status, reason);

	/*보낸 url,cookie와 받은 url,cookie를 비교하여 data 오류 확인*/
	if (strcmp(url, url2) != 0 || strcmp(cookie, cookie2) != 0) {  
		fprintf(stderr, "data corrupted\r\n");
		return;
	}

	puts(url);
	puts(cookie);
	puts(status);
	puts(cookie);

	/*DB에서 해당 url, cookie 검색*/
	item = select_item(url, cookie); 

	/*결과에 따라 pending에서 WHITE, BLACK으로 update*/
	if (strcmp(status, "WHITE") == 0) {  
		update_status(item->idx, "WHITE");
	} else if (strcmp(status, "BLACK") == 0) {
		update_status(item->idx, "BLACK");
		update_reason(item->idx, reason);
	}

	free(item);
	free(url);
	free(cookie);

	/*WHITE, BLACK 결과에따라 패킷전송 및 block 실행*/
	process_by_list(cur_idx, status); 
}

