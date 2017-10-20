/*--------------------------------------------------------------------*/
/* conference server */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

#include <stdlib.h>
#define MAXNAMELEN 256
extern char * recvtext(int sd);
extern int sendtext(int sd, char *msg);

extern int startserver();
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int fd_isset(int fd, fd_set *fsp) {
	return FD_ISSET(fd, fsp);
}

/* main routine */
int main(int argc, char *argv[]) {
	int servsock; /* server socket descriptor */

	fd_set livesdset; /* set of live client sockets */
	int livesdmax; /* largest live client socket descriptor */

	/* check usage */
	if (argc != 1) {
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}
	/* get ready to receive requests */
	servsock = startserver();
	if (servsock == -1) {
		perror("Error on starting server: ");
		exit(1);
	}
	/*
	 FILL HERE:
	 init the set of live clients
	 */
	// struct timeval tv;
	// tv.tv_sec = 30;
	// tv.tv_usec = 0;

	livesdmax = -1;
	FD_ZERO(&livesdset);
	FD_SET(servsock,&livesdset);
	fd_set temp;
	if(livesdmax < servsock)
			livesdmax = servsock;


	/* receive requests and process them */
	while (1) {
		int frsock; /* loop variable */
		FD_ZERO(&temp);
		temp = livesdset;
		
		/*
		 FILL HERE
		 wait using select() for
		 messages from existing clients and
		 connect requests from new clients
		 */

		if(select(livesdmax + 1, &temp, NULL, NULL, NULL)<0)
		{
			printf("select error \n");
		}

		/* look for messages from live clients */
		for (frsock = 3; frsock <= livesdmax; frsock++) {
			
			/* skip the listen socket */
			/* this case is covered separately */
			if (frsock == servsock)
				continue;


			if (FD_ISSET(frsock,&temp)!=0/* FILL HERE: message from client 'frsock'? */ ) {
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */
				
				/*
				 FILL HERE:
				 figure out client's host name and port
				 using getpeername() and gethostbyaddr()
				 */
				struct sockaddr_in clientaddr;
  				socklen_t addrlen = sizeof(clientaddr);
  				memset(&clientaddr, 0, sizeof(clientaddr));
  				getpeername(frsock, (struct sockaddr *)&clientaddr , &addrlen);
  				clienthost = (char*)malloc(MAXNAMELEN *sizeof(char));
  				struct hostent *hp;
  				hp = gethostbyaddr(&(clientaddr.sin_addr.s_addr), sizeof(clientaddr.sin_addr.s_addr), AF_INET);
  				strcpy(clienthost, hp->h_name);
				clientport = ntohs(clientaddr.sin_port);

				/* read the message */
				char *msg;
				msg = recvtext(frsock);
				if (!msg) {
					/* disconnect from client */
					printf("admin: disconnect from '%s(%hu)'\n", clienthost, clientport);
					/*
					 FILL HERE:
					 remove this guy from the set of live clients
					 */
					FD_CLR(frsock, &temp);
					FD_CLR(frsock, &livesdset);
					
					/* close the socket */
					close(frsock);
					
				} else {
					
					/*
					 FILL HERE
					 send the message to all live clients
					 except the one that sent the message
					 */
					int j;
					for(j=3; j<=livesdmax; j++){
						if(FD_ISSET(j, &livesdset) && j!= frsock && j!=servsock)
							sendtext(j,msg);
					}
					/* display the message */
					printf("%s(%hu): %s", clienthost, clientport, msg);

					/* free the message */
					free(msg);
				}
			}
		}

		/* look for connect requests */
		if ( FD_ISSET(servsock, &temp)!=0 /* FILL HERE: connect request from a new client? */ ) {
			/*
			 FILL HERE:
			 accept a new connection request
			 */
			struct sockaddr_in addclient;
			socklen_t clienlen = sizeof(addclient);
			int csd;
			csd = accept(servsock, (void *)&addclient, &clienlen);

			/* if accept is fine? */
			if (csd != -1) {
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */

				/*
				 FILL HERE:
				 figure out client's host name and port
				 using gethostbyaddr() and without using getpeername().
				 */
				if(csd > livesdmax){
					 livesdmax = csd;
				 }
				struct sockaddr_in newclientaddr;
  				socklen_t newaddrlen = sizeof(newclientaddr);
  				memset(&newclientaddr, 0, sizeof(newclientaddr));
  				newclientaddr = addclient;
  				clienthost = (char*)malloc(MAXNAMELEN *sizeof(char));
  				struct hostent *hp;
  				hp = gethostbyaddr(&(newclientaddr.sin_addr.s_addr), sizeof(newclientaddr.sin_addr.s_addr), AF_INET);
  				strcpy(clienthost, hp->h_name);
				clientport = ntohs(newclientaddr.sin_port);
				printf("admin: connect from '%s' at '%hu'\n", clienthost, clientport);

				/*
				 FILL HERE:
				 add this guy to set of live clients
				 */
				FD_SET(csd, &livesdset);

			} else {
				perror("accept");
				exit(0);
			}

		}
	}
	return 0;
}
/*--------------------------------------------------------------------*/

