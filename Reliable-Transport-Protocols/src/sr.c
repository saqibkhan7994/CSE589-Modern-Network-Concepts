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
int nextSeqNum_A;
int base_A;
int windowSize_A;
int timerRunning_A;
int nextSeqNum_B;
int base_B;
int windowSize_B;
int bufferUsed_A;
int bufferUsed_B;
int TIMEOUT = 25;

struct msg messageBuffer_A[1000];
int acked_A[1000];
struct msg messageBuffer_B[1000];
int received_B[1000] = {0};
struct pkt new_packet;
struct pkt new_ack;

struct timerQueue{
  float startTime;
  int seqnum;
  struct timerQueue *next;
};
struct timerQueue *top, *end, *itr, *prev;
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
  if(bufferUsed_A < 1000){
    strncpy(messageBuffer_A[bufferUsed_A+1].data, message.data, 20);
    bufferUsed_A++;
    if(nextSeqNum_A < base_A + windowSize_A){
      new_packet = (struct pkt){0};
      new_packet.seqnum = nextSeqNum_A;
      strncpy(new_packet.payload, messageBuffer_A[nextSeqNum_A].data, 20);
      int checksum = compute_checksum(new_packet);
      new_packet.checksum = checksum;
      tolayer3(0, new_packet);
      float currTime = get_sim_time();
      struct timerQueue *new_timer = (struct timerQueue*) malloc(sizeof(struct timerQueue));
      new_timer->startTime = currTime;
      new_timer->seqnum = nextSeqNum_A;
      new_timer->next = NULL;
      if(top == NULL){
        new_timer->next = top;
        top = new_timer;
        end = top;
      }else{
        end->next = new_timer;
        end = new_timer;
      }
      if(timerRunning_A == 0){
        starttimer(0, 30);
        timerRunning_A = 1;
      }
      nextSeqNum_A++;
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
  if(checksum == packet.checksum && (packet.acknum >= base_A) && (packet.acknum < base_A + windowSize_A)){
    if((top != NULL) && (top->seqnum == packet.acknum)){
      stoptimer(0);
      timerRunning_A = 0;
      itr = top->next;
      float timeUtilized;
      while(itr != NULL){
        timeUtilized = 0.0;
        if(acked_A[itr->seqnum] == 1){
          prev = itr;
          itr = itr->next;
          prev->next = NULL;
        }else{
          timeUtilized = get_sim_time() - itr->startTime;
          float timeLeft = TIMEOUT - timeUtilized;
          starttimer(0, timeLeft);
          timerRunning_A = 1;
          break;
        }
      }
      top->next = NULL;
      top = itr;
    }
    acked_A[packet.acknum] = 1;
  }
  if(packet.acknum == base_A){
    while(acked_A[base_A] == 1){
      base_A++;
    }
  }
  while((nextSeqNum_A < base_A + windowSize_A) && (nextSeqNum_A <= bufferUsed_A)){
    new_packet = (struct pkt){0};
    new_packet.seqnum = nextSeqNum_A;
    strncpy(new_packet.payload, messageBuffer_A[nextSeqNum_A].data, 20);
    int checksum = compute_checksum(new_packet);
    new_packet.checksum = checksum;
    tolayer3(0, new_packet);
    float currTime = get_sim_time();
    struct timerQueue *new_timer = (struct timerQueue*) malloc(sizeof(struct timerQueue));
    new_timer->startTime = currTime;
    new_timer->seqnum = nextSeqNum_A;
    new_timer->next = NULL;
    if(top == NULL){
      new_timer->next = top;
      top = new_timer;
      end = top;
    }else{
      end->next = new_timer;
      end = new_timer;
    }
    if(timerRunning_A == 0){
      starttimer(0, 30);
      timerRunning_A = 1;
    }
    nextSeqNum_A++;
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  timerRunning_A = 0;
  itr = top->next;
  float timeUtilized;
  while(itr != NULL){
    timeUtilized = 0.0;
    if(acked_A[itr->seqnum] == 1){
      prev = itr;
      itr = itr->next;
      prev->next = NULL;
    }else{
      timeUtilized = get_sim_time() - itr->startTime;
      float timeLeft = TIMEOUT - timeUtilized;
      starttimer(0, timeLeft);
      timerRunning_A = 1;
      break;
    }
  }
  new_packet = (struct pkt){0};
  new_packet.seqnum = top->seqnum;
  strncpy(new_packet.payload, messageBuffer_A[top->seqnum].data, 20);
  int checksum = compute_checksum(new_packet);
  new_packet.checksum = checksum;
  tolayer3(0, new_packet);
  struct timerQueue *new_timer = (struct timerQueue*) malloc(sizeof(struct timerQueue));
  float currTime = get_sim_time();
  new_timer->startTime = currTime;
  new_timer->seqnum = top->seqnum;
  new_timer->next = NULL;
  top->next = NULL;
  top = itr;
  if(top == NULL){
    new_timer->next = top;
    top = new_timer;
    end = top;
  }else{
    end->next = new_timer;
    end = new_timer;
  }
  if(timerRunning_A == 0){
    starttimer(0, 30);
    timerRunning_A = 1;
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  top = NULL, end = NULL, itr = NULL, prev = NULL;
  nextSeqNum_A = 0;
  base_A = 0;
  windowSize_A = getwinsize();
  bufferUsed_A = -1;
  timerRunning_A = 0;

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  int checksum = compute_checksum(packet);
  nextSeqNum_B= packet.seqnum;
  if((checksum == packet.checksum) && (nextSeqNum_B >= base_B) && (nextSeqNum_B < base_B + windowSize_B)){
    new_ack = (struct pkt){0};
    new_ack.acknum = packet.seqnum;
    int chksm = compute_checksum(new_ack);
    new_ack.checksum = chksm;
    tolayer3(1, new_ack);
    if(nextSeqNum_B < 1000){
      if(received_B[nextSeqNum_B] != 1){
        strncpy(messageBuffer_B[nextSeqNum_B].data, packet.payload, 20);
        received_B[nextSeqNum_B] = 1;
      }
    }
    if(nextSeqNum_B == base_B){
      while(received_B[base_B] == 1){
        tolayer5(1, messageBuffer_B[base_B].data);
        base_B++;
      }
    }
  }
  else if((checksum == packet.checksum) && (nextSeqNum_B >= base_B - windowSize_B) && (nextSeqNum_B < base_B)){
    new_ack = (struct pkt){0};
    new_ack.acknum = packet.seqnum;
    int chksm = compute_checksum(new_ack);
    new_ack.checksum = chksm;
    tolayer3(1, new_ack);
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  nextSeqNum_B = 0;
  base_B = 0;
  windowSize_B = getwinsize();
  bufferUsed_B = -1;
}
