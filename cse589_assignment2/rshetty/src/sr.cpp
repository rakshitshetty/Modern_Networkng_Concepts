#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <getopt.h>
#include <ctype.h>
#include <vector>
#include <string.h>
#include <limits>
#include <algorithm>
using namespace std;

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* Statistics 
 * Do NOT change the name/declaration of these variables
 * You need to set the value of these variables appropriately within your code.
 * */
int A_application = 0;
int A_transport = 0;
int B_application = 0;
int B_transport = 0;

/* Globals 
 * Do NOT change the name/declaration of these variables
 * They are set to zero here. You will need to set them (except WINSIZE) to some proper values.
 * */
float TIMEOUT = 25.0;
int WINSIZE;         //This is supplied as cmd-line parameter; You will need to read this value but do NOT modify it; 
int SND_BUFSIZE = 0; //Sender's Buffer size
int RCV_BUFSIZE = 0; //Receiver's Buffer size











std::vector<pkt> holdingvector;
std::vector<float> timevector;
std::vector<int> receivedAck;

std::vector<pkt> recievingvector;
std::vector<int> onezerovector;



void tolayer3(int AorB,struct pkt packet);
void starttimer(int AorB,float increment);
void tolayer5(int AorB,char *datasent);
void stoptimer(int AorB);


int packet_lost=0;
int seqnum=-1;
int acknum=-1;

struct pkt sendingpacket;
struct pkt bsendingpacket;
 struct pkt binputpacket;
int expecting=0;
int expectingack=0;
int flag=0;
int maxseqno=10000;
int previousack=-1;
float time_local = 0;
int lastsent_index=0;


/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message) //ram's comment - students can change the return type of the function from struct to pointers if necessary
{
	 printf("time local =%f",time_local);
        A_application++;
        printf("\n\n\t\t\tinside A_output\n");
        printf("\nmessage = %s\n",message.data);



                flag=1;
                struct pkt packet;
                packet.seqnum=(seqnum+1)%maxseqno;
                packet.acknum=(acknum+1)%maxseqno;
                packet.checksum=0;

                seqnum++;
                acknum++;

                printf("packet.seqno=%d\n",packet.seqnum);
                printf("packet.ackno=%d\n",packet.acknum);




                memcpy(packet.payload,message.data,sizeof(message.data));

                printf("\nmessage length=%lu\n",sizeof(message.data));

                int i;
                int n=sizeof(message.data);
                int b=0;

                for(i=0;i<n;i++)
                {

                        char a='a';
                         b += (int) message.data[i];
                        //printf("\nmessage.data in b where we are adding all leters in the message=%d\n",b);


                }

                packet.checksum=b+packet.seqnum+packet.acknum;
                printf("\nchecksum = %d\n",packet.checksum);




                sendingpacket.seqnum=packet.seqnum;
                 sendingpacket.acknum=packet.acknum;
                 sendingpacket.checksum=packet.checksum;
                 memcpy(sendingpacket.payload,packet.payload,sizeof(packet.payload));

		printf("before making it infinity %lu %d\n",timevector.size(),lastsent_index);
                timevector.push_back(std::numeric_limits<float>::max());
                printf("here\n");
		holdingvector.push_back(sendingpacket);
		receivedAck.push_back(0);
		printf("is it here\n");
		cout.flush();

                if(holdingvector.size()<=WINSIZE)
                {
                	A_transport++;

			printf("or here\n");
                if(holdingvector.size()==1)
                {
			printf("starting timer,timeout=%f\n",TIMEOUT);
                        starttimer(0,TIMEOUT);
                }

                tolayer3(0,packet);
		timevector[lastsent_index]=time_local+TIMEOUT;
		lastsent_index++;


                for(unsigned int i=0;i<holdingvector.size();i++)
                {


                        printf("\n\t\t\t\t\tinside holding vector\n\n");
                        printf("\t\t\t\t\tpacket =%d\t",i+1);
                        printf("\t\t\t\t\tpacket ackno= %d\t",holdingvector[i].acknum);
                        printf("\t\t\t\t\tpacket seqno= %d\t",holdingvector[i].acknum);
                        printf("\t\t\t\t\tpacket checksum= %d\t",holdingvector[i].checksum);
                        printf("\t\t\t\t\tpacket payload= %s\n",holdingvector[i].payload);
                }


                for(unsigned int j=0;j<timevector.size();j++)
                {


                        printf("\n\t\t\t\t\tinside time vector\n");
                        printf("\t\t\t\t\tpacket=%d\t",j+1);
                        printf("\t\t\t\t\ttime_local= %f\n",timevector[j]);
                }



        }
        else
        {
                 printf("goes into else\n");
        }




}

void B_output(struct msg message)  /* need be completed only for extra credit */
// ram's comment - students can change the return type of this function from struct to pointers if needed  
{

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
        printf("\n\n\t\t\tinside A_input\n");




        printf("\npacket payload =%s\n",packet.payload);






        int i;
        int n=sizeof(packet.payload);
        int b=0;

        for(i=0;i<n;i++)
        {

                char a='a';
                 b += (int) packet.payload[i];
                //printf("message.data in b where we are adding all leters in the message=%d\n",b);


        }

        int achecksum=b+packet.seqnum+packet.acknum;
        printf("achecksum=%d\n",achecksum);
        printf("packet.checksum =%d\n",packet.checksum);


        printf("expecting ack =%d\n",expectingack);
        printf("packet.acknum =%d\n",packet.acknum);


/*
        for(unsigned int i=0;i<holdingvector.size();i++)
        {
              printf("\n\t\t\t\t\tinside holding vector\n\n");
              printf("\t\t\t\t\tpacket =%d\t",i+1);
              printf("\t\t\t\t\tpacket ackno= %d\t",holdingvector[i].acknum);
              printf("\t\t\t\t\tpacket seqno= %d\t",holdingvector[i].acknum);
              printf("\t\t\t\t\tpacket checksum= %d\t",holdingvector[i].checksum);
              printf("\t\t\t\t\tpacket payload= %s\n",holdingvector[i].payload);

        }
        for(unsigned int j=0;j<timevector.size();j++)
        {


               printf("\n\t\t\t\t\tinside time vector\n\n");
               printf("\t\t\t\t\tpacket=%d\n",j+1);
               printf("\t\t\t\t\ttime_local= %f\n",timevector[j]);
        }
*/


	if(achecksum==packet.checksum)
	{
		printf("same checksum");
		if(expectingack==packet.acknum)
		{
                        //if(packet.payload[0]=='q')
			//	exit(0);
			printf("\t\t\t\t\tremoving from holding vector\n");

                	holdingvector.erase(holdingvector.begin());
			receivedAck.erase(receivedAck.begin());
/*
                        for(unsigned int i=0;i<holdingvector.size();i++)
                        {

                             printf("\n\t\t\t\t\tinside holding vector\n\n");
                             printf("\t\t\t\t\tpacket =%d\n",i+1);
                             printf("\t\t\t\t\tpacket ackno= %d\n",holdingvector[i].acknum);
                             printf("\t\t\t\t\tpacket seqno= %d\n",holdingvector[i].acknum);
                             printf("\t\t\t\t\tpacket checksum= %d\n",holdingvector[i].checksum);
                             printf("\t\t\t\t\tpacket payload= %s\n\n",holdingvector[i].payload);

                        }
                        //if(holdingvector.empty())
                        //{
                        //        printf("stop timer\n");
                        //         stoptimer(0);
//
                       // }
*/


                        printf("\t\t\t\t\tremoving from time vector\n");

                        timevector.erase(timevector.begin());
/*


                        for(unsigned int j=0;j<timevector.size();j++)
                        {


                             printf("\n\t\t\t\t\tinside time vector\n\n");
                             printf("\t\t\t\t\tpacket=%d\n",j+1);
                             printf("\t\t\t\t\ttime_local= %f\n",timevector[j]);
                        }
			
*/

                       // expectingack=(expectingack+1)%maxseqno;
			
			int x=0;
			while(timevector.size()!=0)
			{
				if(receivedAck[0]==1)
				{
					//cout<<"time vector "<<timevector[x]<<endl;
					holdingvector.erase(holdingvector.begin());
receivedAck.erase(receivedAck.begin());
					timevector.erase(timevector.begin());
					x++;
				}
				else
				{
					break;
				}
			}
			 expectingack=(expectingack+1+x)%maxseqno;

			printf("have to check\n");
			lastsent_index=lastsent_index-x-1;
			int lastlimit=lastsent_index+x+1;
			for(;lastsent_index<lastlimit && lastsent_index<timevector.size();lastsent_index++)
			{
				A_transport++;
				tolayer3(0,holdingvector[lastsent_index]);
				timevector[lastsent_index]=time_local+TIMEOUT;
				
			}

			



			int z=std::min_element(timevector.begin(), timevector.end()) - timevector.begin();

			printf("time vector size=%lu\n",timevector.size());			
			if(timevector.empty())
			{
				stoptimer(0);
			}
			else
			{
				stoptimer(0);
				printf("start timer,timeout=%f",timevector[z]);
				starttimer(0,timevector[z]-time_local);
			}
			
		}
		else if(packet.acknum>expectingack)
		{
			int diffack=packet.acknum-expectingack;
			//diffack=diffack+1;
			timevector[diffack]=std::numeric_limits<float>::max();
			receivedAck[diffack]=1;
			
		}	
	}
	else
	{
		printf("different checksum\n");
	}


















  /*      if(achecksum==packet.checksum)
        {
                if(packet.acknum==previousack )
                {
                        printf("duplicate ack\n");
                }
                else
                {
                        printf("else\n");
                        previousack=packet.acknum;
                        printf("expected ack=%d\n",expectingack);
                        int count=(maxseqno+packet.acknum-expectingack)%maxseqno;
                        count=count+1;
                        printf("count=%d\n",count);
                        for(int z=0;z<count;z++)
                        {
                                printf("\t\t\t\t\tremoving from holding vector\n");

                                holdingvector.erase(holdingvector.begin());


                                for(unsigned int i=0;i<holdingvector.size();i++)
                                {

                                        printf("\n\t\t\t\t\tinside holding vector\n\n");
                                        printf("\t\t\t\t\tpacket =%d\n",i+1);
                                        printf("\t\t\t\t\tpacket ackno= %d\n",holdingvector[i].acknum);
                                        printf("\t\t\t\t\tpacket seqno= %d\n",holdingvector[i].acknum);
                                        printf("\t\t\t\t\tpacket checksum= %d\n",holdingvector[i].checksum);
                                        printf("\t\t\t\t\tpacket payload= %s\n\n",holdingvector[i].payload);

                                }
                                printf("\t\t\t\t\tremoving from time vector\n");

                                timevector.erase(timevector.begin());

                                for(unsigned int j=0;j<timevector.size();j++)
                                {


                                        printf("\n\t\t\t\t\tinside time vector\n\n");
                                        printf("\t\t\t\t\tpacket=%d\n",j+1);
                                        printf("\t\t\t\t\ttime_local= %f\n",timevector[j]);
                                }


                                expectingack=(expectingack+1)%maxseqno;

                        }


                        if(holdingvector.empty())
                        {
                                printf("stop timer\n");
                                 stoptimer(0);

                        }
                        else
                        {
                                stoptimer(0);
                                starttimer(0,timevector.front()-time_local);

                        }

                }
        }
        else
        {
                printf("corrupted packet \n");
        }*/

}

int findMinValueVector(){
	int min1 = -1;
	float min_value_inside=std::numeric_limits<float>::max();
        for(int i=0;i<timevector.size();i++){
                if(timevector[i]<min_value_inside){
                        min1=i;
                        min_value_inside = timevector[i];
                }
        }

	return min1;
}

/* called when A's timer goes off */
void A_timerinterrupt() //ram's comment - changed the return type to void.
{
	 printf("\n\n\t\tinside A_timerinterrupt\n\n");



        printf("sending message again\n");

        A_transport++;	

        //starttimer(0,TIMEOUT);

	//int min=std::min_element(timevector.begin(), timevector.end()) - timevector.begin();
	int min1=std::min_element(timevector.begin(), timevector.end()) - timevector.begin();
	min1 = findMinValueVector();
	for(unsigned int f=0;f<0/*timevector.size()*/;f++)
	{
		printf("subtracting min from all in time vector\n");
		//timevector[f]=timevector[f]-timevector[min1];
		 for(unsigned int j=0;j<timevector.size();j++)
                {


                        printf("\n\t\t\t\t\tinside time vector\n\n");
                        printf("\t\t\t\t\tpacket=%d\t",j+1);
                        printf("\t\t\t\t\ttime_local= %f\n",timevector[j]);
                }

	
	
	}


	tolayer3(0,holdingvector[min1]);	
	printf("timevectot[min]=%f   %f\n",timevector[min1],time_local);	
	timevector[min1]=time_local+TIMEOUT;
	printf("timevectot[min]=%f   %f\n",timevector[min1],time_local);
        int min2=std::min_element(timevector.begin(), timevector.end()) - timevector.begin();
	min2 = findMinValueVector();
	printf("min1=%d and min2=%d\n",min1,min2);

	printf("starting timer,time out=%f timer value:%f\n",timevector[min2],timevector[min2]-time_local);
	printf("first packet %d current packet:%d next packet:%d\n",holdingvector[0].seqnum,holdingvector[min1].seqnum,holdingvector[min2].seqnum);
	starttimer(0,timevector[min2]-time_local);

/*
         for(unsigned int i=0;i<holdingvector.size();i++)
                {


                        //tolayer3(0,holdingvector[i]);


                        printf("\n\t\t\t\t\tinside holding vector\n\n");
                        printf("\t\t\t\t\tpacket =%d\t\t",i+1);
                        printf("\t\t\t\t\tpacket ackno= %d\t",holdingvector[i].acknum);
                        printf("\t\t\t\t\tpacket seqno= %d\t",holdingvector[i].seqnum);
                        printf("\t\t\t\t\tpacket checksum= %d\t",holdingvector[i].checksum);
                        printf("\t\t\t\t\tpacket payload= %s\n",holdingvector[i].payload);
                }
           for(unsigned int j=0;j<timevector.size();j++)
                {


                        printf("\n\t\t\t\t\tinside time vector\n\n");
                        printf("\t\t\t\t\tpacket=%d\t",j+1);
                        printf("\t\t\t\t\ttime_local= %f\t",timevector[j]);
                }
*/


}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init() //ram's comment - changed the return type to void.
{
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
         B_transport++;
        printf("\n\n\t\t\tinside B_input\n");
        printf("\n%s\n",packet.payload);


         printf("packet.seqno=%d\n",packet.seqnum);
        printf("packet.ackno=%d\n\n",packet.acknum);
	printf("expecting ack=%d\n",expecting);






        int i;
        int n=sizeof(packet.payload);
        int b=0;

        for(i=0;i<n;i++)
        {

                char a='a';
                 b += (int) packet.payload[i];
                //printf("message.data in b where we are adding all leters in the message=%d\n",b);


        }

        int bchecksum=b+packet.seqnum+packet.acknum;


        if(bchecksum==packet.checksum)
        {

                printf("\nsame checksum\n ");
                        struct msg newmessage;
                        memcpy(newmessage.data,packet.payload,sizeof(packet.payload));
                       // expecting=packet.seqnum;
                        
                       
                        


                	binputpacket.seqnum=packet.seqnum;
                	binputpacket.acknum=packet.acknum;
                	binputpacket.checksum=packet.checksum;
              		memcpy(binputpacket.payload,packet.payload,sizeof(packet.payload));
			//recievingvector.push_back(binputpacket);

			 printf("packet.seqno=%d\n",packet.seqnum);
        		printf("packet.ackno=%d\n\n",packet.acknum);
        		printf("expecting ack=%d\n",expecting);



		 if(packet.seqnum==expecting)
                {
			struct msg expected;
			if(recievingvector.size()==0)
			{
				recievingvector.push_back(binputpacket);
				onezerovector.push_back(1);
                        //memcpy(expected.data,packet.payload,sizeof(packet.payload));
                        //printf("sending to b application=%s\n",newmessage.data);
                        //B_application++;
                        //printf("---------------b application++ \n");
                        //tolayer5(1,expected.data);
			//expecting=(expecting+1)%maxseqno;
			}
			else
			{
				recievingvector[0]=binputpacket;
				onezerovector[0]=1;
			}
		
			

			//for(unsigned int i=0;i<recievingvector.size();i++)
			while(recievingvector.size()>0)
			{
				if(onezerovector[0]==0)
				break;
				memcpy(newmessage.data,holdingvector[0].payload,sizeof(holdingvector[0].payload));
				printf("sending to b application=%s\n",newmessage.data);
				B_application++;
				printf("b application++\n");
		        	tolayer5(1,newmessage.data);
				
				/*printf("\n\t\t\t\t\tinside recieving vector\n\n");
				printf("\t\t\t\t\trecieving vector size= %lu\n\n",recievingvector.size());
                       	 	printf("\t\t\t\t\tpacket =%d\t",i+1);
                        	printf("\t\t\t\t\tpacket ackno= %d\t",recievingvector[i].acknum);
                        	printf("\t\t\t\t\tpacket seqno= %d\t",recievingvector[i].seqnum);
                        	printf("\t\t\t\t\tpacket checksum= %d\t",recievingvector[i].checksum);
                        	printf("\t\t\t\t\tpacket payload= %s\t",recievingvector[i].payload);
*/


				recievingvector.erase(recievingvector.begin());
				onezerovector.erase(onezerovector.begin());

/*

				printf("\n\t\t\t\t\t after deleting from recieving vector\n");
                        	printf("\n\t\t\t\t\tinside recieving vector\n");
                        	printf("\t\t\t\t\tpacket =%d\t",i+1);
                        	printf("\t\t\t\t\tpacket ackno= %d\t",recievingvector[i].acknum);
                        	printf("\t\t\t\t\tpacket seqno= %d\t",recievingvector[i].seqnum);
                        	printf("\t\t\t\t\tpacket checksum= %d\t",recievingvector[i].checksum);
                        	printf("\t\t\t\t\tpacket payload= %s\n",recievingvector[i].payload);

				printf("\t\t\t\t\trecieving vector size= %lu\n",recievingvector.size());
*/

                        	expecting=(expecting+1)%maxseqno;
			}

                }
		else if(packet.seqnum>expecting)
		{	
			int count=packet.acknum-expecting;
			printf("count =%d\n",count);




			if(recievingvector.size()>count)
			{
				recievingvector[count]=binputpacket;
				onezerovector[count]=1;

			}
			else
			{
				struct pkt fake;
				int iteration=count-recievingvector.size();
				for(int c=0;c<iteration;c++)
				{
					recievingvector.push_back(fake);
					onezerovector.push_back(0);
					cout<<"onezero vector"<<endl;
				}
				recievingvector.push_back(binputpacket);
				onezerovector.push_back(1);
				
			}
			 //printf("pushing in vector=%s\n",packet.payload);
			 //recievingvector.push_back(binputpacket);

                         //printf("\n\t\t\t\t\tinside recieving vector\n\n");
                         //printf("\t\t\t\t\tpacket =%d\t",i+1);
                         //printf("\t\t\t\t\tpacket ackno= %d\t",recievingvector[i].acknum);
                         //printf("\t\t\t\t\tpacket seqno= %d\t",recievingvector[i].seqnum);
                         //printf("\t\t\t\t\tpacket checksum= %d\t",recievingvector[i].checksum);
                         //printf("\t\t\t\t\tpacket payload= %s\n",recievingvector[i].payload);


		}
	

		if(packet.payload[0]=='q'){
			for(int i=0;i<recievingvector.size();i++){
				cout<<recievingvector[i].payload<<endl;
			}
			//exit(0);
		}


                printf("sending ack payload =%s\n",binputpacket.payload);
                tolayer3(1,binputpacket);

        }
        else
        {
                printf("\ndifferent checksum\n");




        }






}

/* called when B's timer goes off */
void B_timerinterrupt() //ram's comment - changed the return type to void.
{
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init() //ram's comment - changed the return type to void.
{
}

int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
//float time_local = 0;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand() 
{
  double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */ 
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}  


/*****************************************************************
***************** NETWORK EMULATION CODE IS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/



/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1


struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */


void insertevent(struct event *p)
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time_local);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime); 
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q; 
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}





/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
//    //char *malloc();
   float ttime;
   int tempint;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */

   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time_local + x;
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
}





void init(int seed)                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();
  
  
   printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("Enter the number of messages to simulate: ");
   scanf("%d",&nsimmax);
   printf("Enter  packet loss probability [enter 0.0 for no loss]:");
   scanf("%f",&lossprob);
   printf("Enter packet corruption probability [0.0 for no corruption]:");
   scanf("%f",&corruptprob);
   printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
   scanf("%f",&lambda);
   printf("Enter TRACE:");
   scanf("%d",&TRACE);

   srand(seed);              /* init random number generator */
   sum = 0.0;                /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" ); 
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(0);
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   time_local=0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}






//int TRACE = 1;             /* for my debugging */
//int nsim = 0;              /* number of messages from 5 to 4 so far */ 
//int nsimmax = 0;           /* number of msgs to generate, then stop */
//float time = 0.000;
//float lossprob;            /* probability that a packet is dropped  */
//float corruptprob;         /* probability that one bit is packet is flipped */
//float lambda;              /* arrival rate of messages from layer 5 */   
//int   ntolayer3;           /* number sent into layer 3 */
//int   nlost;               /* number lost in media */
//int ncorrupt;              /* number corrupted by media*/

/**
 * Checks if the array pointed to by input holds a valid number.
 *
 * @param  input char* to the array holding the value.
 * @return TRUE or FALSE
 */
int isNumber(char *input)
{
    while (*input){
        if (!isdigit(*input))
            return 0;
        else
            input += 1;
    }

    return 1;
}

int main(int argc, char **argv)
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;
   
   int i,j;
   char c; 
  
   int opt;
   int seed;

   //Check for number of arguments
   if(argc != 5){
    fprintf(stderr, "Missing arguments\n");
  printf("Usage: %s -s SEED -w WINDOWSIZE\n", argv[0]);
    return -1;
   }

   /* 
    * Parse the arguments 
    * http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html 
    */
   while((opt = getopt(argc, argv,"s:w:")) != -1){
       switch (opt){
           case 's':   if(!isNumber(optarg)){
                           fprintf(stderr, "Invalid value for -s\n");
                           return -1;
                       }
                       seed = atoi(optarg);
                       break;

           case 'w':   if(!isNumber(optarg)){
                           fprintf(stderr, "Invalid value for -w\n");
                           return -1;
                       }
                       WINSIZE = atoi(optarg);
                       break;

           case '?':   break;

           default:    printf("Usage: %s -s SEED -w WINDOWSIZE\n", argv[0]);
                       return -1;
       }
   }
  
   init(seed);
   A_init();
   B_init();
   
   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nEVENT time: %f,",eventptr->evtime);
           printf("  type: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", timerinterrupt  ");
             else if (eventptr->evtype==1)
               printf(", fromlayer5 ");
             else
	     printf(", fromlayer3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        time_local = eventptr->evtime;        /* update time to next event time */
        if (nsim==nsimmax)
	  break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */    
            j = nsim % 26; 
            for (i=0; i<20; i++)  
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: data given to student: ");
                 for (i=0; i<20; i++) 
                  printf("%c", msg2give.data[i]);
               printf("\n");
	     }
            nsim++;
            if (eventptr->eventity == A) 
               A_output(msg2give);  
             else
               B_output(msg2give);  
            }
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)  
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* deliver packet by calling */
   	       A_input(pkt2give);            /* appropriate entity */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A) 
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
             }
        free(eventptr);
        }

terminate:
   //Do NOT change any of the following printfs
   printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time_local,nsim);

   printf("\n");
   printf("Protocol: ABT\n");
   printf("[PA2]%d packets sent from the Application Layer of Sender A[/PA2]\n", A_application);
   printf("[PA2]%d packets sent from the Transport Layer of Sender A[/PA2]\n", A_transport);
   printf("[PA2]%d packets received at the Transport layer of Receiver B[/PA2]\n", B_transport);
   printf("[PA2]%d packets received at the Application layer of Receiver B[/PA2]\n", B_application);
   printf("[PA2]Total time: %f time units[/PA2]\n", time_local);
   printf("[PA2]Throughput: %f packets/time units[/PA2]\n", B_application/time_local);
   cout<<"lost packets"<<packet_lost<<" "<<holdingvector[0].payload<<" "<<holdingvector[0].seqnum<<endl;
for(int i=0;i<recievingvector.size();i++){
if(onezerovector[i]==1){
cout<<recievingvector[i].payload<<" "<<i<<" "<<recievingvector[i].seqnum<<endl;
}
}
   return 0;
}


/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
 
/*void generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
    //char *malloc();
   float ttime;
   int tempint;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
 
   x = lambda*jimsrand()*2;  // x is uniform on [0,2*lambda] 
                             // having mean of lambda       
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + x;
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
} */




void printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB)
 //AorB;  /* A or B is trying to stop timer */
{
 struct event *q,*qold;

 if (TRACE>2)
    printf("          STOP TIMER: stopping timer at %f\n",time_local);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void starttimer(int AorB,float increment)
// AorB;  /* A or B is trying to stop timer */

{

 struct event *q;
 struct event *evptr;
 ////char *malloc();

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time_local);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)  
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }
 
/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time_local + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
} 


/************************** TOLAYER3 ***************/
void tolayer3(int AorB,struct pkt packet)
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 ////char *malloc();
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
	packet_lost++;
      if (TRACE>0)    
	printf("          TOLAYER3: packet being lost\n");
      return;
    }  

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */ 
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time_local;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) 
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();
 


 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
packet_lost++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)    
	printf("          TOLAYER3: packet being corrupted\n");
    }  

  if (TRACE>2)  
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
} 

void tolayer5(int AorB,char *datasent)
{
  
  int i;  
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)  
        printf("%c",datasent[i]);
     printf("\n");
   }
  
}
