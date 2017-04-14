#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <time.h>

#define ADDRESS     "mysocket"  /* addr to connect */
#define WELCOME		"Welcome to the server!\n"
#define MAXMSG  1024

int read_from_client (int filedes)
{
  char buffer[MAXMSG];
  int nbytes;

  nbytes = read (filedes, buffer, MAXMSG);
  if (nbytes < 0)
    {
      /* Read error. */
      perror ("read");
      exit (1);
    }
  else if (nbytes == 0)
    /* End-of-file. */
    return -1;
  else
    {
      /* Data read. */
      fprintf (stderr, "Server: got message: `%s'\n", buffer);
      return 0;
    }
}

int main(int argc, char *argv[])
{
	char c;
	FILE *fp;
	int fromlen;
	register int i, s, ns, len, sock;
	struct sockaddr_un address, fsaun, clientname;
	time_t t;
	struct tm *tm_info;
	char buffer[MAXMSG];
	fd_set active_fd_set, read_fd_set;
	  size_t size;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("server: socket");
		exit(1);
	}

	/*
	* Create the address we will be binding to.
	*/
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, ADDRESS);

	/*
	* Try to bind the address to the socket.  We
	* unlink the name first so that the bind won't
	* fail. 
	*
	* The third argument indicates the "length" of
	* the structure, not just the length of the
	* socket name.
	*/
	unlink(ADDRESS);
	len = sizeof(address.sun_family) + strlen(address.sun_path);

	if (bind(s, (struct sockaddr *)&address, len) < 0) {
		perror("server: bind");
		exit(1);
	}

	/*
	* Listen on the socket.
	*/
	if (listen(s, 5) < 0) {
		perror("server: listen");
		exit(1);
	}

	/*
	* Accept connections.  When we accept one, ns
	* will be connected to the client.  fsaun will
	* contain the address of the client.
	*/

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (s, &active_fd_set);

	do {

	    /* Block until input arrives on one or more active sockets. */
	    read_fd_set = active_fd_set;
	    if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
	    {
	        perror ("select");
	        exit (1);
	    }

	     /* Service all the sockets with input pending. */
      	for (i = 0; i < FD_SETSIZE; ++i)
      	{
        	if (FD_ISSET (i, &read_fd_set))
          	{
            	if (i == s)
	            {
	                /* Connection request on original socket. */
	                int new;
	                size = sizeof (address);
	                new = accept (s,
	                              (struct sockaddr *) &address,
	                              &size);
	                if (new < 0)
	                {
	                	perror ("accept");
	                    exit (1);
	                }
	                fprintf (stderr,
	                         "Server: connect from host.\n");
	                         
	                FD_SET (new, &active_fd_set);
              	}
            	else
              	{
                	/* Data arriving on an already-connected socket. */
                	if (read_from_client (i) < 0)
                  	{
                    	close (i);
                    	FD_CLR (i, &active_fd_set);
                  	}
              	}
          	}
    	}

		/*if ((ns = accept(s, (struct sockaddr *)&fsaun, &fromlen)) < 0) {
			perror("server: accept");
			exit(1);
		}*/

		/*
		* We'll use stdio for reading the socket.
		*/
		/*fp = fdopen(ns, "r");
		strcpy(buffer, WELCOME);
		printf("%d, %s\n", strlen(buffer), buffer);
		send(ns, buffer, strlen(buffer), 0);*/

		/*
		* then we read some strings from the client and
		* print them out.
		*/
			/*while ((c = fgetc(fp)) != EOF) {
				putchar(c);
				if (c == '\n')
					break;
			}
		
		printf("\n");
		//strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		//printf("%s.%03d\n", buffer);
		
		*/
	} while (1);
	/*
	* We can simply use close() to terminate the
	* connection, since we're done with both sides.
	*/
	close(s);

	exit(0);
}