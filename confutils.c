/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

#include <stdlib.h>

#define MAXNAMELEN 256
/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* prepare server to accept requests
 returns file descriptor of socket
 returns -1 on error
 */
int startserver() {
	int sd; /* socket descriptor */

	char * servhost; /* full name of this host */
	ushort servport; /* port assigned to this server */

	/*
	 FILL HERE
	 create a TCP socket using socket()
	 */
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd < 0 )
	{
		printf("socket error\n");
		exit(-1);
	}
	else
	{
		printf("socket() successed\n");
	}

	/*
	 FILL HERE
	 bind the socket to some port using bind()
	 let the system choose a port
	 */
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port=htons(0);
	sa.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(sd, (struct sockaddr *) &sa,sizeof(sa))== -1)
	{
		printf("bind() failed\n");
	}
	else
  		printf("Successfully bound to port %u\n", sa.sin_port);

	/* we are ready to receive connections */
	listen(sd, 5);
	printf("======waiting for client's request======\n");//added by fang

	/*
	 FILL HERE
	 figure out the full host name (servhost)
	 use gethostname() and gethostbyname()
	 full host name is remote**.cs.binghamton.edu
	 */

	servhost = (char*)malloc(MAXNAMELEN *sizeof(char));

    if( gethostname(servhost, MAXNAMELEN) < 0 )
 		return -1;

 	struct hostent *hp;

    if( (hp = gethostbyname(servhost)) < 0 )
 		return -1;
    strcpy(servhost, hp->h_name);

    //printf("hostname: %s\n",servhost);
    //  int i = 0;
    // while(hp->h_addr_list[i] != NULL)
    // {
    //         printf("hostname: %s\n",hp->h_name);
    //         printf("ip: %s\n",inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]));
    //         i++;
    // }

	/*
	 FILL HERE
	 figure out the port assigned to this server (servport)
	 use getsockname()
	 */
	struct sockaddr_in sockaddr;
  	socklen_t addrlen = sizeof(sockaddr);
  	memset(&sockaddr, 0, sizeof(sockaddr));

	if (getsockname(sd, (struct sockaddr *)&sockaddr, &addrlen) < 0)
	{
    	printf("getsockname error\n");
    	exit(-1);
  	}
  	servport = ntohs(sockaddr.sin_port);

	/* ready to accept requests */
	printf("admin: started server on '%s' at '%hu'\n", servhost, servport);
	free(servhost);
	return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 establishes connection with the server
 returns file descriptor of socket
 returns -1 on error
 */
int hooktoserver(char *servhost, ushort servport) {
	int sd; /* socket descriptor */

	ushort clientport; /* port assigned to this client */

	/*
	 FILL HERE
	 create a TCP socket using socket()
	 */
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd < 0 )
	{
		printf("socket error\n");
		exit(-1);
	}
	else
	{
		printf("socket() successed\n");
	}

	/*
	 FILL HERE
	 connect to the server on 'servhost' at 'servport'
	 use gethostbyname() and connect()
	 */

	struct sockaddr_in caddr;
	memset(&caddr, 0, sizeof(caddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port=htons(servport);
  	socklen_t caddrlen = sizeof(caddr);
  	

 	struct hostent *hp;

    if( (hp = gethostbyname(servhost)) < 0 )
 		return -1;
 	else
 	{
 		memcpy(&caddr.sin_addr,hp->h_addr,hp->h_length);
 	}


    connect(sd, (struct sockaddr *)&caddr, sizeof(struct sockaddr_in));

	/*
	 FILL HERE
	 figure out the port assigned to this client
	 use getsockname()
	 */
  	memset(&caddr, 0, sizeof(caddr));
	if (getsockname(sd, (struct sockaddr *)&caddr, &caddrlen) < 0)
	{
    	printf("getsockname error\n");
    	exit(-1);
  	}
  	clientport = ntohs(caddr.sin_port);


	/* succesful. return socket descriptor */
	printf("admin: connected to server on '%s' at '%hu' thru '%hu'\n", servhost,
			servport, clientport);
	printf(">");
	fflush(stdout);
	return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
int readn(int sd, char *buf, int n) {
	int toberead;
	char * ptr;

	toberead = n;
	ptr = buf;
	while (toberead > 0) {
		int byteread;

		byteread = read(sd, ptr, toberead);
		if (byteread <= 0) {
			if (byteread == -1)
				perror("read");
			return (0);
		}

		toberead -= byteread;
		ptr += byteread;
	}
	return (1);
}

char *recvtext(int sd) {
	char *msg;
	long len;

	/* read the message length */
	if (!readn(sd, (char *) &len, sizeof(len))) {
		return (NULL);
	}
	len = ntohl(len);

	/* allocate space for message text */
	msg = NULL;
	if (len > 0) {
		msg = (char *) malloc(len);
		if (!msg) {
			fprintf(stderr, "error : unable to malloc\n");
			return (NULL);
		}

		/* read the message text */
		if (!readn(sd, msg, len)) {
			free(msg);
			return (NULL);
		}
	}

	/* done reading */
	return (msg);
}

int sendtext(int sd, char *msg) {
	long len;

	/* write lent */
	len = (msg ? strlen(msg) + 1 : 0);
	len = htonl(len);
	write(sd, (char *) &len, sizeof(len));

	/* write message text */
	len = ntohl(len);
	if (len > 0)
		write(sd, msg, len);
	return (1);
}
/*----------------------------------------------------------------*/

