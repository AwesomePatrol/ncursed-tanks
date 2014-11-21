#include "server.h"

int mysocket;            /* socket used to listen for incoming connections */

int map_seed;
map_t map = NULL;
int map_length = 80, map_height = 24;

void process_request(char *request)
{
    if (strlen(request) < 1)
    {
        if (DEBUG <= 5) puts("Received data length is <1 !");
        return;
    }

    Command cmd = request[0];
    char answer[MAXRCVLEN + 1];

    switch(cmd)
    {
    case GET_MAP:
        /* TODO send */
        map_seed = rand();
        if (DEBUG == 0) printf("Map seed is %d\n", map_seed);

        snprintf(answer, MAXRCVLEN + 1, "%d", map_seed);
        send(mysocket, answer, strlen(answer), 0);

        map = generate_map(map_seed, map_length, map_height);

        break;
    default:
        if (DEBUG <= 5) printf("Unrecognized command from client: '%c' !", cmd);
    }
}

int main(int argc, char *argv[])
{
    unsigned int random_seed;

    char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
    int len;

    struct sockaddr_in dest; /* socket info about the machine connecting to us */
    struct sockaddr_in serv; /* socket info about our server */
    socklen_t socksize = sizeof(struct sockaddr_in);

    random_seed = time(NULL);
    if (DEBUG == 0) printf("Initial random seed is %u\n", random_seed);
    srand(random_seed);

    /* zero the struct before filling the fields */
    memset(&serv, 0, sizeof(serv));
    /* set the type of connection to TCP/IP */
    serv.sin_family = AF_INET;
    /* set our address to any interface */
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    /* set the server port number */ 
    serv.sin_port = htons(PORTNUM);
 
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
 
    /* bind serv information to mysocket */
    bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
 
    /* start listening, allowing a queue of up to 16 pending connection */
    listen(mysocket, 16);
    int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
 
    if (consocket)
    {
        if (DEBUG <= 3) printf("Incoming connection from %s\n",
                               inet_ntoa(dest.sin_addr));
        len = recv(mysocket, buffer, MAXRCVLEN, 0); /* receive data */
        buffer[len] = '\0'; /* add null terminator */
        if (DEBUG <= 3) printf("Received: %s", buffer);

        process_request(buffer);
    }
    else
    {
        if (DEBUG <= 5) printf("Couldn't accept connection!\n");
    }

    close(consocket);
    close(mysocket);
    return EXIT_SUCCESS;
}
