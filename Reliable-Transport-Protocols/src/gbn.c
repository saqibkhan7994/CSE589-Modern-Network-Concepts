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
int nextSeqNum;
int base;
int windowSize;
int expectedSequenceNum;
int bufferUsed;
int TIMEOUT = 25;

struct msg messageBuffer[1000];
struct pkt new_packet;
struct pkt new_ack;
/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
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
  if(bufferUsed < 1000){
    strcpy(messageBuffer[bufferUsed+1].data, message.data);
    bufferUsed++;

    if(nextSeqNum < base + windowSize){
      new_packet = (struct pkt){0};
      new_packet.seqnum = nextSeqNum;
      strncpy(new_packet.payload, messageBuffer[nextSeqNum].data, 20);
      int checksum = compute_checksum(new_packet);
      new_packet.checksum = checksum;
      tolayer3(0, new_packet);

      if(base == nextSeqNum){
        starttimer(0, TIMEOUT);
      }
      nextSeqNum++;
    }
  }else{
    printf("Maximum buffer used\n");
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  int checksum = compute_checksum(packet);
  if(checksum == packet.checksum){
    base = packet.acknum + 1;
    if(base == nextSeqNum){
      stoptimer(0);
    }else{
      stoptimer(0);
      starttimer(0, TIMEOUT);
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  starttimer(0, TIMEOUT);
  int i = base;
  while(i < nextSeqNum){
    new_packet = (struct pkt){0};
    new_packet.seqnum = i;
    strncpy(new_packet.payload, messageBuffer[i].data, 20);
    int checksum = compute_checksum(new_packet);
    new_packet.checksum = checksum;
    tolayer3(0, new_packet);
    i++;
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  nextSeqNum = 0;
  base = 0;
  windowSize = getwinsize();
  bufferUsed = -1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  int checksum = compute_checksum(packet);
  if((checksum == packet.checksum) && (expectedSequenceNum == packet.seqnum)){
    tolayer5(1, packet.payload);
    new_ack = (struct pkt){0};
    new_ack.acknum = expectedSequenceNum;
    int chksm = compute_checksum(new_ack);
    new_ack.checksum = chksm;
    tolayer3(1, new_ack);
    expectedSequenceNum++;
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  expectedSequenceNum = 0;
}
