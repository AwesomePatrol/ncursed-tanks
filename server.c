#include "server.h"

int server_socket; /* socket used to listen for incoming connections */
int consocket;

int map_seed;
map_t map = NULL;
int map_length = 80, map_height = 24;

void exit_cleanup(void);
void sigint_handler(int signum);
void process_request(char *request);

int main(int argc, char *argv[])
{
    struct sigaction sigaction_new = {0};

    unsigned int random_seed;

    char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
    int len;

    struct sockaddr_in dest; /* socket info about the machine connecting to us */
    struct sockaddr_in serv = {0}; /* socket info about our server */
    socklen_t socksize = sizeof(struct sockaddr_in);

    /* Cleanup on normal exit */
    if (atexit(exit_cleanup) != 0)
        if (DEBUG <= 5) puts(
"Warning! Couldn't register exit handler. Won't be able to clean up on exit!");
    /* Handle SIGINT (Control-c) */
    sigaction_new.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sigaction_new, NULL) != 0)
        if (DEBUG <= 5) puts(
"Warning! Couldn't assign handler to SIGINT. If the server is terminated,\n"
"won't be able to clean up!");

    /* Initialize random number renerator with current time */
    random_seed = time(NULL);
    if (DEBUG == 0) printf("Initial random seed is %u\n", random_seed);
    srand(random_seed);

    map_seed = rand();
    if (DEBUG == 0) printf("Map seed is %d\n", map_seed);

    /* set the type of connection to TCP/IP */
    serv.sin_family = AF_INET;
    /* set our address to any interface */
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    /* set the server port number */ 
    serv.sin_port = htons(PORTNUM);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    /* bind serv information to server_socket */
    bind(server_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

    /* start listening, allowing a queue of up to 16 pending connection */
    listen(server_socket, 16);

    while (1)
    {
        consocket = accept(server_socket, (struct sockaddr *)&dest, &socksize);

        if (!consocket)
        {
            if (DEBUG <= 5) puts("Couldn't accept connection!");
            continue;
        }

        if (DEBUG <= 3) printf("Incoming connection from %s\n",
                               inet_ntoa(dest.sin_addr));
        /* receive data */
        while ((len = recv(consocket, buffer, MAXRCVLEN, 0)) != 0)
        {
            buffer[len] = '\0'; /* add null terminator */
            if (DEBUG <= 3) printf("Received: %s\n", buffer);

            process_request(buffer);
        }

        close(consocket);
    }

    return EXIT_SUCCESS;
}

void exit_cleanup(void)
{
    if (map) free(map);
    close(consocket);
    close(server_socket);
}

void sigint_handler(int signum)
{
    if (DEBUG <= 3) puts("Received SIGINT, cleaning up...\n");

    exit(EXIT_SUCCESS);
}

void process_request(char *request)
{
    if (strlen(request) < 1)
    {
        if (DEBUG <= 5) puts("Received data length is <1 !");
        return;
    }

    Command cmd = request[0];
    char reply[MAXRCVLEN + 1];

    switch (cmd)
    {
    case GET_MAP:
        if (DEBUG == 0) puts("Received GET_MAP. Sending map...");
        snprintf(reply, MAXRCVLEN + 1, "%d %d %d",
                 map_seed, map_length, map_height);
        /* TODO check sent length */
        send(consocket, reply, strlen(reply), 0);

        map = generate_map(map_seed, map_length, map_height);

        break;
    default:
        if (DEBUG <= 5) printf("Unrecognized command from client: '%c' !\n",
                               cmd);
    }
}
