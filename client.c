#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define ADDRESS         "mysocket"  /* addr to connect */
#define STDIN           0
#define MAX_LENGTH      1024
#define SEND_FILE       "Sending file"

void reset_string_memory(char* buffer)
{
    memset(buffer,'\0', sizeof(buffer));
    memset(buffer, 0, MAX_LENGTH);
}

int main(int argc, char *argv[])
{
    char c;
    FILE *fp;
    register int i, sock, len, opt;
    struct sockaddr_un address;
    char msg[MAX_LENGTH], servaddr_reply[MAX_LENGTH], buffer[MAX_LENGTH], file_name[MAX_LENGTH];
    fd_set rs;
    ssize_t r, w;
    int valread, activity;



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
        FD_ZERO(&rs);
        //add master socket to set
        FD_SET(STDIN, &rs);
        FD_SET(sock, &rs);

        activity = select(sock + 1, &rs, NULL, NULL, NULL);

        if (FD_ISSET(sock, &rs)) 
        {
            if (recv(sock, buffer, MAX_LENGTH, 0) < 0)
            {
                puts("recv failed");
            }
            if (strcmp(buffer, "shutdown") == 0 || strcmp(buffer, "Server is full, sorry :(") == 0)
            {
                fprintf(stderr, "%s\n", buffer);
                break;
            }
            else
            {
                fprintf(stderr, "Reply received: %s\n", buffer);
                reset_string_memory(buffer);
            }
        }
        else if (FD_ISSET(STDIN, &rs)) 
        {
            valread = read(0, buffer, MAX_LENGTH);
            buffer[valread - 1] = 0;
            printf("Enter msg: ");
            scanf("%s" , msg);

            if (strcmp(msg, "exit") == 0)
            {
                break;
            }

            // run function - sending a file to the server
            if (strcmp(msg, "run") == 0)
            {
                strcpy(buffer, SEND_FILE);
                send(sock, buffer, strlen(buffer), 0);
                reset_string_memory(buffer);
                printf("Enter name of the file: ");
                scanf("%s", file_name);
                send(sock, file_name, strlen(file_name), 0);
                int amount;
                recv(sock, &amount, sizeof(amount), 0);
                fprintf(stderr, "Number of lines: %d\n", amount);
                recv(sock, &amount, sizeof(amount), 0);
                fprintf(stderr, "Numer of words: %d\n", amount);
                recv(sock, &amount, sizeof(amount), 0);
                fprintf(stderr, "Number of letters: %d\n", amount);
                recv(sock, &amount, sizeof(amount), 0);
                fprintf(stderr, "Number of characters: %d\n", amount);
            }
            else
            {
                if (send(sock, msg, strlen(msg), 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
            }
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
        //uts("Echo: ");
        //if (n < 0) error("ERROR reading from socket");
            //Receive a reply from the server
        /*if( recv(sock, buffer, 1024, 0) < 0)
        {
            puts("recv failed");
        }*/
        
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