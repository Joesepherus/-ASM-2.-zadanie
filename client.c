#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define ADDRESS     "mysocket"  /* addr to connect */

int main(int argc, char *argv[])
{
    char c;
    FILE *fp;
    register int i, sock, len, opt;
    struct sockaddr_un address;
    char msg[1024], servaddr_reply[1024], buffer[1024];
    fd_set rs;
    ssize_t r, w;



    while ((opt = getopt(argc, argv, "p")) != -1)
    {
        switch (opt)
        {
        case 'p':
            printf("Output: '%s': parameter prepinaca -p\n", argv[2]);
            break;
        default:
            printf("Error: '%c': nesparvny prepinac", argv[0]);
            exit(1);
        }
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

    while(1)
    {
        if (FD_ISSET(sock, &rs) < 0) break;
        printf("Enter msg:");
        scanf("%s" , msg);

        if(strcmp(msg, "exit") == 0){
            break;
        }

        if( send(sock , msg , strlen(msg) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         // server reply
        /*if( recv(sock, servaddr_reply , 2000 , 0) < 0)
        {
            puts("Recv failed");
            break;
        }*/
        //select(sock + 1, &rs, NULL, NULL, NULL);
        //if(FD_ISSET(sock, &rs)){

           /*if(read(sock, buffer, sizeof(buffer)) < 0)
            {
                fprintf(stderr, "Chyba pri citani zo socketu\n");
            }
            printf(buffer);*/
        //}
        puts("Echo: ");
        //if (n < 0) error("ERROR reading from socket");
            //Receive a reply from the server
        if( recv(sock, buffer, 1024, 0) < 0)
        {
            puts("recv failed");
        }
        fprintf(stderr, "Reply received\n%s\n", buffer);
        /*
        puts("Reply received\n");
        puts(buffer);*/
        /*while ((c = fgetc(fp)) != EOF) {
            putchar(c);
            if (c == '\n')
                break;
        }   */
        //puts(servaddr_reply);             

    }


    // send a string to the server
    /*send(sock, argv[2], strlen(argv[2]), 0);
        */
    // read & print string from the server


    // close socket
    close(sock);

    exit(0);
}   