#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLEN 1024

void pi2sand(char url[], char cookie[]){

	int sock; /* Socket descriptor */	
	struct sockaddr_in echoServAddr; /* Echo server address */
	struct sockaddr_in fromAddr;
	unsigned short echoServPort; /* Echo server port */
	unsigned int fromSize;
	char *servIP; /* Server IP addr. (dotted quad) */
	char buf[MAXLEN]; 
	unsigned int bufLen;
	int byteSend;
	int bytesRcvd; /* Bytes read in single recv() */
	int totalBytesRcvd; /* Total bytes read */
	servIP = "192.168.200.119"; /* server IP address (dotted quad) */

	echoServPort = 9700; /* port */
	
	
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);
	echoServAddr.sin_port = htons(echoServPort);
	
	
	//no bind()
	memset(buf,0,MAXLEN);

	sprintf(buf, "{\nrequest : {\n\turl : %s,\n\tcookie : %s\n\t}\n}", url, cookie);
	
	/* Send the string to the server */
	bufLen = strlen(buf);
	buf[bufLen] = '\0';


	byteSend = sendto(sock, buf, bufLen, 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr));

	close (sock);
	
}
