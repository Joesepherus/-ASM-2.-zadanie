#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <time.h>

#define ADDRESS     "mysocket"  /* addr to connect */
#define WELCOME		"Welcome to the server!\n"

int main(int argc, char *argv[])
{
	char c;
	FILE *fp;
	int fromlen;
	register int i, s, ns, len;
	struct sockaddr_un address, fsaun;
	time_t t;
	struct tm *tm_info;
	char buffer[1024];

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
	do {
		if ((ns = accept(s, (struct sockaddr *)&fsaun, &fromlen)) < 0) {
			perror("server: accept");
			exit(1);
		}

		/*
		* We'll use stdio for reading the socket.
		*/
		fp = fdopen(ns, "r");
		strcpy(buffer, WELCOME);
		printf("%d, %s\n", strlen(buffer), buffer);
		send(ns, buffer, strlen(buffer), 0);

		/*
		* then we read some strings from the client and
		* print them out.
		*/
			while ((c = fgetc(fp)) != EOF) {
				putchar(c);
				if (c == '\n')
					break;
			}
		
		printf("\n");
		//strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		//printf("%s.%03d\n", buffer);


	} while (1);
	/*
	* We can simply use close() to terminate the
	* connection, since we're done with both sides.
	*/
	close(s);

	exit(0);
}