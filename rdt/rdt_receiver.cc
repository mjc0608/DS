/*
 * FILE: rdt_receiver.cc
 * DESCRIPTION: Reliable data transfer receiver.
 * NOTE: This implementation assumes there is no packet loss, corruption, or 
 *       reordering.  You will need to enhance it to deal with all these 
 *       situations.  In this implementation, the packet format is laid out as 
 *       the following:
 *
 *       The first byte of each packet indicates the size of the payload
 *       (excluding this single-byte header)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <queue>
#include <list>
#include <pthread.h>
#include <iterator>
using namespace std;

#include "rdt_struct.h"
#include "rdt_receiver.h"
#include "rdt_packetctl.h"

//#define DEBUG_RECEIVER
#ifdef DEBUG_RECEIVER
#define DPRINTF(fmt, ...) \
    do { fprintf(stderr, "Receiver: " fmt, ## __VA_ARGS__); } while(0)
#else
#define DPRINTF(fmt, ...) \
    do { } while(0)
#endif

#define DEFAULT_TIMEOUT 0.3

static int curr_seq_id;

/* receiver initialization, called once at the very beginning */
void Receiver_Init()
{
    fprintf(stdout, "At %.2fs: receiver initializing ...\n", GetSimulationTime());
    curr_seq_id = 0;
}

/* receiver finalization, called once at the very end.
   you may find that you don't need it, in which case you can leave it blank.
   in certain cases, you might want to use this opportunity to release some 
   memory you allocated in Receiver_init(). */
void Receiver_Final()
{
    fprintf(stdout, "At %.2fs: receiver finalizing ...\n", GetSimulationTime());
}

static void send_ack_to_sender(int seq_id) {
    packet *ack = make_pkt_ack(seq_id);
    Receiver_ToLowerLayer(ack);
    delete ack;
}

/* event handler, called when a packet is passed from the lower layer at the 
   receiver */
void Receiver_FromLowerLayer(struct packet *pkt)
{
    if (!pkt_is_valid(pkt)) {
        DPRINTF("receive invalid pkt\n");
        return;
    }

    int seq_id = pkt_get_seq_id(pkt);
    if (seq_id > curr_seq_id) {
        DPRINTF("receive pkt %d, expect pkt %d, ignore\n", seq_id, curr_seq_id);
        return;
    }
    else if (seq_id < curr_seq_id) {
        DPRINTF("receive pkt %d, expect pkt %d, an ack loss might happen, resend ack\n", seq_id, curr_seq_id);
        send_ack_to_sender(seq_id);
        return;
    }

    DPRINTF("receive pkt %d, send to upper and send ack\n", curr_seq_id);
    send_ack_to_sender(curr_seq_id);

    message *msg = pkt_to_message(pkt);
    Receiver_ToUpperLayer(msg);
    delete msg;
    curr_seq_id++;

    return;
}
