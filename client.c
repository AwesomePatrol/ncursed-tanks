#include "client.h"

int main(int argc, char *argv[])
{
   char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
   int len, mysocket;
   struct sockaddr_in dest; 
 
   mysocket = socket(AF_INET, SOCK_STREAM, 0);
 
   /* zero the struct */
   memset(&dest, 0, sizeof(dest));
   
   /* support for IPv4 */
   dest.sin_family = AF_INET;
   
   /* set destination IP number */ 
   dest.sin_addr.s_addr = inet_addr(argv[1]);
   if ( DEBUG <= 1 ) printf("%s\n", inet_ntoa(dest.sin_addr));
   
   /* set destination port number */
   dest.sin_port = htons(PORTNUM);
 
   connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
 
   if ( DEBUG <= 3 ) printf("Received %s (%d bytes).\n", buffer, len);
 
   close(mysocket);
   return EXIT_SUCCESS;
}
