#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#define ADDRESS     "mysocket"  /* addr to connect */
#define WELCOME		"Welcome to the server!\n"
#define MAXMSG  	1024
#define STDIN 		0

char* read_from_client (int filedes)
{
  char* buffer = (char*)malloc(MAXMSG * sizeof(char));
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
	char* buffer = (char*)malloc(MAXMSG * sizeof(char));

	strcpy(buffer, "Help");

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
	fd_set active_fd_set, read_fd_set, readfds;
	size_t size;
	char* test_buffer = (char*)malloc(MAXMSG * sizeof(char));
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
	unlink(ADDRESS);
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
				memset(buffer,'\0', sizeof(buffer));
			    memset(buffer, 0, 1024);
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
        	valread = read(0, buffer, 1024);
        	buffer[valread - 1] = 0;

        	if(strcmp(buffer, "quit") == 0){
        		fprintf(stderr, "quit iniciated\n");
        		// let all connected clients know that the server is shutting down
        		for (i = 0; i < max_clients; i++) {
					sd = client_socket[i];
					if(sd > 0){
						strcpy(buffer, "shutdown");
						send(sd, buffer, strlen(buffer), 0);
						close(sd);
					}
				}
				fprintf(stderr, "Server was shut down\n");
        		break;
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
	                if ((valread = read( sd , buffer, 1024)) == 0)
	                {
	                    //Somebody disconnected , get his details and print
	                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
	                    printf("Client with socket fd %d disconnected\n", sd);
	                      
	                    //Close the socket and mark as 0 in list for reuse
	                    close(sd);
	                    client_socket[i] = 0;
	                    number_of_clients--;
	                }
	                  
	                //Echo back the message that came in
	                else
	                {

	                	fprintf(stderr, "Got message:'%s' from a client with socket fd: %d\n", buffer, sd);
	                    //set the string terminating NULL byte on the end of the data read
	                    buffer[valread] = '\0';
	                    send(sd, buffer, strlen(buffer), 0);
						memset(buffer,'\0', sizeof(buffer));
			        	memset(buffer, 0, 1024);
	                }
	            }
	        }
    	}

    }while(1);
    return 0;
}




















/*
	    if (FD_ISSET(, &read_fd_set, NULL, NULL, NULL) < 0)
	    {
	        perror ("select");
	        exit (1);
	    }

	      Service all the sockets with input pending. 
      	for (i = 0; i < FD_SETSIZE; ++i)
      	{
        	if (FD_ISSET (i, &read_fd_set))
          	{
            	if (i == s)
	            {
	                /* Connection request on original socket. 

	                size = sizeof (address);
	                new = accept (s,
	                              (struct sockaddr *) &address,
	                              &size);
	                fp = fdopen(ns, "r");
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
                	/* Data arriving on an already-connected socket. 
                	strcpy(test_buffer, read_from_client(i));
                	fprintf(stderr, "Idk");
                	//printf(buffer);
                    strcpy(buffer, WELCOME);
					//printf("%d, %s\n", strlen(buffer), buffer);
					//write(s, buffer , strlen(buffer));
					send(new, buffer , strlen(buffer), 0);
					//if (n < 0) error("ERROR writing to socket");
                    {
                    	send_help_to_client();
                    }
                	if (test_buffer != NULL)
                  	{
						close (i);
                    	FD_CLR (i, &active_fd_set);


                  	}
              	}
          	}
    	}


			        /*memset(client_message,'\0',sizeof(client_message));
			        memset(client_message, 0, 2000); */ 
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
		
		
	} while (1);
	/*
	* We can simply use close() to terminate the
	* connection, since we're done with both sides.
	
	close(s);

	exit(0);
}*/