#include <stdio.h>
#include <sys/socket.h> /* for socket(), bind, and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and getpid() */
#include <fcntl.h>      /* for fcntl() */
#include <sys/file.h>   /* for O_NONBLOCK and FASYNC */
#include <signal.h>     /* for signal() and SIGALRM */
#include <time.h>


void UseIdleTime();                     /* Function to use idle time */
void SIGIOHandler(int signalType);      /* Function to handle SIGIO */

int sock;

int main() {
    struct sockaddr_in echoServAddr; /* Server address */
    unsigned short echoServPort;     /* Server port */
    struct sigaction handler;        /* Signal handling action definition */

    echoServPort = 9700;

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        fprintf(stderr, "Socket open error! Please restart later...\n");
        exit(1);
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Port */

    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        fprintf(stderr, "Socket binding error! Please restart later...\n");
        exit(1);
    }

    /* Set signal handler for SIGIO */
    handler.sa_handler = SIGIOHandler;

    /* Create mask that mask all signals */
    if (sigfillset(&handler.sa_mask) < 0) {
        fprintf(stderr, "sigfillset() fuction error! Please restart later...\n");
        exit(1);
    }

    /* No flags */
    handler.sa_flags = 0;
    if (sigaction(SIGIO, &handler, 0) < 0) {
        fprintf(stderr, "Signal action failed! Please restart later...\n");
        exit(1);
    }

    /* We must own the socket to receive the SIGIO message */
    if (fcntl(sock, F_SETOWN, getpid()) < 0) {
        fprintf(stderr, "Unable to set process owner to us...\n");
        exit(1);
    }

    /* Arrange for nonblocking I/O and SIGIO delivery */
    if (fcntl(sock, F_SETFL, O_NONBLOCK | FASYNC) < 0) {
        fprintf(stderr, "Unable to put client sock into non-blocking/async mode...\n");
        exit(1);
    }
    /* Go off and do real work; echoing happens in the background */

    for (;;) UseIdleTime();


    return 0;
}

void UseIdleTime() {
    printf(".\n");
    sleep(3);     /* 3 seconds of activity */
}

void SIGIOHandler(int signalType) {
    struct sockaddr_in echoClntAddr;  /* Address of datagram source */
    unsigned int clntLen;             /* Address length */
    int recvMsgSize, i;                  /* Size of datagram */
    char buffer[BUFSIZ];         /* Datagram buffer */
    char url[BUFSIZ], cookie[BUFSIZ];
    int returnVal = 0;
    srand(time(NULL));

    clntLen = sizeof(echoClntAddr);

    memset(buffer, 0, BUFSIZ);
    if ((recvMsgSize = recvfrom(sock, buffer, BUFSIZ, 0, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0) {
        /* Only acceptable error: recvfrom() would have blocked */
        perror("recvfrom error");
        return;
    }
    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

    memset(url, 0, BUFSIZ);
    memset(cookie, 0, BUFSIZ);

    /*받은 결과를 url, cookie 각각 파싱*/
    sscanf(buffer, "{\"url\":\"%[^\"]\",\"cookie\":\"%[^\"]\"}", url, cookie);

    puts(buffer);

    returnVal = rand() % 2;

    memset(buffer, 0, BUFSIZ);

    /*보낼 형식에 맞추기위한 작업(status, reason 추가)*/
    sprintf(buffer, "{\"url\":\"%s\",\"cookie\":\"%s\",\"status\":\"%s\",\"reason\":\"%s\"}", url, cookie, returnVal? "WHITE": "BLACK", returnVal? "": "MALWARE FOUND");
    puts(buffer);

    /*send to pi*/
    i = strlen(buffer);
    if (sendto(sock, buffer, i, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != i) {
        perror("recvfrom error");
    }

    /* Nothing left to receive */
}
