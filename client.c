#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define NSTRS       3           /* no. of strings  */
#define ADDRESS     "mysocket"  /* addr to connect */



int main(int argc, char *argv[])
{
    char c;
    FILE *fp;
    register int i, sock, len, opt;
    struct sockaddr_un address;

    while ((opt = getopt(argc, argv, "p")) != -1)
        switch (opt)
        {
        case 'p':
            printf("Output: '%s': parameter prepinaca -p\n", argv[2]);
            break;
        default:
            printf("Error: '%c': nesparvny prepinac", argv[0]);
            exit(1);
        }


    // initialize socket
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("client: socket");
        exit(1);
    }

    /*
    * Create the address we will be connecting to.
    */
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, ADDRESS);


    len = sizeof(address.sun_family) + strlen(address.sun_path);

    connect(sock, (struct sockaddr *)&address, len);

    /*
    * We'll use stdio for reading
    * the socket.
    */

    fp = fdopen(sock, "r");


    // read & print string from the server
        while ((c = fgetc(fp)) != EOF) {
            putchar(c);
            if (c == '\n')
                break;
        }

    // send a string to the server
    send(sock, argv[2], strlen(argv[2]), 0);


    // close socket
    close(sock);

    exit(0);
}   