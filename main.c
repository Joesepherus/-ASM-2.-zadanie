#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>

#define ADDRESS     	"mysocket"  /* addr to connect */
#define ADRESS 			"mysocke"
#define WELCOME			"Welcome to the server!\n"
#define MAX_LENGTH  	1024
#define STDIN 			0
#define SEND_FILE		"Sending file"
#define DATE_AND_TIME	"Current date and time: "
#define SEND_INFO		"Send me info"
#define MAX_CLIENTS 	5

// TO DO dat do kniznice lebo to pouziva aj client.c
void reset_string_memory(char* buffer)
{
	//memset(buffer,'\0', sizeof(buffer));
	memset(buffer, 0, MAX_LENGTH);
}

char* read_from_client (int filedes)
{
  char* buffer = (char*)malloc(MAX_LENGTH * sizeof(char));
  int nbytes;

  nbytes = read (filedes, buffer, MAX_LENGTH);
  if (nbytes < 0)
    {
      /* Read error. */
      perror ("read");
      exit (1);
    }
  else if (nbytes == 0)
    /* End-of-file. */
    return NULL;
  else
    {
      /* Data read. */
      fprintf (stderr, "Server: got message: '%s'\n", buffer);
    }
    return buffer;
}

void send_help_to_client()
{
	char* buffer = (char*)malloc(MAX_LENGTH * sizeof(char));

	strcpy(buffer, "Help");

}

char* current_date_and_time()
{
  	char* buffer = (char*)malloc(MAX_LENGTH * sizeof(char));
  	char* buffer1 = (char*)malloc(MAX_LENGTH * sizeof(char));

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	// itoa doesn't work on student :) so we had to use snprintf which is safer than sprintf 

	snprintf(buffer1, MAX_LENGTH, "%d", tm.tm_mday);
	strcat(buffer, buffer1);
	strcat(buffer, " ");

	snprintf(buffer1, MAX_LENGTH, "%d", tm.tm_mon + 1);
	strcat(buffer, buffer1);

	snprintf(buffer1, MAX_LENGTH, "%d", tm.tm_year + 1900);
	strcpy(buffer, buffer1);
	strcat(buffer, "-");

	snprintf(buffer1, MAX_LENGTH, "%d", tm.tm_hour);
	strcat(buffer, buffer1);
	strcat(buffer, ":");

	snprintf(buffer1, MAX_LENGTH, "%d", tm.tm_min);
	strcat(buffer, buffer1);
	strcat(buffer, ":");

	snprintf(buffer1, MAX_LENGTH, "%d", tm.tm_sec);
	strcat(buffer, buffer1);


	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return buffer;
}

server_quit(int client_socket[])
{
	int i, sd;
	char buffer[MAX_LENGTH];
 	fprintf(stderr, "quit iniciated\n");
    // let all connected clients know that the server is shutting down
    for (i = 0; i < MAX_CLIENTS; i++) {
		sd = client_socket[i];
		if(sd > 0){
			strcpy(buffer, "Server is shutting down");
			send(sd, buffer, strlen(buffer), 0);
			close(sd);
		}
	}
	fprintf(stderr, "Server has been shut down\n");
}		

void info(int sd){
	struct timeval time_val, start, end, total;
	time_t seconds;
	char date[65];
	char time[65];
	struct rusage usage, use;
	int i, j, a = 0, who = 0;
	long mem;


	printf("Informations incialized:\n");

	asm(
		"movl $116, %%eax;"
		"push $0;"	
		"push %0;"
		"push $0;"
		"int $0x80;"
		:
		: "b" (&time_val)
		);

	for(i = 0; i < 10000; i++)
	{
		for(j = 0; j < 1000; j++)
		{
			a ++;
			a *= 87;
			a -= 2;
		}
	}

	asm(
		"movl $117, %%eax;"
		"push %0;"
		"push $0;"
		"push $0;"
		"int $0x80;"
		:
		: "b" (&use)
		);

	mem = use.ru_maxrss;
	total = use.ru_utime;
	seconds = time_val.tv_sec;

	strftime(date, 65, "%d.%m.%Y", localtime(&seconds));
	strftime(time, 65, "%T", localtime(&seconds));
	if(sd == NULL)
	{
		printf("Current date: %s\nCurrent time: %s\n", date, time);
		printf("Total CPU usage: %ld.%06lds\n", total.tv_sec, total.tv_usec);
		printf("Memory used: %ldKB\n", mem);
	}
	else
	{
		if (send(sd, date, strlen(date), 0) < 0)
		{
            puts("Send failed");
    	}
		send(sd, time, strlen(time), 0);
		send(sd, &total.tv_sec, sizeof(total.tv_sec), 0);
		send(sd, &total.tv_usec, sizeof(total.tv_usec), 0);
		send(sd, &mem, sizeof(mem), 0);
	}
}	


int server()
{
	char c;
	FILE *fp;
	int fromlen;
	register int i, s, ns, len, sock;
	struct sockaddr_un address, fsaun, clientname;
	time_t t;
	struct tm *tm_info;
	char buffer[MAX_LENGTH];
	fd_set active_fd_set, read_fd_set, readfds;
	size_t size;
	char* test_buffer = (char*)malloc(MAX_LENGTH * sizeof(char));
	pid_t pid;
	int new, max_sd, number_of_clients = 0;
	int opt = 1;
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 5 , activity, valread , sd;
    char *message = "Welcome to the server, friend :)\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }

     if( (master_socket = socket(AF_UNIX , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
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
	unlink(ADRESS);
	len = sizeof(address.sun_family) + strlen(address.sun_path);

	if (bind(master_socket, (struct sockaddr *)&address, len) < 0) 
	{
		perror("server: bind");
		exit(1);
	}

	/*
	* Listen on the socket.
	*/
	if (listen(master_socket, 5) < 0) {
		perror("server: listen");
		exit(1);
	}

	//accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

	do {
       	FD_ZERO(&readfds);
		//add master socket to set
		FD_SET(STDIN, &readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
         
        //add child sockets to set
        for ( i = 0 ; i < 5 ; i++) 
        {
            //socket descriptor
            sd = client_socket[i];
             
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET(sd, &readfds);
             
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);
		if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) 
        {
        	number_of_clients++;
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                perror("accept");
                exit(1);
            }

            if (number_of_clients > max_clients){
				strcpy(buffer, "Server is full, sorry :(");
				send(new_socket, buffer, strlen(buffer), 0);
				reset_string_memory(buffer);
            }
            else{
	            //inform user of socket number - used in send and receive commands
	            printf("New connection, socket fd is %d\n" , new_socket);
	        
	            //send new connection greeting message
	            if(send(new_socket, message, strlen(message), 0) != strlen(message) ) 
	            {
	                perror("send");
	            }
	              
	            puts("Welcome message sent successfully");
	              
	            //add new socket to array of sockets
	            for (i = 0; i < max_clients; i++) 
	            {
	                //if position is empty
	                if( client_socket[i] == 0 )
	                {
	                    client_socket[i] = new_socket;
	                    printf("Adding to list of sockets as %d\n" , i);
	                    break;
	                }
	            }
        	}
        }
		else if (FD_ISSET(STDIN, &readfds)) 
        {
        	valread = read(0, buffer, MAX_LENGTH);
        	buffer[valread - 1] = 0;

        	if(strcmp(buffer, "quit") == 0)
        	{
        		server_quit(client_socket);
        		/*fprintf(stderr, "quit iniciated\n");
        		// let all connected clients know that the server is shutting down
        		for (i = 0; i < max_clients; i++) {
					sd = client_socket[i];
					if(sd > 0){
						strcpy(buffer, "shutdown");
						send(sd, buffer, strlen(buffer), 0);
						close(sd);
					}
				}
				fprintf(stderr, "Server has been shut down\n");*/
				close(master_socket);
        		return 0;
        	}
        	if(strcmp(buffer, "info") == 0)
        	{
        		info(NULL);
        	}
        }
        //else its some IO operation on some other socket :)
        else{
	        for (i = 0; i < max_clients; i++) 
	        {
	            sd = client_socket[i];
	              
	            if (FD_ISSET(sd, &readfds)) 
	            {
	                //Check if it was for closing , and also read the incoming message
	                if ((valread = recv(sd, buffer, MAX_LENGTH, 0)) == 0)
	                {
	                    //Somebody disconnected , get his details and print
	                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
	                    printf("Client with socket fd %d disconnected\n", sd);
	                      
	                    //Close the socket and mark as 0 in list for reuse
	                    close(sd);
	                    client_socket[i] = 0;
	                    number_of_clients--;
	                }
	                else if (strcmp(buffer, SEND_FILE) == 0)
	                {
	                	fprintf(stderr, "Server is receiving file from client with a socket fd %d\n", sd);
						reset_string_memory(buffer);
	                	valread = read(sd, buffer, MAX_LENGTH);
	                	fprintf(stderr, "Name of the file: %s\n", buffer);
	                	// run vypocita počet riadkov, slov a znakov
	                	FILE *file_for_reading;
	                	file_for_reading = fopen(buffer, "r");
	                	char char_read_from_file, previous_char = NULL;
	                	int number_of_lines = 0, number_of_words = 0, number_of_letters = 0, number_of_characters = 0;
	                	while ((char_read_from_file = fgetc(file_for_reading)) != EOF)
	                	{             		
	                		if (char_read_from_file == '\n')
	                		{
	                			number_of_lines++;
	                		}
	                		if((char_read_from_file >= 65 && char_read_from_file <= 90) ||
	                			(char_read_from_file >= 97 && char_read_from_file <= 122))
	                		{
	                			number_of_letters++;
	                		}
	                		if (((previous_char >= 65 && previous_char <= 90) ||
	                			(previous_char >= 97 && previous_char <= 122)) &&
	                			!((char_read_from_file >= 65 && char_read_from_file <= 90) ||
	                			(char_read_from_file >= 97 && char_read_from_file <= 122)))
	                		{
	                			number_of_words++;
	                		}
	                		previous_char = char_read_from_file;
	                		number_of_characters++;
	                		//printf("%c", char_read_from_file);
	                	}
	                	/*printf("number of lines = %d\nnumber of word = %d\nnumber of letters = %d\nnumber of characters = %d\n",
	                			number_of_lines, number_of_words, number_of_letters, number_of_characters);*/
						send(sd, &number_of_lines, sizeof(number_of_lines), 0);
						send(sd, &number_of_words, sizeof(number_of_words), 0);
						send(sd, &number_of_letters, sizeof(number_of_letters), 0);
						send(sd, &number_of_characters, sizeof(number_of_characters), 0);
	                	fclose(file_for_reading);
	                }
	                else if (strcmp(buffer, SEND_INFO) == 0)
	                {
	                	fprintf(stderr, "Server is processing info from socket %d\n", sd);
						reset_string_memory(buffer);
	                	info(sd);
	                	//strcpy(buffer, DATE_AND_TIME);
	                	//current_date_and_time();
    				 	//strcat(buffer, current_date_and_time());
    				 	//send(sd, buffer, strlen(buffer), 0);
    				 	//reset_string_memory(buffer);

    				 	// TO DO consumed processor time
    				 	// TO DO used memory
	                }
	                else if (strcmp(buffer, "halt") == 0)
	                {
	                	server_quit(client_socket);
	                	close(master_socket);
	                	return 0;
	                }
	                //Echo back the message that came in
	                else
	                {
	                	fprintf(stderr, "Got message:'%s' from a client with socket fd: %d\n", buffer, sd);
	                    //set the string terminating NULL byte on the end of the data read
	                    buffer[valread] = '\0';
	                    send(sd, buffer, strlen(buffer), 0);
						reset_string_memory(buffer);
	                }
	            }
	        }
    	}

    }while(1);
    return 0;
}



int client()
{
    char c;
    FILE *fp;
    register int i, sock, len, opt;
    struct sockaddr_un address;
    char msg[MAX_LENGTH], servaddr_reply[MAX_LENGTH], buffer[MAX_LENGTH], file_name[MAX_LENGTH];
    fd_set rs;
    ssize_t r, w;
    int valread, activity;

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
            if (strcmp(buffer, "Server is shutting down") == 0 || strcmp(buffer, "Server is full, sorry :(") == 0)
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
            /*valread = read(0, buffer, MAX_LENGTH);
            buffer[valread - 1] = 0;*/
            //printf("Enter msg: ");
            scanf("%s", buffer);

            if (strcmp(buffer, "quit") == 0)
            {
            	fprintf(stderr, "Closing client now\n");
                break;
            }

            // run function - sending a file to the server
            if (strcmp(buffer, "run") == 0)
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

            if(strcmp(buffer, "info") == 0)
            {
            	strcpy(buffer, SEND_INFO);
            	send(sock, buffer, strlen(buffer), 0);
            	reset_string_memory(buffer);

            	char date[65];
            	char time[65];
            	long sec, usec, mem;

            	recv(sock, &date, sizeof(date), 0);
            	printf("Current date: %s\n", date);
            	recv(sock, &time, sizeof(time), 0);
            	printf("Current time: %s\n", time);
            	recv(sock, &sec, sizeof(sec), 0);
            	printf("Total CPU usage: %ld", sec);
            	recv(sock, &usec, sizeof(usec), 0);
            	printf(".%lds\n", usec);
            	recv(sock, &mem, sizeof(mem), 0);
            	printf("Memory used: %ldKB\n", mem);
            	/*recv(sock, total.tv_usec, sizeof(total.tv_usec), 0);
            	recv(sock, mem, sizeof(mem), 0);*/
            }
            else
            {
                if (send(sock, buffer, strlen(buffer), 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
            }
        }       
    }

    // close socket
    close(sock);

   	return 0;
}   


int main(int argc, char *argv[])
{
	FILE *fp;
	int fromlen;
	register int i, s, ns, len, sock;
	struct sockaddr_un address, fsaun, clientname;
	time_t t;
	struct tm *tm_info;
	char buffer[MAX_LENGTH];
	fd_set active_fd_set, read_fd_set, readfds;
	size_t size;
	char* test_buffer = (char*)malloc(MAX_LENGTH * sizeof(char));
	pid_t pid;
	int new, max_sd, number_of_clients = 0;
	int opt = 1;
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 5 , activity, valread , sd;
    char *message = "Welcome to the server, friend :)\n";
    int c = 0;
	char *log_name = (char *) malloc(256 * sizeof(char));
    strcpy(log_name, "client_log.txt");

    // Nacita konfiguracny subor z premennej prostredia
    if (getenv("PROJECT_CONFIG") != NULL) {
        FILE *config_file = fopen(getenv("PROJECT_CONFIG"), "r");
        if (config_file) {
            fgets(log_name, 255, config_file);
            log_name[strlen(log_name) - 1] = '\0';
            printf("%s\n", log_name);
        } else {
            perror("Chyba otvarania konfiguracneho suboru!");
        }
        fclose(config_file);
    }

    while ((opt = getopt(argc, argv, "sc")) != -1)
    {
        switch (opt)
        {
        case 's':
            break;
        case 'c':
        	c = 1;
        	break;
        default:
            printf("Error: '%c': nesparvny prepinac", argv[0]);
            exit(1);
        }
    }
    if(c)
        client();
    else server();
}

