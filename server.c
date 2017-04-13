#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <time.h>

#define NSTRS       3           /* no. of strings  */
#define ADDRESS     "mysocket"  /* addr to connect */

/*
 * Strings we send to the client.
 */
char *strs[NSTRS] = {
	"This is the first string from the server.\n",
	"This is the second string from the server.\n",
	"This is the third string from the server.\n"
};

int main(int argc, char *argv[])
{
	char c;
	FILE *fp;
	int fromlen;
	register int i, s, ns, len;
	struct sockaddr_un saun, fsaun;
	time_t t;
	struct tm *tm_info;
	char buffer[1024];

    /*
     * Get a socket to work with.  This socket will
     * be in the UNIX domain, and will be a
     * stream socket.
     */
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("server: socket");
		exit(1);
	}

    /*
     * Create the address we will be binding to.
     */
	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path, ADDRESS);

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
	len = sizeof(saun.sun_family) + strlen(saun.sun_path);

	if (bind(s, (struct sockaddr *)&saun, len) < 0) {
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
	do{
		if ((ns = accept(s, (struct sockaddr *)&fsaun, &fromlen)) < 0) {
			perror("server: accept");
			exit(1);
		}

    /*
     * We'll use stdio for reading the socket.
     */
		fp = fdopen(ns, "r");

    /*
     * First we send some strings to the client.
     */
		for (i = 0; i < NSTRS; i++)
			send(ns, strs[i], strlen(strs[i]), 0);

    /*
     * Then we read some strings from the client and
     * print them out.
     */
		for (i = 0; i < NSTRS; i++) {
			while ((c = fgetc(fp)) != EOF) {
				putchar(c);

				if (c == '\n')
					break;
			}
		}
		printf("\n");
		strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		printf(buffer);
			//DATUM
            /*printf("%.4i-%.2i-%.2iT%.2i:%.2i:%.2iZ\n",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);*/

	}while(1);
    /*
     * We can simply use close() to terminate the
     * connection, since we're done with both sides.
     */
	close(s);

	exit(0);
}