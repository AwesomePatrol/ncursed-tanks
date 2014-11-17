#include "server.h"

int main()
{
    char msg[] = "Hello!\n";    

    struct sockaddr_in dest; /* socket info about the machine connecting to us */
    struct sockaddr_in serv; /* socket info about our server */
    int mysocket;            /* socket used to listen for incoming connections */
    socklen_t socksize = sizeof(struct sockaddr_in);
 
    memset(&serv, 0, sizeof(serv));           /* zero the struct before filling the fields */
    serv.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
    serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
    serv.sin_port = htons(PORTNUM);           /* set the server port number */ 
 
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
 
    /* bind serv information to mysocket */
    bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
 
    /* start listening, allowing a queue of up to 16 pending connection */
    listen(mysocket, 16);
    int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
 
    while(consocket)
    {
        if (DEBUG <3) printf("Incoming connection from %s\n", inet_ntoa(dest.sin_addr));
        send(consocket, msg, strlen(msg), 0); 
        close(consocket);
        consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
    }
 
    close(mysocket);
    return EXIT_SUCCESS;
}
