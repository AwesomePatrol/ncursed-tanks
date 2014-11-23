#include "client.h"

void init_curses()
{
    initscr(); /* initialize screen to draw on */
    noecho(); /* do not echo any keypress */
    curs_set(FALSE); /* do not show cursor */

    /* Initialize colors */
    start_color();
    use_default_colors();
    init_pair(COL_W, COLOR_WHITE, -1);
    init_pair(COL_R, COLOR_RED, -1);
    init_pair(COL_RR, COLOR_RED, COLOR_RED);
    init_pair(COL_YY, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(COL_WW, COLOR_WHITE, COLOR_WHITE);
    init_pair(COL_G, COLOR_GREEN, -1);
    init_pair(COL_C, COLOR_CYAN, -1);
    init_pair(COL_M, COLOR_MAGENTA, -1);
    init_pair(COL_B, -1, -1);
}

int main(int argc, char *argv[])
{
    /* Be sure we have proper arguments */
    if ( argc <= 2 || strlen(argv[1]) < 7
            || strlen(argv[2]) < 3)
    {
        if (DEBUG <= 5) puts("Wrong command-line arguments!");
        return EXIT_FAILURE;
    }

    /* Init ncurses */
    init_curses();
    
    /* Get connection to server */
    char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
    int len, sent, cl_sock;
    struct sockaddr_in dest; 
 
    cl_sock = socket(AF_INET, SOCK_STREAM, 0);

    /* zero the struct */
    memset(&dest, 0, sizeof(dest));
    
    /* support for IPv4 */
    dest.sin_family = AF_INET;
    
    /* set destination IP number */ 
    dest.sin_addr.s_addr = inet_addr(argv[1]);

    /* set destination port number */
    dest.sin_port = htons(PORTNUM);

    /* in case socket or connection is broken we should fail here */
    if ( cl_sock == -1 ||
         connect(cl_sock, (struct sockaddr *)&dest, sizeof(struct sockaddr))
         == -1 )
    {
        if (DEBUG <= 5) puts("Socket or connection is broken!");
        return EXIT_FAILURE;
    }

    sent = send(cl_sock, "M", 1, 0);
    if (DEBUG <= 3)
    {
        printw("Sent %d bytes.", sent);
        getch();
    }

    /* Close connection */
    close(cl_sock);
    return EXIT_SUCCESS;
}
