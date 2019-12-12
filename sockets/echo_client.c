#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 8192

int main(int argc, char **argv) 
{
  //
  // Check the arguments for the host name and port number of 
  // an echo service.
  //
  if (argc != 3) {
    fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
    exit(0);
  }
  
  //
  // Look up the host's name to get its IP address.
  //
  char *host = argv[1];
  int port = atoi(argv[2]);
  struct hostent *hp;
  if ((hp = gethostbyname(host)) == NULL) {
    fprintf(stderr,"GETHOSTBYNAME failed.\n");
    exit(-1);
  }

  //
  // Request a socket and get its file descriptor.
  //
  int clientfd;
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"SOCKET creation failed.\n");
    exit(-1);
  }
    

  //
  // Fill in the host/port info into a (struct sockaddr_in).
  //
  struct sockaddr_in serveraddr;
  bzero((char *) &serveraddr, sizeof(struct sockaddr_in));
  serveraddr.sin_family = AF_INET;
  bcopy((char *) hp->h_addr_list[0], 
	(char *)&serveraddr.sin_addr.s_addr, 
	hp->h_length);
  serveraddr.sin_port = htons(port);

  //
  // Connect to the given host at the given port number.
  //
  if (connect(clientfd,
	      (struct sockaddr *)&serveraddr, 
	      sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr,"CONNECT failed.\n");
    exit(-1);
  }


  unsigned char *ip;
  ip = (unsigned char *)&serveraddr.sin_addr.s_addr;
  printf("Connected to echo service at %d.%d.%d.%d. Enter lines that you'd like to send.\n",
	 ip[0], ip[1], ip[2], ip[3]);

  //
  // Read a line of input, send it, output the response.
  //
  int line = 1;
  while (1) {
    
    char buffer[MAXLINE];

    printf("%d: ", line);
    // Grab a line of text from the user.
    char *s = fgets(buffer, MAXLINE, stdin);
    if (s == NULL) {
      // We encountered END OF FILE.  User must want to quit.
      printf("Ok. Sent %d lines. Exiting.\n", line-1);
      break;
    }

    // Send it to the server.
    printf("%d: %s <-- %s", line, host, buffer);
    write(clientfd,buffer,strlen(buffer)+1);

    // Read the server's response.
    int n = read(clientfd, buffer, MAXLINE);
    if (n == 0) {
      // No bytes received.  The server closed the connection.
      printf("Server closed the connection after handling %d lines.  Exiting.\n", line-1);
      break;
    }
    printf("%d: %s --> ",line,host);

    // Output the response to the user.
    fputs(buffer, stdout);

    line = line+1;
  }

  //
  // Close the connection.
  //
  close(clientfd); 

  exit(0);
}
