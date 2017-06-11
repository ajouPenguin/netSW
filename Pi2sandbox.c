#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <listControl.h>

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
	int bytesRcvd; /* Bytes read in single recv() */
	
	char* temp;
     char status[MAXLEN];
	char reason[MAXLEN];
	char checksum[MAXLEN];
     char query[MAXLEN];

	servIP = "192.168.200.119"; /* server IP address (dotted quad) */

	echoServPort = 9700; /* port */
	
	
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);
	echoServAddr.sin_port = htons(echoServPort);
	
	
	//no bind()
	memset(buf,0,MAXLEN);

	sprintf(buf, "{\nrequest : {\n\t\"url\" : %s,\n\t\"cookie\" : %s\n\t}\n}", url, cookie);

	/* Send the string to the server */
	bufLen = strlen(buf);
	buf[bufLen] = '\0';


	sendto(sock, buf, bufLen, 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr));
	fromSize = sizeof(fromAddr);
	memset(buf,0,MAXLEN);

	/* Receive the same string back from the server */
	if(bytesRcvd = recvfrom (sock, buf, MAXLEN, 0,(struct sockaddr*)&fromAddr, &fromSize) < 0){
		 fprintf(stderr, "recvfrom() error!\n");	
	
	buf[bytesRcvd] = '\0';
	printf("Receive message: %s\n", buf);
	

    if(strstr(buf, "true") != NULL){
		strcpy(status, "true");
	}
	else strcpy(status, "false");

	printf("\n%s\n", status);

    temp = strstr(buf, "reason") + 9;
    int i;
    printf("\nreason : ");
    for(i = 0; temp[i]!= ','; i++)
    {
        printf("%c", temp[i]);
        reason[i] = temp[i];
    }

	temp = strstr(buf, "checksum") + 11; 

    	printf("\nchecksum : ");
    	for(i = 0; temp[i] != '}'; i++)
    	{
            printf("%c", temp[i]);
            checksum[i] = temp[i];
    	}
    	printf("\n");

    sprintf(query, "\'%s\',%s,\'%s\',%s,%s", url, "now()", status, reason);
    printf("%s", query);
    //insertURL(mysql, black, query);

	close (sock);
}
