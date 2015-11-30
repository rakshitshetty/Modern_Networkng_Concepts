/***reference:
http://www.binarytides.com/socket-programming-c-linux-tutorial/
http://www.binarytides.com/server-client-example-c-sockets-linux/
http://www.binarytides.com/multiple-socket-connections-fdset-select-linux/
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
http://mcalabprogram.blogspot.com/2012/01/ftp-sockets-server-client-using-c.html
recitation slides
***/
/*
creator:rshetty
person#50133314
*/




#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include<errno.h>
#include<netdb.h>

#include<arpa/inet.h>
#include<sys/time.h>
#include <netinet/in.h>

#include<string.h>
#include<sys/socket.h>

#include<signal.h>

#define length 800


//Delcare global an static variables used
static char *passVal[30];
static char myhost[1024];
static char ipaddr[100];
int portno;
static int clsockfd;
static int peerconsockfd;

static int registerflag;
static int clientfd[4];
int maxclients=4;


struct statistics
{
	int id;
	char host1[100];
	char host2[100];
	double uploadtime;
	int uploadnum;
	double downloadtime;
	int downloadnum;
};




struct servL
{
	int id;
	char hostname[2000];
	char ipaddr[100];
	int portno;
	int sockfd;
};

struct cltL
{
	int id;
	char hostname[2000];
	char ipaddr[100];
	int portno;
	int sockfd;
};




void myip()
{
	char choice='y';


			/*Variable declarations*/
			struct addrinfo hints, *info, *p;

			struct sockaddr_in *sa;
			struct sockaddr_in6 *sa6;

			int gai_result; // For result of getaddrinfo()

			char hostname[1024]; //For hostname
			hostname[1023] = '\0';
			//char ip_addr[100]; //For IP
			ipaddr[100]='\0';


			gethostname(hostname, 1023);
			printf("Hostname: %s\n", hostname);


			/*Using modern getaddrinfo and sockaddr*/
			/*set hints parameters to call specific addrinfo*/
			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_CANONNAME;

			/*getaddrinfo + standard error check*/

			/****************check with second parameter null********************************************/
			if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
			    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
			    exit(1);
			}

			/* loop through all the results and connect to the first we can */
			for(p = info; p != NULL; p = p->ai_next) {

				printf("Hostname: %s\n", p->ai_canonname);  //For printing hostname


				//sa = (struct sockaddr_in *) p->ai_addr; //Get binary address *****Test 1.a with only this and deprecated on 1.b this and deprecated off****

				/* Deprecated way - for ip address --- start
				strcpy(ip_addr , inet_ntoa( sa->sin_addr ) );
				printf("IP address: %s" , ip_addr);
				Deprecated way - for ip address --- end */


				/* for IPv4 or IPV6 using ntop - n/w to presentation - beejguide*/
				switch(p->ai_family) {
					case AF_INET:
						sa = (struct sockaddr_in *) p->ai_addr; //Get binary address ***** Test 2 with this and below active
						inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), ipaddr, 99);
						break;

					case AF_INET6:
						sa6 = (struct sockaddr_in *) p->ai_addr; //Get binary address in case  ***** Test 2 with this and above active
						inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), ipaddr, 99);
	            		break;

					default:
						strncpy(ipaddr, "Unknown AF", 99);
	            		return ;
	    			}
				printf("IP address : %s\n", ipaddr);
			}

			//freeaddrinfo(info);

			//return ;
}




    /*-----SERVER-----*/



void server (char var, char **argv)
{
	myip();
	portno=atoi(argv[2]);

	int maxsd;
	int i=0,j=0;
	char *pt;
  	char *userinput[5];
	int true=1;
	int gaioutput;
	int n,repeat,full;
	int argc=0;
	char buffer[2048];

	char temp[10];
	fd_set readfds;
	char hostname[2000];

	char input[2000];
	int mastersocket,newsocket;
	int sd;





	for (i = 0; i < maxclients; i++)
    	{
    	    clientfd[i] = -1;
    	}


	/*Structures*/
	struct servL s[5]; // Structure holding list values
	struct statistics stat_s[30],stat_c[30];
	struct addrinfo hints, *info, *p ; //addrinfo structure
	struct sockaddr_in *sa, sin; //IPV4
	struct hostent *hostentry;
	struct sockaddr_in6 *sa6; // IPV6


	printf("\n\nThis is a : %c\n", var);

	gethostname(hostname, 2000);
	printf("hostname: %s\n", hostname);


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;




	if ((gaioutput = getaddrinfo(hostname, argv[2], &hints, &info)) != 0)
	{
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gaioutput));
	    exit(1);
	}


	//init server
	for (i=0;i<5;i++)
	{
		s[i].id=0;
		strcpy(s[i].hostname,"--");
		strcpy(s[i].ipaddr,"--");
		s[i].portno=0;
		s[i].sockfd=0;
	}

	//Set id of server
	s[0].id=1;


//init values

	for (i=0;i<20;i++)
	{
		//server statistics
		stat_s[i].id=0;
		strcpy(stat_s[i].host1,"--");
		strcpy(stat_s[i].host2,"--");
		stat_s[i].uploadtime=0;
		stat_s[i].uploadnum=0;
		stat_s[i].downloadtime=0;
		stat_s[i].downloadnum=0;

		//client statistics
		stat_c[i].id=0;
		strcpy(stat_c[i].host1,"--");
		strcpy(stat_c[i].host2,"--");
		stat_c[i].uploadtime=0;
		stat_c[i].uploadnum=0;
		stat_c[i].downloadtime=0;
		stat_c[i].downloadnum=0;

	}



	for(p = info; p != NULL; p = p->ai_next)
	{

		int test;

		//create a master socket

		mastersocket = socket(p->ai_family, p->ai_socktype, IPPROTO_TCP);

     	if (mastersocket == -1)
		{
    			perror("socket failed");
        		//exit(EXIT_FAILURE);
		}
        else
		{
		       printf("Server:master socket success.\n");
		}



   		setsockopt(mastersocket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

		//bind the socket

		if ( (test=bind(mastersocket, p->ai_addr, p->ai_addrlen))== -1)
		{
 			perror("bind failed");
        	//exit(EXIT_FAILURE);
     	}
		else
		{
			printf("Server: bind success\n");
		}


     	if ((test=listen(mastersocket, 5)) == -1)
		{
			perror("listen");
			//exit(EXIT_FAILURE);
		}
		else
		{
			printf("Server:listening .........\n\n");
		}


		while (1)
		{


	    	FD_ZERO(&readfds);

			//add master socket to set

	    	FD_SET(mastersocket, &readfds);
			maxsd=mastersocket;

	    	FD_SET(0, &readfds);        // for taking input

  			printf("\nvalue of mastersocket:%d\n",mastersocket);
			repeat=0;
			full=0;



			//adding child sockets

			for ( i = 0 ; i < maxclients ; i++)
        	{
            	//socket descriptor
            	sd = clientfd[i];


            	if(sd > 0)
                FD_SET( sd , &readfds);

           		if(sd > maxsd)
           	    maxsd = sd;
        	}

			if (select(maxsd+1, &readfds, 0, 0, 0) < 0)
			{
	     	 	printf("select error");
	    	}

	 		// respond to user input

    		if (FD_ISSET(0, &readfds))
			{
     			printf("\nReading from stdin\n");

				gets(input);

				i = 0;

  				pt = strtok (input," ");

				while (pt != NULL)
  				{
    				userinput[i++] = pt;
  					pt = strtok (NULL, " ");
  				}

				//CREATOR

				if (strcmp(userinput[0],"creator")==0)
				{
					printf("\n\nUBIT name : rshetty \nUBemail: rshetty@buffalo.edu\n");
					printf("\nI have read and understood the course academic integrity policy at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity\n\n");

				}

				//HELP

				else if (strcmp(userinput[0],"help")==0)
				{
					printf("\n welcome to the help menu,all the commands that can be used are given below\n");
					printf("\n creator: information of the creator can be found here\n");
					printf("\n myip: ipaddress of the process\n");
					printf("\n myport : used to display the port number on which the machine is listening for incoming connections\n");
					printf("\n register <server ip> <port> : register with server whos ip is <server ip> and port number is<port>\n");
					printf("\n connect <destination> <port> : connect to a fellow client by providing hostname/ip and port number\n");
					printf("\n list : list of all active connections is displayed\n");
					printf("\n terminate <connection id> : connection mentioned through <connection id> will be terminated\n");
					printf("\n exit : all connections will be terminated\n");
					printf("\n upload <connection_id> <file_name> : the file specified at<file_name> is uploaded to the mentioned <connection_id>\n");
					printf("\n download <connection_id1> <file_name1> <connection_id2> <file_name2> <connection_id3> <file_name3> : download files from serveres specified through <connection_id1> <connection_id2> <connection_id3> \n");
					printf("\n statistics : servers updated statistics is displayed \n for client display client statistics \n");
					printf("\n");

				}

				//IP


				else if (strcmp(userinput[0],"myip")==0)
				{
					myip();
				}


				//PORT


				else if (strcmp(userinput[0],"myport")==0)
				{
					int size = sizeof(struct sockaddr);
					if (getsockname(mastersocket, (struct sockaddr *)&sin, &size) == -1)
					{
						perror("getsockname");
					}
					else
					{
						portno=ntohs(sin.sin_port);
						printf("Port number:%d\n",portno);
					}
				}

				//EXIT
				else if (strcmp(userinput[0],"exit")==0)
				{

					printf("\nServer must not exit\n");
				}
				//LIST
				else if (strcmp(userinput[0],"list")==0)
				{

					printf("%-5s%-35s%-20s%-8s\n","ID","HOSTNAME","IPADDRESS","PORT");



					strcpy(s[0].hostname,hostname);
					strcpy(s[0].ipaddr,ipaddr);//.............
					
					s[0].portno=portno;

					i=0; //Re-declaration;
					for (i=0;i<5;i++)
					{
						if (s[i].id!=0)
						{

							printf("%-5d%-35s%-20s%-8d\n",(--s[i].id),s[i].hostname,s[i].ipaddr,s[i].portno);
						}
						else
						{
							printf("%-5d%-35s%-20s%-8d\n",s[i].id,s[i].hostname,s[i].ipaddr,s[i].portno);
						}

					}
				}
				else
				{
					printf("\n error: invalid command check list of commands using help\n");
				}
     		}



    		if (FD_ISSET(mastersocket, &readfds))
			{
     			printf("\n\n connection accepted: reading request");

				struct sockaddr_in client;
				int lengthclient=sizeof(client);



      			if ((newsocket = accept(mastersocket,(struct sockaddr*)&client, (socklen_t*)&lengthclient)) < 0)
      			{
					perror("accept failed");
        			return;
      			}

				puts("connection accepted\n");

      			bzero(buffer,2048);

				//repeat ip present or not

				strcpy(buffer,inet_ntoa(client.sin_addr));
				printf("\n\n ip address from client is :%s\n\n",buffer);
				//repeats check

				for (i = 0; i < 5; i++)
            	{
					printf("ip address:%s and buffer:%s\n",s[i].ipaddr,buffer);
					if ((strcmp(s[i].ipaddr,buffer)==0))
					{
						bzero(buffer,2048);
						strcpy(buffer,"duplicate");
						test=send(newsocket, buffer , strlen(buffer) , 0);
						close(newsocket);
						printf("repeat connection received");
						repeat=1;
					}
				}

				//Check for list full

				for (i = 0; i < 5; i++)
            	{
					printf("ip address:%s and buffer:%s\n",s[i].ipaddr,buffer);
					if (s[1].id!=0&&s[2].id!=0&&s[3].id!=0&&s[4].id!=0)
					{
						bzero(buffer,2048);
						strcpy(buffer,"full");
						test=send(newsocket, buffer , strlen(buffer) , 0);
						close(newsocket);
						printf("server is maxed out cannot accept any more connections");
						full=1;
					}
				}
				if(repeat!=1&&full!=1)
				{
					bzero(buffer,2048);


      				if ((n = recv(newsocket, buffer, 2048,0)) < 0)
      				{
      					puts("recv failed\n");
      					break;
      				}
      				else
      				{
      					printf("message : %s",buffer);
      					printf("\n");
      				}



            		printf("new connection , socket fd is %d , ip is : %s , port : %d \n" , newsocket , inet_ntoa(client.sin_addr) , ntohs(client.sin_port));


            		for (i = 0; i < maxclients; i++)
            		{

                		if( clientfd[i] == -1 )
                		{
                    		clientfd[i] = newsocket;
                    		break;
                		}
            		}

            		//Add to list

            		for (j=1;j<5;j++)
            		{
            			if (s[j].id==0)
            			{
            				s[j].id=j+1;
            				hostentry=gethostbyaddr(&client.sin_addr, sizeof (client.sin_addr), AF_INET);
            				strcpy(s[j].hostname,hostentry->h_name);
            				inet_ntop(AF_INET, &client.sin_addr, s[j].ipaddr, 99);
            				s[j].portno=atoi(buffer); //String to integer
            				s[j].sockfd=newsocket;
            				break;
            			}
            		}

            		bzero(buffer,2048);

            		//server list sent to all

            		for (i = 0; i < 5; i++)
            		{
            			if (s[i].id != 0 )
            			{
            				sprintf(temp,"%d",s[i].id); // convert int to string
            				strcat(buffer,temp);
            				strcat(buffer," ");
            				strcat(buffer,s[i].hostname);
            				strcat(buffer," ");
            				strcat(buffer,s[i].ipaddr);
            				strcat(buffer," ");
            				sprintf(temp,"%d",s[i].portno); //convert int to string
            				strcat(buffer,temp);
            				strcat(buffer," ");
            				printf("\n\nMessage in buffer is %s\n\n",buffer);
            			}
	           		}

            		// Send server clients list to all connected except itself
            		for (i = 1; i < 5; i++)
            		{
            			if (s[i].id != 0 )
            			{
            				printf("\n\n sockfd=%d \n\n",s[i].sockfd);
            				test=send(s[i].sockfd, buffer , strlen(buffer) , 0);
            				if( test < 0)
            				{
            					puts("send failed\n");
            					return;
            				}
            				else
            				{
            					printf("list sent to client sockets successfully\n");
            				}
            			}
            		}
				}
			}

    		//If something happened on the socket , then its an incoming connection

    		for (i = 0; i < maxclients; i++)

    		{
            	newsocket = clientfd[i];

            	if(clientfd[i]>0)
            	{
            		if (FD_ISSET( newsocket, &readfds))
            		{

                		bzero(buffer,2048);
                		printf("client sock id in client = %d",newsocket);

                		if ((n = recv(newsocket, buffer, 2048,0)) < 0)

                			puts("recv failed");

                		else if (strcmp(buffer,"terminate")==0)
                		{
                			printf("\n Message : %s\n",buffer);




                			for (i=0;i<maxclients;i++)
                			{
                				if (newsocket==clientfd[i])
                					clientfd[i]=-1;
                				if (s[i].sockfd==newsocket)
                				{
                					s[i].id=0;
                					strcpy(s[i].hostname,"--");
                					strcpy(s[i].ipaddr,"--");
                					s[i].portno=0;
                					s[i].sockfd=0;
                				}
                			}
                			test=send(newsocket, buffer , strlen(buffer) , 0);
                			if( test < 0)
                			{
                				puts("Send failed");
                				return;
                			}
                			else
                			{
                				printf("Socket id %d has been deleted",newsocket);
                				close(newsocket);
                				bzero(buffer,2048);

                				//send servL to all
                				for (i = 0; i < 5; i++)
                				{
                					if (s[i].id != 0 )
                					{
                						sprintf(temp,"%d",s[i].id); // convert int to string
                						strcat(buffer,temp);
                						strcat(buffer," ");
                						strcat(buffer,s[i].hostname);
                						strcat(buffer," ");
                						strcat(buffer,s[i].ipaddr);
                						strcat(buffer," ");
                						sprintf(temp,"%d",s[i].portno); 								//convert int to string
                						strcat(buffer,temp);
                						strcat(buffer," ");
                						printf("\n\nMessage in buffer is %s\n\n",buffer);
                					}
                				}

                				// send server's client list to all connected except itself
                				for (i = 1; i < 5; i++)
                				{
                					if (s[i].id != 0 )
                					{
                						printf("\n\n sock FD :%d\n\n",s[i].sockfd);

                						if( (test=send(s[i].sockfd, buffer , strlen(buffer) , 0)) < 0)
                						{
                							puts("Send failed:server list to clients");
                							return;
                						}
                						else
                							printf("server list to clients sent successfully\n");
                					}
                				}
                			}
                		}
            		}
            	}
    		}
		}

	}

	freeaddrinfo(info);
}









    /*-----CLIENT------*/




void client (char var, char **argv)
{

	myip();
	portno=atoi(argv[2]);
	int mastersocket,newsocket,terminatesock,downloadsocket, uploadsocket;

	char temp[100];
	int n,gaioutput;
	int argc=0;
	int true=1;
	char hostname[2000];

	char input[2000];
	int i=0,j=0,k=0;
	char buffer[2048];
	fd_set readfds;

	int repeat,full;

	int sd,maxsd;
	char *pt;
  	char *userinput[15];
	char location[200] = "./";
	FILE *fl;
	int bytes;
	char uploadbuffer[length];
	char filename[200];
	FILE *f2;
	int writesize;
	struct timeval start,end;
	double timeS;
	double txnrate;





	struct servL s[5];
	struct cltL c[4];
	struct statistics st[30];
	struct addrinfo hints, *info, *p;
	struct sockaddr_in *sa, sin;
	struct hostent *hostentry;


	struct sockaddr_in6 *sa6;



	for (i = 0; i < maxclients; i++)
    {
       clientfd[i] = -1;
    }



	gethostname(hostname, 2000);
	printf("hostname: %s\n", hostname);
	strcpy(myhost,hostname);



	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;


	if ((gaioutput = getaddrinfo(hostname, argv[2], &hints, &info)) != 0)
	{
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gaioutput));
	    exit(1);
	}




	for (i=0;i<maxclients;i++)
	{
		c[i].id=0;
		strcpy(c[i].hostname,"--");
		strcpy(c[i].ipaddr,"--");
		c[i].portno=0;
		c[i].sockfd=0;
	}

	//value for servL initialised

	for (i=0;i<5;i++)
	{
		s[i].id=0;
		strcpy(s[i].hostname,"--");
		strcpy(s[i].ipaddr,"--");
		s[i].portno=0;
		s[i].sockfd=0;
	}

	////value for statistics initialised

	for (i=0;i<30;i++)
	{
		st[i].id=0;
		strcpy(st[i].host1,"--");
		strcpy(st[i].host2,"--");
		st[i].uploadtime=0;
		st[i].uploadnum=0;
		st[i].downloadtime=0;
		st[i].downloadnum=0;

	}


	for(p = info; p != NULL; p = p->ai_next)
	{



		int test;

     	if ((mastersocket = socket(p->ai_family, p->ai_socktype, IPPROTO_TCP)) == -1)
		{
    		perror("socket failed");
        	exit(EXIT_FAILURE);
		}
        else
        {
        	printf("Client:new socket.\n");
        }


   		setsockopt(mastersocket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));



		if ((test=bind(mastersocket, p->ai_addr, p->ai_addrlen)) == -1)
		{
     		perror("bind failed");
        	exit(EXIT_FAILURE);
     	}
		else
		{
			printf("Client:bind done\n");
		}


     	if ((test=listen(mastersocket, 5)) == -1)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Client:listening...");
		}

		while (1)
		{


	    	FD_ZERO(&readfds);

	    	FD_SET(mastersocket, &readfds);
	    	maxsd=mastersocket;

	    	FD_SET(0, &readfds);
			repeat=0;
			full=0;


        	for ( i = 0 ; i < maxclients ; i++)
        	{

            	sd = clientfd[i];



            	if(sd > 0)
            		FD_SET( sd , &readfds);

           		if(sd > maxsd)
           		     maxsd = sd;
        	}


			for (i=0;i<10;i++)
			{
				input[i]="-";
			}

			if (select(maxsd+1, &readfds, 0, 0, 0) < 0)
			{
	     	 	printf("select error");
	    	}


    		if (FD_ISSET(0, &readfds))
			{
     			printf("\nReading from user command\n\n");

				gets(input);


				i = 0;
  				pt = strtok (input," ");

				while (pt != NULL)
  				{
    				userinput[i++] = pt;
  					pt = strtok (NULL, " ");
  				}
				printf("values of user input are %s %s %s",userinput[0],userinput[1],userinput[2]);

				//CREATOR
				if (strcmp(userinput[0],"creator")==0)
				{
					printf("\nCREATOR\n");
					printf("\n\nUBIT name : rshetty \n UBemail: rshetty@buffalo.edu\n");
					printf("\n I have read and understood the course academic integrity policy at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
					printf("\n\n");;

				}


				//HELP
				else if (strcmp(userinput[0],"help")==0)
				{
					printf("\nHELP\n");
					printf("\n welcome to the help menu,all the commands that can be used are given below\n");
					printf("\n creator: information of the creator can be found here\n");
					printf("\n myip: ipaddress of the process\n");
					printf("\n myport : used to display the port number on which the machine is listening for incoming connections\n");
					printf("\n register <server ip> <port> : register with server whos ip is <server ip> and port number is<port>\n");
					printf("\n connect <destination> <port> : connect to a fellow client by providing hostname/ip and port number\n");
					printf("\n list : list of all active connections is displayed\n");
					printf("\n terminate <connection id> : connection mentioned through <connection id> will be terminated\n");
					printf("\n exit : all connections will be terminated\n");
					printf("\n upload <connection_id> <file_name> : the file specified at<file_name> is uploaded to the mentioned <connection_id>\n");
					printf("\n download <connection_id1> <file_name1> <connection_id2> <file_name2> <connection_id3> <file_name3> : download files from serveres specified through <connection_id1> <connection_id2> <connection_id3>\n ");
					printf("\n statistics : servers updated statistics is displayed \n for client : display client statistics\n");
					printf("\n");


				}


				//IP

				else if (strcmp(userinput[0],"myip")==0)
				{
					myip();
				}


				//PORT

				else if (strcmp(userinput[0],"myport")==0)
				{

					int size = sizeof(struct sockaddr);
					if (getsockname(mastersocket, (struct sockaddr *)&sin, &size) == -1)
					    	perror("getsockname");
					else
						portno=ntohs(sin.sin_port);
					    	printf("Port number:%d\n",portno); //PORT NUMBER
				}


				//REGISTER

				else if (strcmp(userinput[0],"register")==0)
				{


					registerflag=1;
					clsockfd=registerClient(mastersocket,userinput);
					i=0;



					for (j=0;j<5;j++)
					{
						s[j].id=atoi(passVal[i++]);
						strcpy(s[j].hostname,passVal[i++]);
						strcpy(s[j].ipaddr, passVal[i++]);
						s[j].portno=atoi(passVal[i++]);
						if (passVal[i]==NULL)
						break;
					}

					//Set details in client list when client connects
					c[0].id=1;
					strcpy(c[0].hostname,s[0].hostname);
					strcpy(c[0].ipaddr,s[0].ipaddr);
					c[0].portno=s[0].portno;

					if(clsockfd>0)
					c[0].sockfd=clsockfd;

				}


				//CONNECT


				else if (strcmp(userinput[0],"connect")==0)
				{
					n=-1;
					printf("userinput 1=%s,userinput 2=%s",userinput[1],userinput[2]);
					for (j=1;j<5;j++)

					{

						if(strcmp(userinput[1],s[j].hostname)==0)
						{
							userinput[1]=s[j].ipaddr;
							break;
						}
					}

					for (j=1;j<5;j++)
					{
						if ((strcmp(userinput[1],s[j].ipaddr)==0)&&(s[j].portno=atoi(userinput[2]))&&strcmp(ipaddr,s[j].ipaddr)!=0)
						{
							n=1;
							strcpy(hostname,s[j].hostname);
							strcpy(ipaddr,s[j].ipaddr);
							portno=s[j].portno;
							break;
						}
					}
					if (n!=1)
						printf("connect was used with incorrect arguments\n PLEASE NOTE:do not connect with server or self");

					else
					{
						for (i=0;i<maxclients;i++)
						{
							if (c[i].id==0)
							{
								n=1;
								break;
							}
						}
						if (n=1)
						{
							peerconsockfd=fellowclientconnections(userinput,s);
							printf("\n\n\nValue of peerconsockfd after return %d\n\n\n",peerconsockfd);
							if (peerconsockfd>0)
							{
								for (i=0;i<maxclients;i++)
								{

									if (c[i].id==0)
									{
										c[i].id=i+1;
										strcpy(c[i].hostname,hostname);
										strcpy(c[i].ipaddr,ipaddr);
										c[i].portno=portno;
										c[i].sockfd=peerconsockfd;
										break;
									}
								}
							}
						}
						else
							perror("Client has already registered with max number of peers");
					}

				}



				//TERMINATE

				else if (strcmp(userinput[0],"terminate")==0)
				{


					n=atoi(userinput[1]);
					j=-1;
					for (i=0;i<maxclients;i++)
					{
						if (n==c[i].id)
						{
							terminatesock=c[i].sockfd;

							j=1;
							break;
						}
					}
					if (j!=1)
						printf("please check if the client you want to terminate exists");
					else
					{
						bzero(buffer,2048);
						strcpy(buffer,"terminate");

						if( (test=send(terminatesock, buffer , strlen(buffer) , 0)) < 0)
    		  					perror("full message unable to send");
    						else
    						{

    							if ((n = recv(terminatesock, buffer, 2048,0)) < 0)
    								perror("failed to recieve");
    							else
    							{
    								for (i=0;i<maxclients;i++)
    								{
    									if (terminatesock==clientfd[i])
    										clientfd[i]=-1;
    									if (c[i].sockfd==terminatesock)
    									{
    										c[i].id=0;
    										strcpy(c[i].hostname,"--");
    										strcpy(c[i].ipaddr,"--");
    										c[i].portno=0;
    										c[i].sockfd=0;
    									}
    								}

    								close(terminatesock);

    							}
    						}
					}

				}



				//EXIT



				else if (strcmp(userinput[0],"exit")==0)
				{

					j=-1;
					k=0;


					for (k=0;k<maxclients;k++)
					{
						if (c[k].sockfd!=0)
						{
							newsocket=c[k].sockfd;

							bzero(buffer,2048);
							strcpy(buffer,"terminate");

							if( (test=send(newsocket, buffer , strlen(buffer) , 0)) < 0)
	    		  					perror("full message unable to send");

	    						else
	    						{

	      						if ((test = recv(newsocket, buffer, 2048,0)) < 0)
								perror("failed to recieve");
	      						else
	      						{
	      							perror("failed to recieve");
	      							for (i=0;i<maxclients;i++)
	      							{
	      								if (newsocket==clientfd[i])
											clientfd[i]=-1;
	      								if (c[i].sockfd==newsocket)
	      								{
	      									c[i].id=0;
	      									strcpy(c[i].hostname,"--");
	      									strcpy(c[i].ipaddr,"--");
	      									c[i].portno=0;
	      									c[i].sockfd=0;
	      								}
	      							}
	      							close(newsocket);
	      						}
	    					}
						}
					}

				}



				//UPLOAD

				else if (strcmp(userinput[0],"upload")==0)
				{

					n=atoi(userinput[1]);
					j=-1;
					for (i=0;i<maxclients;i++)
					{
						if (n==c[i].id)
						{
							uploadsocket=c[i].sockfd;
							strcpy(hostname,c[i].hostname);
							j=1;
							break;
						}
					}
					if (j!=1)
						printf("invalid command please check list");
					else
					{
						bzero(buffer,2048);
						strcpy(buffer,"upload");

						if( (test=send(uploadsocket, buffer , strlen(buffer) , 0)) < 0)
						{
							perror("send failed");

     	     	   			exit(EXIT_FAILURE);
						}
    						else
						{
							bzero(buffer,2048);
							strcpy(buffer,userinput[2]);
							sleep(2);

							if( (test=send(uploadsocket, buffer , strlen(buffer) , 0)) < 0)

    		  					perror("name sending failed");


							else
							{

								sleep(2);
								bzero(location, 200);
								strcpy(location,"./");
								strcat(location,userinput[2]);

								// Open the filein local path

								fl = fopen(location,"r");
								if (!fl)
								{
    								perror("unable to open file\n");

								}
								bzero(uploadbuffer,length);
								i=0;
								gettimeofday(&start, NULL);
								while((bytes=fread(uploadbuffer, sizeof (char),length, fl))>0)
								{
									printf("\n\n no of bytes read %d",bytes);

									if(send(uploadsocket, uploadbuffer, bytes, 0) < 0)
		    						{
										perror("file send failed");
										exit(EXIT_FAILURE);
		       	 						break;
									}
									i=i+bytes;
									bzero(uploadbuffer, length);
								}

								gettimeofday(&end, NULL);

								timeS =  (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0);
								txnrate=i/timeS;
								printf("\n\n\n Tx %s -> %s, File Size: %d Bytes, Time Taken %f seconds, Tx Rate = %f bits/sec\n\n\n",myhost, hostname,i,timeS,txnrate);

								printf("\n\nOk File %s from Client was Sent!\n", userinput[2]);
								n=0;
								printf("\n hostname= %s first slot in stasts = %s\n",hostname,st[0].host2);
								// Insert into statistics table
								for (i=0;i<30;i++)
								{
									if(strcmp(st[i].host2,hostname)==0)
									{
										//insert and break
										st[i].uploadtime=((st[i].uploadtime*st[i].uploadnum)+txnrate)/(st[i].uploadnum+1);
										st[i].uploadnum=st[i].uploadnum+1;
										n=1;
										break;
									}
								}
								if (n!=1)
								{
									// Insert new entry into statistics table
									for (i=0;i<30;i++)
									{
										if(st[i].id==0)
										{
											//insert and break
											st[i].id=i+1;
											strcpy(st[i].host1,myhost);
											strcpy(st[i].host2,hostname);
											st[i].uploadtime=txnrate;
											st[i].uploadnum=1;
											n=1;
											break;
										}
									}
								}

							}
						}
					}
				}


				




				
				//STATS



				else if (strcmp(userinput[0],"statistics")==0)
				{
					printf("hostname\tTotal Uploads\tAverage Upload speed\tTotal Downloads\tAverage Down Rate\n");
					for (i=0;i<30;i++)
					{

						if(st[i].id!=0)
						{
							printf("%s \t%d \t%f \t%d \t%f\n",st[i].host2,st[i].uploadnum,st[i].uploadtime,st[i].downloadnum,st[i].downloadtime);
						}
					}
				}

				//LIST

				else if (strcmp(userinput[0],"list")==0)
				{
					printf("%-5s%-35s%-20s%-8s\n","ID","HOSTNAME","IPADDRESS","PORT");

					i=0; //Re-declaration;
					for (i=0;i<maxclients;i++)
					{
						printf("%-5d%-35s%-20s%-8d\n",c[i].id,c[i].hostname,c[i].ipaddr,c[i].portno,c[i].sockfd);
						fflush(stdout);
					}
				}


				else
				{
					printf("\n invalid command:check help for available commands\n");
				}
			}


    		if (FD_ISSET(mastersocket, &readfds))
			{

				struct sockaddr_in client;
				int lengthclient=sizeof(client);

      			if ((newsocket = accept(mastersocket,(struct sockaddr*)&client, (socklen_t*)&lengthclient)) < 0)
					perror("accept error");
      			else
      			{
      				printf("New connection accepted\n\n");

      			}
      			bzero(buffer,2048);


      			strcpy(buffer,inet_ntoa(client.sin_addr));

      			for (i = 0; i < maxclients; i++)
      			{
					if ((strcmp(c[i].ipaddr,buffer)==0))
					{
						bzero(buffer,2048);
						strcpy(buffer,"duplicate");

						if( (test=send(newsocket, buffer , strlen(buffer) , 0)) < 0)
							puts("Send failed");
    					else
						{
							close(newsocket);
							printf("duplicate connection not  added");
							repeat=1;
						}
					}
				}

				for (i = 0; i < maxclients; i++)
				{
					if (c[0].id!=0&&c[1].id!=0&&c[2].id!=0&&c[3].id!=0)
					{
						bzero(buffer,2048);
						strcpy(buffer,"full");

						if( (test=send(newsocket, buffer , strlen(buffer) , 0)) < 0)
    		  					puts("Send failed");
						else
						{
							close(newsocket);
							printf("fellow client is maxed out");
							full;
						}
					}
				}

				if(repeat!=1&&full!=1)
				{
					bzero(buffer,2048);


					if ((n = recv(newsocket, buffer, 2048,0)) < 0)

						puts("recieve failed");





					//add new socket to userinput
            		for (i = 0; i < maxclients; i++)
            		{
            			if( clientfd[i] == -1 )
            			{
            				clientfd[i] = newsocket;
            				break;
                		}
            		}

            		for (j=0;j<maxclients;j++)
            		{
            			if (c[j].id==0)
            			{
            				c[j].id=j+1;
            				hostentry=gethostbyaddr(&client.sin_addr, sizeof (client.sin_addr), AF_INET);
            				strcpy(c[j].hostname,hostentry->h_name);
            				inet_ntop(AF_INET, &client.sin_addr, c[j].ipaddr, 99);
            				c[j].portno=atoi(buffer);
            				c[j].sockfd=newsocket;
            				break;
            			}
            		}

            		bzero(buffer,2048);
            		strcpy(buffer,"sucess");

					printf("\n\n connection successful\n\n");


					if( (test=send(newsocket, buffer , strlen(buffer) , 0)) < 0)
						puts("Send failed");
					else
						printf("Send success\n");

				}

			}

			else if (registerflag==1)
			{

				for (i = 0; i < maxclients; i++)
        			{
						newsocket = clientfd[i];
						strcpy(hostname,c[i].hostname);
						if(clientfd[i]>0)
						{
							if (FD_ISSET( newsocket, &readfds))
							{

								bzero(buffer,2048);

								if ((n = recv(newsocket, buffer, 2048,0)) < 0)
									puts( "recieve failed\n");

								else if (strcmp(buffer,"terminate")==0)
								{
									printf("\n message :%s\n",buffer);



									for (i=0;i<maxclients;i++)
									{
										if (newsocket==clientfd[i])
											clientfd[i]=-1;
										if (c[i].sockfd==newsocket)
										{
											c[i].id=0;
											strcpy(c[i].hostname,"--");
											strcpy(c[i].ipaddr,"--");
											c[i].portno=0;
											c[i].sockfd=0;
										}
									}

									if( (test=send(newsocket, buffer , strlen(buffer) , 0)) < 0)
    		  							puts("send failed\n");
    								else
    									printf("socket id=%d  deleted",newsocket);
										close(newsocket);
								}

								else if(strcmp(buffer,"upload")==0)
								{
									//--------------------upload
									bzero(buffer, 2048);

									if ((n = recv(newsocket, buffer, 2048,0)) < 0)
										puts("filename send failed\n");
									else if(n>0)
									{
										bzero(filename, 200);
										strcpy(filename,"./");
										strcat(filename,buffer);
										strcat(filename,"_rakshith");
										printf("buffer value: %s",buffer);

										printf("\n\nfilename is %s",filename);

										f2 = fopen(filename, "ab+");
										if(f2 == NULL)
											printf("file %s cannot be opened\n", filename);
										else
										{

											bzero(uploadbuffer, length);
											bytes= 0;
											i=0;
											gettimeofday(&start, NULL);
											while((bytes= recv(newsocket, uploadbuffer, length, 0)) > 0)
											{

												printf("block size: %d and writesize:%d",bytes,writesize);


												if((writesize = fwrite(uploadbuffer, sizeof (char), bytes, f2)) < 0)
												{
			        								puts("failed file write\n");
			    								}
												i=i+bytes;

												if (bytes== 0 || bytes!= length)
												{
													break;
												}
												bzero(uploadbuffer, length);

											}
											gettimeofday(&end, NULL);
											timeS =  (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0);

											txnrate=i/timeS;
											printf("\n\n\n Rx %s -> %s, File Size: %d Bytes, Time Taken %f seconds, Rx Rate = %f bits/sec\n\n\n",hostname,myhost,i,timeS,txnrate);

											if(bytes< 0)
											{
												puts("recieve failed");
											}
											printf(" received from client!\n");
											fclose(f2);

										}
									}
								}

								

							else
							{


								i = 0;
								pt = strtok (buffer," ");
								while (pt != NULL)
								{
									passVal[i++] = pt;
									pt = strtok (NULL, " ");
								}
								passVal[i]='\0';
								printf("tokenizing successful");
								i=0;

								for (j=0;j<5;j++)
								{
									s[j].id=atoi(passVal[i++]);
									strcpy(s[j].hostname,passVal[i++]);
									strcpy(s[j].ipaddr, passVal[i++]);
									s[j].portno=atoi(passVal[i++]);
									if (passVal[i]==NULL)
										break;
								}
								i=0; //Re-declaration;
								printf("ID\tHOSTNAME\tIPADDRESS\tPORT\n\n");
								for (i=0;i<5;i++)
								{
									printf("\n%d\t%s\t%s\t%d\t",s[i].id,s[i].hostname,s[i].ipaddr,s[i].portno);
								}

							}
						}
					}
				}

			}
		}

	}
	freeaddrinfo(info);
}

/*---REGISTER CLIENT---*/







int registerClient(int socket_desc, char **argv)

{

	int test,i,max=0,j;
	struct sockaddr_in server;
	char mssg[1024];
	char *p;

	printf("\n\nValue of server ip and port %s %s\n\n", argv[1],argv[2]);


	struct servL s[5]; // Structure holding list values
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));


	/*fill the list with initial value of server*/


	for (i=0;i<5;i++)
	{
		s[i].id=0;
		strcpy(s[i].hostname,"--");
		strcpy(s[i].ipaddr,"--");
		s[i].portno=0;
		s[i].sockfd=0;
	}


	//Create socket


	if ((clsockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	printf("Inside register\n");

	//Connect to remote server

	test=(connect(clsockfd , (struct sockaddr *)&server , 	sizeof(server)));
	if (test < 0)
	{
    		perror("connect failed. Error");

	}

	else
	{
		printf("server connection successful\n\n");


		printf(" portnum = %d",portno);
		sprintf(mssg, "%d", portno);
		printf  ("sent port number : %s\n\n", mssg);



		if( (test=(send(clsockfd , mssg , strlen(mssg) , 0))) < 0)
		{
			printf("Send failed\n");
			return 1;
		}
    	else
		{
			printf("port number sent successfully to server\n");
		}

      	if ((test=(recv(clsockfd, mssg, 1024,0))) < 0)
		{
			printf("recv failed\n");

		}
      	else
		{
			mssg[test]='\0';
      		printf("\nMessage = %s\n\n",mssg);

			if(strcmp(mssg,"duplicate")==0)
			{
				clsockfd=-1;
			}
			else if(strcmp(mssg,"full")==0)
			{
				clsockfd=-1;
			}
			else
			{


				for (i = 0; i < maxclients; i++)
           		{

           			if( clientfd[i] == -1 )
           			{
           				clientfd[i] = clsockfd;
						printf("Register success: client socket added at %d ",i);
						break;
           			}
           		}



				i = 0;
  				p = strtok (mssg," ");
  				while (p != NULL)
  				{
  					passVal[i++] = p;
  					p = strtok (NULL, " ");
  				}
				passVal[i]='\0';
				printf("\n\ntokenizing success\n\n");

				i=0;
				//while(passVal[i]!=NULL)
				//{
					//printf("\n passVal %d = %s",i,passVal[i]);
					//i++;
				//}

				i=0;

				for (j=0;j<5;j++)
				{
					s[j].id=atoi(passVal[i++]);
					strcpy(s[j].hostname,passVal[i++]);
					strcpy(s[j].ipaddr, passVal[i++]);
					s[j].portno=atoi(passVal[i++]);
					if (passVal[i]==NULL)
					break;
				}
				i=0;
				printf("%-5s%-35s%-20s%-8s\n","ID","HOSTNAME","IPADDRESS","PORT");
				for (i=0;i<5;i++)
				{
					printf("%-5d%-35s%-20s%-8d\n",s[i].id,s[i].hostname,s[i].ipaddr,s[i].portno);
				}
			}
		}
	}
	return(clsockfd);
}





/*---CONNECT---*/





int fellowclientconnections(char **argv, struct servL s[5])

{

	printf("\n\n\n CONNECT \n\n\n");
	int test,i,max=0,j;
	struct sockaddr_in clientconnect;
	char mssg[1024];
	char *p;
	printf("argv 1= %s,argv 2=%s",argv[1],argv[2]);


	for (j=1;j<5;j++)
	{

		if(strcmp(argv[1],s[j].hostname)==0)
		{
			argv[1]=s[j].ipaddr;
			break;
		}
	}

	printf("port = %d ",atoi(argv[2]));

	clientconnect.sin_addr.s_addr = inet_addr(argv[1]);
	clientconnect.sin_family = AF_INET;
	clientconnect.sin_port = htons(atoi(argv[2]));


	//Create socket
	if ((peerconsockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//Connect to remote server

	if ((test=connect(peerconsockfd , (struct sockaddr *)&clientconnect , sizeof(clientconnect))) < 0)
	{
       	 perror("connect failed");
       	 exit(EXIT_FAILURE);
	}
	else
	{
		printf("Connection to fellow client sucessful\n");
		printf("\n");

		printf("Value of portnum %d",portno);
		sprintf(mssg, "%d", portno);
		printf  ("Port num to be sent : %s\n\n", mssg);



		if( (test=send(peerconsockfd , mssg , strlen(mssg) , 0))
 < 0)
		{
			perror("send failed");
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Send port number to server suceeded\n");
		}

		if (	(test = recv(peerconsockfd, mssg, 1024,0)) < 0)
		{
			perror("send failed");
			exit(EXIT_FAILURE);
		}

		else
		{
			mssg[test]='\0';
			printf("\nMessage = %s\n\n",mssg);

			if(strcmp(mssg,"duplicate")==0)
			peerconsockfd=-1;
			else if(strcmp(mssg,"full")==0)
			peerconsockfd=-1;
			else if(strcmp(mssg,"sucess")==0)
			{

				for (i = 0; i < maxclients; i++)
				{
           			//if position is empty
           			if( clientfd[i] == -1 )
           			{
               			clientfd[i] = peerconsockfd;
               			printf("Peer  socket fd added at %d after connect success",i);
               			break;
           			}
				}

				return(peerconsockfd);

			}

		}


	}
	return(-1);
}




int main(int argc[], char *argv[])
{


	char *var=argv[1];

	if (strcmp(var,"s")==0)
	{
		printf("\nfor server\n");
		server(*var, argv);
	}
	else if(strcmp(var,"c")==0)
	{
		printf("\nfor client\n");
		client(*var, argv);
	}
	else
	{
		printf("worng input\n");
	}
	return 0;
}
