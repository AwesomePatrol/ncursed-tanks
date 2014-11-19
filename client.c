#include "client.h"

int main(int argc, char *argv[])
{
    char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
    int len, mysocket;
    struct sockaddr_in dest; 
 
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
 
    memset(&dest, 0, sizeof(dest));              /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl((int) argv[1]); /* set destination IP number */
    dest.sin_port = htons(PORTNUM);              /* set destination port number */
 
    connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
 
    len = recv(mysocket, buffer, MAXRCVLEN, 0);
 
    /* We have to null terminate the received data ourselves */
    buffer[len] = '\0';
 
    if ( DEBUG <= 3 ) printf("Received %s (%d bytes).\n", buffer, len);
 
    close(mysocket);
    return EXIT_SUCCESS;
}
