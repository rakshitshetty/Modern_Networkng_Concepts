/**
 * @rshetty_assignment3
 * @author  rakshit shetty <rshetty@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */

#include <stdio.h>

#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"




#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <stdint.h>


#define MAXBUFLEN 1000

uint16_t serverid;
char* serverip;
char* expserverip;
uint16_t serverport;

char** iparray;
uint16_t* idarray;
uint16_t* portarray;

char routingbuffer[2000];
char temp[1000];

int16_t neighbourstatus[5];
uint16_t neighbourid[5];
uint16_t neighbourport[5];
char** neighbourip;

uint16_t inf=UINT16_MAX;

uint8_t serveripmod[4];
uint8_t serveripmod1[4];
uint8_t serveripmod2[4];
uint8_t serveripmod3[4];
uint8_t serveripmod4[4];
uint8_t serveripmod5[4];


//splitting the ipaddress using "." as the delimiter and assigning each byte to a integer of fixed size one byte 

tokenizer(char* a,uint8_t c[4])
{

        int b=0;

        char* token2;

        char* copiedvalue;

        copiedvalue = malloc(strlen(a));

        strcpy(copiedvalue,a);

        token2=strtok(copiedvalue,".");

        while(token2!=NULL)
        {

                c[b]=atoi(token2);

                token2=strtok(NULL,".");

                b++;

        }

}

//finding index of a server in the routing table using port and ip address


int findingindex(int findusingport,char* findusingip)

{
        int indextoreturn=-1;

        int in;

        for(in=0;in<6;in++)
        {
            if(portarray[in]==findusingport)
            {
		if(strcmp(iparray[in],findusingip)==0)
                {
			indextoreturn=in;
		}
            }
        }
        return indextoreturn;
}


//finding index of the neighbour using id 

int findingneighbourindex(int findusingid,int nc)
{
	int indextoreturn=-1;

	int in;

	for(in=0;in<nc+1;in++)
	{
		if(neighbourid[in]==findusingid)
		{
			indextoreturn=in;

		}
	}
	return indextoreturn;
}

//calculating the next hop and minimum cost and updating the routing table

void updatematrix(uint16_t size,uint16_t routingtable[6][6],int16_t nexthop[6][3],uint16_t revert[6])
{
        int dest,neighbourindex,t;
	
	for(t=1;t<size+1;t++)
	{
		routingtable[serverid][t]=revert[t];
	}
		
	for(neighbourindex=1;neighbourindex<size+1;neighbourindex++)
	{
		
		if(neighbourstatus[neighbourindex]==1)
        
		{
			for(dest=1;dest<=size;dest++)
			{
				if(routingtable[serverid][dest]>routingtable[serverid][neighbourid[neighbourindex]]+routingtable[neighbourid[neighbourindex]][dest])

				{

					routingtable[serverid][dest]=routingtable[serverid][neighbourid[neighbourindex]]+routingtable[neighbourid[neighbourindex]][dest];
					
					routingtable[dest][serverid]=routingtable[serverid][neighbourid[neighbourindex]]+routingtable[neighbourid[neighbourindex]][dest];

					nexthop[dest][1]=routingtable[serverid][neighbourid[neighbourindex]]+routingtable[neighbourid[neighbourindex]][dest];

					nexthop[dest][2]=neighbourid[neighbourindex];
				}

       			}	
		}


	}
	nexthop[serverid][1]=0;
	nexthop[serverid][2]=serverid;



}

//the broken down ip is fixed by adding "." after every byte

char *fixip(uint8_t theip[4],char* tempr2)
{

        char tempr[100];

        char tempr1[100];

        int te,te2;

        bzero(tempr1,100);


        for(te=0;te<4;te++)
        {
            sprintf(tempr,"%d",theip[te]);

            strcat(tempr1,tempr);

            if(te!=3)
            {

                strcat(tempr1,".");

            }
        }

        tempr2 = malloc(strlen(tempr1));

        strcpy(tempr2,tempr1);

        return tempr2;
}




// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}





void listener(char filenamerecieved[100],int timeoutrecieved)
{


    int userinput=timeoutrecieved;

    char filebuffer[100];
    
    FILE *fp = fopen(("%s",filenamerecieved), "r");


    uint16_t noofvertices,noofedges;

    int i=1,j=1,m,n;

    uint16_t matrix[6][6];

    uint16_t backup[6];

    int16_t viamatrix[6][3];

    int16_t neighbourcount=0;

//initialising the tables for storing cost,nexthop and other details

    for(m=0;m<5;m++)
    {
        neighbourstatus[m]=0;
    }

	
    for(m=1;m<6;m++)
    {
	backup[m]=UINT16_MAX;
    }	


    for (m = 0 ; m < 6 ; m++ )
    {
	for (n = 0 ; n < 6 ; n++ )
	{

		if (m == n)
		{
			matrix[m][n] = 0;
		}
		else if ( m == 0 )
		{
			matrix[m][n] = n ;
		}

		else if (n==0)
		{
			matrix[m][n] = m ;
		}
		else
		{
			matrix[m][n] = UINT16_MAX;

		}


	}


    }


    for (m = 0 ; m < 6 ; m++ )
    {
        for (n = 0 ; n < 3 ; n++ )
        {

                if( n == 0 )
                {
                        viamatrix[m][n] = m;
                }
                if( n == 1 )
                {
                        viamatrix[m][n] = INT16_MAX ;
                }
                if( n == 2 )
                {
                        viamatrix[m][n] = -1;
                }



        }

    }






    while (fgets(filebuffer, sizeof(filebuffer), fp))
    {


        if(i==1)
        {
  
            noofvertices=atoi(filebuffer);

            iparray=malloc(noofvertices+100);

            idarray=malloc(noofvertices * sizeof(int));

            portarray=malloc(noofvertices * sizeof(int));


            j = 2 + noofvertices ;



        }

        if(i==2)
        {
                noofedges=atoi(filebuffer);

        }

        if (i > 2 && i <= j )
        {

            int i2=1;

            char* ip;

            uint16_t id=0,port=0;

            int size_of_ip;

            char* token=strtok(filebuffer," ");

            while(token)
            {
                if (i2 == 1)
                {
                    id =atoi(token) ;

                    idarray[i-2]=id;
                    
                }

                if (i2 == 2)
                {
                    size_of_ip = strlen(token);
                    
		    ip = malloc(size_of_ip);
                    
                    memcpy(ip,token,size_of_ip);
                    
                }

                if (i2 == 3)
                {
                    port= atoi(token) ;

                    portarray[i-2]=port;
                }

                i2++;

                token = strtok(NULL, " ");
            }

            *(iparray + (i-2))=malloc(size_of_ip);

            memcpy(*(iparray+(i-2)),ip,size_of_ip);

        }

        if ( i > j && i<=j+noofedges)
        {

            neighbourcount++;

            uint16_t row = 0,col = 0,cost=0;

            int i1 = 1;


            char* token=strtok(filebuffer," ");

            while(token)
            {
                if (i1 == 1)
                {
                    row =atoi(token) ;
                }

                if (i1 == 2)
                {
                    col =atoi(token) ;
                }

                if (i1 == 3)
                {
                    cost= atoi(token) ;
                }

                i1++;
    		token = strtok(NULL, " ");
            }
            
	    neighbourid[i-(noofvertices+2)]=col;

	    if(col>0)
	    {
	    	neighbourstatus[i-(noofvertices+2)]=1;
	    }

	    int z;

            
            matrix[row][col] = cost ;

            matrix[col][row] = cost ;

	    backup[row]=0;

	    backup[col]=cost;

	    viamatrix[col][1]=cost;

	    viamatrix[col][2]=row;

            serverid=row;


        }

        i = i +1;

    }


    fclose(fp);

    viamatrix[serverid][1]=0;

    viamatrix[serverid][2]=serverid;

    neighbourip=malloc(neighbourcount+100);

    int i5,j5;

    for(i5=0;i5<6;i5++)
    {

        if(idarray[i5]==serverid)
        {
            serverip = malloc(strlen(iparray[i5]));

            strcpy(serverip,iparray[i5]);

            serverport=portarray[i5];

        }
        for(j5=1;j5<=neighbourcount;j5++)
        {
            if(idarray[i5]==neighbourid[j5])
            {

                *(neighbourip+j5) = malloc(strlen(iparray[i5]));

                strcpy(*(neighbourip+j5),iparray[i5]);

                neighbourport[j5]=portarray[i5];

            }
        }

    }


    uint16_t padding=htons(0X0);

    char temp2[2];

    bzero(routingbuffer,2000);

    uint16_t noofupdatefields=noofvertices;

    int i7;


    tokenizer(serverip,serveripmod);

    if(noofvertices==1)
    {
        tokenizer(iparray[1],serveripmod1);

    }
    else if(noofvertices==2)
    {
        tokenizer(iparray[1],serveripmod1);

        tokenizer(iparray[2],serveripmod2);

    }
    else if(noofvertices==3)
    {
        tokenizer(iparray[1],serveripmod1);

        tokenizer(iparray[2],serveripmod2);

        tokenizer(iparray[3],serveripmod3);

    }
    else if(noofvertices==4)
    {
        tokenizer(iparray[1],serveripmod1);

        tokenizer(iparray[2],serveripmod2);

        tokenizer(iparray[3],serveripmod3);

        tokenizer(iparray[4],serveripmod4);
    }
    else if(noofvertices==5)
    {

        tokenizer(iparray[1],serveripmod1);

        tokenizer(iparray[2],serveripmod2);

        tokenizer(iparray[3],serveripmod3);

        tokenizer(iparray[4],serveripmod4);

        tokenizer(iparray[5],serveripmod5);
    }
	
	
	int myindex=findingindex(serverport,serverip);

	uint16_t htonnoofupdatefields=htons(noofupdatefields);

	memcpy(routingbuffer,&htonnoofupdatefields,2);

	memcpy(routingbuffer+2,serveripmod,4);

	uint16_t htonserverport=htons(portarray[myindex]);

	memcpy(routingbuffer+6,&htonserverport,2);

	

	int inc;
	
	for(inc=0;inc<noofupdatefields;inc++)
	{
		int skipcount=8+inc*12;

		if(inc==0)
		{
		    memcpy(routingbuffer+skipcount,serveripmod1,4);
		}
        	if(inc==1)
        	{
        	    memcpy(routingbuffer+skipcount,serveripmod2,4);
        	}
        	if(inc==2)
        	{
        	    memcpy(routingbuffer+skipcount,serveripmod3,4);
        	}
        	if(inc==3)
        	{
        	    memcpy(routingbuffer+skipcount,serveripmod4,4);
        	}
        	if(inc==4)
        	{
        	    memcpy(routingbuffer+skipcount,serveripmod5,4);
        	}
		
		uint16_t htonportarray=htons(portarray[inc+1]);

		memcpy(routingbuffer+skipcount+4,&htonportarray,2);


		memcpy(routingbuffer+skipcount+6,&padding,2);
		

		uint16_t htonidarray=htons(idarray[inc+1]);

		memcpy(routingbuffer+skipcount+8,&htonidarray,2);

		

		uint16_t htonmatrix=htons(matrix[serverid][inc+1]);

		memcpy(routingbuffer+skipcount+10,&htonmatrix,2);

	}


	int n2=0;

	for(n2=1;n2<6;n2++)
        {

	         int connection=matrix[serverid][n2];

        }



    int serverindex;

    serverindex=findingindex(serverport,serverip);



    char* fixedserveripmod;

    fixedserveripmod=fixip(serveripmod,fixedserveripmod);


    int x1;

    int timearray[neighbourcount+1];

    timearray[0]=userinput;

    for(x1=1;x1<neighbourcount+1;x1++)
    {
        timearray[x1]=INT_MAX;

    }

    int true=1;

    int sockfd;

    struct addrinfo hints, *servinfo, *p;

    int rv;

    int numbytes;

    struct sockaddr_storage their_addr;

    char buf[MAXBUFLEN];

    socklen_t addr_len;

    char s[INET6_ADDRSTRLEN];

    char command[1024];

    int packetcount=0;

    uint16_t tempserverport;

    tempserverport=serverport;


    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET; // set to AF_INET to force IPv4

    hints.ai_socktype = SOCK_DGRAM;

//    hints.ai_flags =AI_PASSIVE; // use my IP

    sprintf(temp,"%d",tempserverport);


    if ((rv = getaddrinfo(serverip, temp, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
     
    }

// loop through all the results and bind to the first we can

    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
        {
            perror("listener: socket");
     
            continue;
        }


        if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) < 0 )
    	{
        	perror("setsockopt");

        	exit(EXIT_FAILURE);
    	}



        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);

            perror("listener: bind");

            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "listener: failed to bind socket\n");

    }

    freeaddrinfo(servinfo);

    printf("listener: waiting to recvfrom...\n\n");


    fd_set read_fds;  // temp file descriptor list for select()

    int fdmax;        // maximum file descriptor number

    fdmax = sockfd; 
    struct timeval tv;


    tv.tv_sec=userinput;

    tv.tv_usec=0;





    while(1)
    {
        FD_ZERO(&read_fds);

        FD_SET(sockfd,&read_fds);

        FD_SET(0, &read_fds);

        char *input[4];

        int isTimeout;

        if ((isTimeout=select(fdmax+1, &read_fds, NULL, NULL, &tv)) == -1)
        {
            perror("select");

            exit(4);
        }
        if(isTimeout !=0)
        {
        	if (FD_ISSET(0, &read_fds))
        	{


        	    gets(command);

        	    int z=0;

        	    char *token;

        	    token = strtok(command," ");

        	    while( token != NULL )
        	    {
        	        input[z++] = token;

        	        token = strtok(NULL," ");
        	    }

        	    if (strcasecmp(input[0],"academic_integrity")==0)
        	    {
			cse4589_print_and_log("%s:SUCCESS\n",input[0]);

        	        cse4589_print_and_log("I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity\n\n");
        	    }

        	    else if (strcasecmp(input[0],"update")==0)
        	    {
			cse4589_print_and_log("%s:SUCCESS\n",input[0]);
       	        
			if(atoi(input[1])==serverid)
			{
                        	int neighbourindextoupdate;

                        	neighbourindextoupdate=findingneighbourindex(atoi(input[2]),neighbourcount);

                        	if(neighbourid[neighbourindextoupdate]>0)
				{
					if(neighbourstatus[neighbourindextoupdate]==1)
					{
	                        		if(strcasecmp(input[3],"inf")==0)
	                        		{
	                        		        //printf("SUCCESS\n");

	                        		        matrix[atoi(input[1])][atoi(input[2])]=UINT16_MAX;

	                        		        matrix[atoi(input[2])][atoi(input[1])]=UINT16_MAX;

							viamatrix[atoi(input[2])][1]=INT16_MAX;

	                        		        backup[atoi(input[2])]=UINT16_MAX;

	                        		        updatematrix(noofvertices,matrix,viamatrix,backup);
						}
	                        		
						else
						{
	                        			//printf("SUCCESS\n");
	
        	        				matrix[atoi(input[1])][atoi(input[2])]=atoi(input[3]);

                	  				matrix[atoi(input[2])][atoi(input[1])]=atoi(input[3]);

							viamatrix[atoi(input[2])][1]=atoi(input[3]);

							backup[atoi(input[2])]=atoi(input[3]);

							updatematrix(noofvertices,matrix,viamatrix,backup);
						}
					}
					else
					{
                                        	char* neeerror="cannot update link to the server since that server has been disabled \n";

                                        	cse4589_print_and_log("%s:%s\n", input[0],neeerror);
					}

				}
				else
				{
					if(atoi(input[2])==serverid)
					{
                                                char* selferror="server cannot update link to itself\n";
                                                cse4589_print_and_log("%s:%s\n", input[0],selferror);
						
					}
					else
                                	{
						char* neerror="cannot update link to the server since its not connected to this server \n";
                                		cse4589_print_and_log("%s:%s\n", input[0],neerror);
					} 
				}
			}
			else
			{
                        	char* updateerror="the 1st  argument wasn't the running server id \n";
                        	cse4589_print_and_log("%s:%s\n", input[0],updateerror);				
			}
        	    }

                    else if (strcasecmp(input[0],"step")==0)
                    {
                	cse4589_print_and_log("%s:SUCCESS\n",input[0]);

                	
                	int k;

                	for(k=1;k<neighbourcount+1;k++)
                	{
                	    if(neighbourstatus[k]==1)
                	    {
                	        char* timetemp=malloc(strlen(neighbourip[k]));

                	        strcpy(timetemp,neighbourip[k]);

                	        int timetempport=neighbourport[k];

                	        sendpacket(routingbuffer,timetemp,timetempport);
                	    }
                	}
			
                    }

           	    else if (strcasecmp(input[0],"packets")==0)
	            {
	                cse4589_print_and_log("%s:SUCCESS\n",input[0]);

                    	cse4589_print_and_log("%d\n",packetcount);

                    	packetcount=0;
	            }

	            else if (strcasecmp(input[0],"display")==0)
	            {
	                cse4589_print_and_log("%s:SUCCESS\n",input[0]);

			int j1;

			for(j1=1;j1<=noofvertices;j1++)
			{
				printf("%-15d%-15d%-15d\n",viamatrix[j1][0],viamatrix[j1][2],viamatrix[j1][1]);
			}
	            }

	            else if (strcasecmp(input[0],"disable")==0)
	            {
			cse4589_print_and_log("%s:SUCCESS\n",input[0]);

			int neighbourindextodisable;

			neighbourindextodisable=findingneighbourindex(atoi(input[1]),neighbourcount);
 
			if(neighbourid[neighbourindextodisable]>0)
			{
				if(neighbourstatus[neighbourindextodisable]==1)
				{
					//printf("SUCCESS\n");
	
					neighbourstatus[neighbourindextodisable]=0;

					matrix[serverid][neighbourid[neighbourindextodisable]]=UINT16_MAX;

					matrix[neighbourid[neighbourindextodisable]][serverid]=UINT16_MAX;

					viamatrix[neighbourid[neighbourindextodisable]][1]=INT16_MAX;

					viamatrix[neighbourid[neighbourindextodisable]][2]=-1;

					backup[neighbourid[neighbourindextodisable]]=UINT16_MAX;

					updatematrix(noofvertices,matrix,viamatrix,backup);
					
				}
				else
				{
					char* repeatdisable="the server you have chosen to disable has already been disabled,disable action cannot be performed \n";

                                	cse4589_print_and_log("%s:%s\n", input[0],repeatdisable);
				}
			}
			else
			{
				char* disableerror="the server you have chosen to disable isn't connected to this server,disable action cannot be performed \n";

                        	cse4589_print_and_log("%s:%s\n", input[0],disableerror);
			}
				
			
                        

	            }
	            else if (strcasecmp(input[0],"crash")==0)
	            {
	                
			while(1)
			{
			}
	            }
	            else if (strcasecmp(input[0],"dump")==0)
	            {
	                cse4589_print_and_log("%s:SUCCESS\n",input[0]);

			cse4589_dump_packet(routingbuffer,2000); 

	            }
		    else
		    {
			char* elseerror="invalid  command entered \n";

			cse4589_print_and_log("%s:%s\n", input[0],elseerror);
			
		    }

	        }
	        else
        	{
                int i;
	            for(i = 0; i <= fdmax; i++)
	            {
	                if (FD_ISSET(i, &read_fds))
	                {
	                    addr_len = sizeof their_addr;

	                    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
	                    {
	                        perror("recvfrom");
	                        exit(1);
	                    }


                        int16_t recieved_fields;

                        memcpy(&recieved_fields,buf,2);

			recieved_fields=ntohs(recieved_fields);

                        uint8_t received_ip[4];

                        memcpy(received_ip,buf+2,4);

    			char* fixedrecieved_ip;

    			fixedrecieved_ip=fixip(received_ip,fixedrecieved_ip);



                        uint16_t recieved_port;

                        memcpy(&recieved_port,buf+6,2);

			recieved_port=ntohs(recieved_port);

                        int fromwhomindex;

                        int neighbourindex;

                        fromwhomindex=findingindex(recieved_port,fixedrecieved_ip);

			cse4589_print_and_log("RECEIVED A MESSAGE FROM SERVER %d\n",idarray[fromwhomindex]); 

                        neighbourindex=findingneighbourindex(idarray[fromwhomindex],neighbourcount);

                        timearray[neighbourindex]=3*userinput;

                        int k;

                        for(k=0;k<recieved_fields;k++)
                        {

                            int byteskip=8+12*k;

                            uint16_t recieved_port1,recieved_id1,recieved_padding,recieved_cost1;

                            uint8_t received_ip1[4];

                            memcpy(received_ip1,buf+byteskip,4);
			    
	                        char* fixedrecieved_ip1;

	                        fixedrecieved_ip1=fixip(received_ip1,fixedrecieved_ip1);

                            memcpy(&recieved_port1,buf+byteskip+4,2);

			    recieved_port1=ntohs(recieved_port1);

                            memcpy(&recieved_padding,buf+byteskip+6,2);

			    recieved_padding=ntohs(recieved_padding);

                            memcpy(&recieved_id1,buf+byteskip+8,2);

			    recieved_id1=ntohs(recieved_id1);

                            memcpy(&recieved_cost1,buf+byteskip+10,2);

			    recieved_cost1=ntohs(recieved_cost1);

                            int matchingindex=findingindex(recieved_port1,fixedrecieved_ip1);


                            matrix[idarray[fromwhomindex]][recieved_id1]=recieved_cost1;
			     


                            
                        }
			int p;

			for(p=1;p<=noofvertices;p++)
			{
				cse4589_print_and_log("%-15d%-15d\n",p,matrix[idarray[fromwhomindex]][p] );
			}
 
			updatematrix(noofvertices,matrix,viamatrix,backup);

	                    buf[numbytes] = '\0';

                        packetcount++;





	                }
	            }


            }
	    }
	    else
	    {

            int x3;

            int timeoutindex=getminimum(timearray,neighbourcount);

            int minimumvalue=timearray[timeoutindex];

            int x4;

            for(x4=0;x4<neighbourcount+1;x4++)
            {
                if(timearray[x4]!=UINT16_MAX)
                {
                    timearray[x4]=(timearray[x4]-minimumvalue);

        	}
	    }



            if(timeoutindex==0)
            {
		int k;

                for(k=1;k<neighbourcount+1;k++)
                {
		    if(neighbourstatus[k]==1)
		    {
		    	char* timetemp=malloc(strlen(neighbourip[k]));

		    	strcpy(timetemp,neighbourip[k]);

		    	int timetempport=neighbourport[k];

                    	sendpacket(routingbuffer,timetemp,timetempport);
		    }
                }
                timearray[0]=userinput;

            }
            else
            {

                    timearray[timeoutindex]=INT_MAX;

                    matrix[serverid][neighbourid[timeoutindex]]=UINT16_MAX;

		    matrix[neighbourid[timeoutindex]][serverid]=UINT16_MAX;

		    backup[neighbourid[timeoutindex]]=UINT16_MAX;
		    
            }

            timeoutindex=getminimum(timearray,neighbourcount);

            tv.tv_sec=timearray[timeoutindex];

            tv.tv_usec=0;



        }

    }
}


int getminimum(int timearray[],int neighbourcount)
{
                int x2;

                int minimumindex;

                int minimum=timearray[0];

                minimumindex=0;

                for(x2=1;x2<neighbourcount+1;x2++)
                {

                    if(timearray[x2]<minimum)
                    {


                        minimum=timearray[x2];

                        minimumindex=x2;
                    }

                }
                 return minimumindex;

}








sendpacket(char routingbuffer2[2000],char* neighbouriptemp,int neighbourporttemp)
{

                   

    int sockfd;

    struct addrinfo hints, *servinfo, *p;

    int rv;

    int numbytes;

    int broadcast=1;


    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;

    hints.ai_socktype = SOCK_DGRAM;

    sprintf(temp,"%d",neighbourporttemp);

    if ((rv = getaddrinfo(neighbouriptemp,temp, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

    }


    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1)
        {
            perror("talker: socket");

            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to bind socket\n");

    }

    if( setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, &broadcast, sizeof(broadcast)) < 0 )
    {
                perror("setsockopt");

                exit(EXIT_FAILURE);
    }




    if ((numbytes = sendto(sockfd, routingbuffer2, 2000, 0,
             p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("talker: sendto");

        exit(1);
    }
    freeaddrinfo(servinfo);


    close(sockfd);



}

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */



int main(int argc, char *argv[])
{
        /*Init. Logger*/
        cse4589_init_log();

        /*Clear LOGFILE and DUMPFILE*/
        fclose(fopen(LOGFILE, "w"));

        fclose(fopen(DUMPFILE, "wb"));

        /*Start Here*/


	char filename[100];

	int timeout;

	if(argc<=4)
	{
		printf("arguments are not right please check\n");

		return 0;

	}
	if (strcmp(argv[1], "-t") == 0 && strcmp(argv[3], "-i") == 0) 
	{
		strcpy(filename, argv[2]);

		timeout = atoi(argv[4]);

		listener(filename,timeout);
		
	} 
	else if (strcmp(argv[1], "-i") == 0 && strcmp(argv[3], "-t") == 0) 
	{
		strcpy(filename, argv[4]);

		timeout = atoi(argv[2]);

		listener(filename,timeout);
	} 
	else 
	{
			printf("arguments are not right please check\n");

			return 0;
		
	}
	return 0;
}
