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

#define BUFSIZE 500

void UseIdleTime();                     /* Function to use idle time */
void SIGIOHandler(int signalType);      /* Function to handle SIGIO */

int sock;

int main()
{
  struct sockaddr_in echoServAddr; /* Server address */
  unsigned short echoServPort;     /* Server port */
  struct sigaction handler;        /* Signal handling action definition */

  printf("Enter listening port : ");
  scanf("%hd", &echoServPort);

  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    fprintf(stderr, "Socket open error! Please restart later...\n");
    exit(1);
  }

  memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
  echoServAddr.sin_family = AF_INET;                /* Internet family */
  echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
  echoServAddr.sin_port = htons(echoServPort);      /* Port */

  if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
  {
    fprintf(stderr, "Socket binding error! Please restart later...\n");
    exit(1);
  }

  /* Set signal handler for SIGIO */
  handler.sa_handler = SIGIOHandler;
  /* Create mask that mask all signals */
  if (sigfillset(&handler.sa_mask) < 0)
  {
    fprintf(stderr, "sigfillset() fuction error! Please restart later...\n");
    exit(1);
  }
  /* No flags */
  handler.sa_flags = 0;

  if (sigaction(SIGIO, &handler, 0) < 0)
  {
    fprintf(stderr, "Signal action failed! Please restart later...\n");
    exit(1);
  }
  /* We must own the socket to receive the SIGIO message */
  if (fcntl(sock, F_SETOWN, getpid()) < 0)
  {
    fprintf(stderr, "Unable to set process owner to us...\n");
    exit(1);
  }

  /* Arrange for nonblocking I/O and SIGIO delivery */
  if (fcntl(sock, F_SETFL, O_NONBLOCK | FASYNC) < 0)
  {
    fprintf(stderr, "Unable to put client sock into non-blocking/async mode...\n");
    exit(1);
  }
  /* Go off and do real work; echoing happens in the background */

  for (;;)
      UseIdleTime();


  return 0;
}

void UseIdleTime()
{
    printf(".\n");
    sleep(3);     /* 3 seconds of activity */
}

void SIGIOHandler(int signalType)
{
    struct sockaddr_in echoClntAddr;  /* Address of datagram source */
    unsigned int clntLen;             /* Address length */
    int recvMsgSize, i;                  /* Size of datagram */
    char buffer[BUFSIZE] = {0};         /* Datagram buffer */
    char sendBuf[BUFSIZE] = {0};
    char* host;
    char* cookies;
    int returnVal = 0;
    char reason[25];
    char *checkSum = "a531912d4dfb12a451faeed";

    srand(time(NULL));

    clntLen = sizeof(echoClntAddr);

    if ((recvMsgSize = recvfrom(sock, buffer, BUFSIZE, 0,
           (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
    {
        /* Only acceptable error: recvfrom() would have blocked */
        fprintf(stderr, "recvfrom() error!\n");
    }
    else
    {
        host = strchr(buffer, 'h');
        cookies = strchr(buffer, ',');
        cookies += 14;

        printf("host : ");
        for(i = 0; host[i] != '\"'; i++)
        {
          printf("%c", host[i]);
        }

        printf("\ncookies : ");
        for(i = 0; cookies[i] != '\"'; i++)
        {
          printf("%c", cookies[i]);
        }
        printf("\n");

        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        returnVal = rand() % 2;

	returnVal == 0 ? strcpy(reason, "malware is found") : strcpy(reason, "malware is not found");

        sprintf(sendBuf, "{\n\"response\" : {\n\"status\" : \"%s\",\n\"reason\" : \"%s\",\n\"checksum\" : \"%s\"\n}\n}",
                returnVal == 0 ? "false" : "true", reason, checkSum);

        if (sendto(sock, sendBuf, sizeof(sendBuf), 0, (struct sockaddr *)
              &echoClntAddr, sizeof(echoClntAddr)) != sizeof(sendBuf))
        {
            fprintf(stderr, "sendto() error!\n");
        }

    }
    /* Nothing left to receive */
}
