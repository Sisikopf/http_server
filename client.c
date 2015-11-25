#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>

#define MAXBUF  1024

int main(int Count, char *Strings[])
{   int sockfd, bytes_read;
    struct sockaddr_in dest;
    char buffer[MAXBUF];

    if ( Count != 3 )
		{
				printf("usage: testport <IP-addr> <send-msg>\n");
				return;
		}
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        printf("unable to create socket");
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(8080);
    if ( inet_addr(Strings[1], &dest.sin_addr.s_addr) == 0 )
        printf("unable to convert inet_addr");

    if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
        printf("unable to connect");

    sprintf(buffer, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", Strings[2], Strings[1]);
    send(sockfd, buffer, strlen(buffer), 0);

    do
    {
        bzero(buffer, sizeof(buffer));
        bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
        if ( bytes_read > 0 )
            printf("%s", buffer);
    }
    while ( bytes_read > 0 );

    close(sockfd);
    return 0;
}
