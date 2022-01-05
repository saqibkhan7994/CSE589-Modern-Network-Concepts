#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/simulator.h"


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
struct msg messageBuffer[1000];
int TIMEOUT = 10;
int msgNum;
int bufferUsed;
int ACK_FLAG;
int SEQ_NUM;
int ACK_NUM;
struct pkt new_packet;
struct pkt new_ack;
/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
int compute_checksum(struct pkt new_packet){
  int seqnum, acknum;
  int checksum = 0;
  seqnum = new_packet.seqnum;
  checksum = checksum + seqnum;
  acknum = new_packet.acknum;
  checksum = checksum + acknum;
  new_packet.payload[strcspn(new_packet.payload, "\n")] = 0;
  for(int i = 0; i<20; i++)
    checksum = checksum + new_packet.payload[i];

  checksum = ~checksum;
  return checksum;
}
/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
  {
    if(ACK_FLAG == 1){
      ACK_FLAG = 0;
      new_packet = (struct pkt){0};
      strncpy(new_packet.payload, message.data, 20);
      if(SEQ_NUM == 0)
        SEQ_NUM = 1;
      else
        SEQ_NUM = 0;
      new_packet.seqnum = SEQ_NUM;
      int checksum = compute_checksum(new_packet);
      new_packet.checksum = checksum;
      tolayer3(0, new_packet);
      starttimer(0, TIMEOUT);
    }else{
      strcpy(messageBuffer[bufferUsed+1].data, message.data);
      bufferUsed++;
    }
  }

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if(packet.acknum == SEQ_NUM){
    stoptimer(0);
    int checksum = compute_checksum(packet);
    if(checksum != packet.checksum){
      tolayer3(0, new_packet);
      starttimer(0, TIMEOUT);
    }
    else{
      if(msgNum<=bufferUsed){
        new_packet = (struct pkt){0};
        strncpy(new_packet.payload, messageBuffer[msgNum].data, 20);
        msgNum++;
        if(SEQ_NUM == 0)
          SEQ_NUM = 1;
        else
          SEQ_NUM = 0;
        new_packet.seqnum = SEQ_NUM;
        int checksum = compute_checksum(new_packet);
        new_packet.checksum = checksum;
        tolayer3(0, new_packet);
        starttimer(0, TIMEOUT);
      }else{
        ACK_FLAG = 1;
      }
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  tolayer3(0, new_packet);
  starttimer(0, TIMEOUT);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  SEQ_NUM = 1;
  ACK_FLAG = 1;
  msgNum = 0;
  bufferUsed = -1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  if(packet.seqnum == ACK_NUM){
    int checksum = compute_checksum(packet);
    if(checksum == packet.checksum){
      new_ack = (struct pkt){0};
      tolayer5(1, packet.payload);
      new_ack.acknum = ACK_NUM;
      int chksm = compute_checksum(new_ack);
      new_ack.checksum = chksm;
      tolayer3(1, new_ack);
      if(ACK_NUM == 0)
        ACK_NUM = 1;
      else
        ACK_NUM = 0;
    }
  }else{
    new_ack = (struct pkt){0};
    new_ack.acknum = packet.seqnum;
    int chksm = compute_checksum(new_ack);
    new_ack.checksum = chksm;
    tolayer3(1, new_ack);
  }
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  ACK_NUM = 0;
}
