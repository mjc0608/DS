/*
 * FILE: rdt_sender.cc
 * DESCRIPTION: Reliable data transfer sender.
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
#include "rdt_sender.h"
#include "rdt_packetctl.h"

#define DEBUG_SENDER
#ifdef DEBUG_SENDER
#define DPRINTF(fmt, ...) \
    do { fprintf(stderr, "Sender: " fmt, ## __VA_ARGS__); } while(0)
#else
#define DPRINTF(fmt, ...) \
    do { } while(0)
#endif

#define DEFAULT_TIMEOUT 0.3

static queue<packet*> sender_buffer;
static list<packet*> on_air_list;
static int curr_seq_id;
static int on_air_pkts;

/* sender initialization, called once at the very beginning */
void Sender_Init()
{
    fprintf(stdout, "At %.2fs: sender initializing ...\n", GetSimulationTime());
    curr_seq_id = 0;
    on_air_pkts = 0;
}

/* sender finalization, called once at the very end.
   you may find that you don't need it, in which case you can leave it blank.
   in certain cases, you might want to take this opportunity to release some 
   memory you allocated in Sender_init(). */
void Sender_Final()
{
    fprintf(stdout, "At %.2fs: sender finalizing ...\n", GetSimulationTime());
}

static void pkt_add_to_sender_buffer(packet* pkt) {
    sender_buffer.push(pkt);
}

static void pkt_remove_from_sender_buffer() {
    sender_buffer.pop();
}

static packet* pkt_lookup_sender_buffer() {
    return sender_buffer.front();
}

static long pkt_get_send_buffer_size() {
    return sender_buffer.size();
}

static bool send_one_from_sender_buffer() {
    if (pkt_get_send_buffer_size() == 0) return false;

    packet *pkt = pkt_lookup_sender_buffer();
    pkt_remove_from_sender_buffer();
    on_air_list.push_back(pkt);

    if (!Sender_isTimerSet()) {
        DPRINTF("timer is set\n");
        Sender_StartTimer(DEFAULT_TIMEOUT);
    }
    on_air_pkts++;

    DPRINTF("pkt %d is sent\n", pkt_get_seq_id(pkt));
    Sender_ToLowerLayer(pkt);

    return true;
}

static int send_fill_window() {
    int pkt_sent = 0;

    DPRINTF("send to fill window start\n");
    while (on_air_pkts < RDT_WINDOW_SIZE) {
        if (!send_one_from_sender_buffer()) break;
        pkt_sent++;
    }

    DPRINTF("send to fill window finished, %d pkgs sent\n", pkt_sent);
    return pkt_sent;
}

static int get_first_waiting_ack_seq_id() {
    packet *pkt = on_air_list.front();
    return pkt_get_seq_id(pkt);
}

static int resend_all_from_on_air_list() {
    list<packet*>::iterator it = on_air_list.begin();
    int pkt_sent = 0;

    DPRINTF("resend all from on_air_list\n");
    DPRINTF("timer is set due to resend\n");
    Sender_StartTimer(DEFAULT_TIMEOUT);
    for (; it!=on_air_list.end(); it++) {
        pkt_sent++;
        DPRINTF("resend pkt %d\n", pkt_get_seq_id(*it));
        Sender_ToLowerLayer(*it);
    }
    DPRINTF("resend finished, %d pkts sent\n", pkt_sent);
    return pkt_sent;
}

/* event handler, called when a message is passed from the upper layer at the 
   sender */
void Sender_FromUpperLayer(struct message *msg)
{
    int header_size = RDT_PKT_HEADER_SIZE;
    int maxpayload_size = RDT_PKTSIZE - header_size;

    int remain_size = msg->size;

    DPRINTF("receive msg from upper layer, msg size %d\n", msg->size);
    while (remain_size > 0) {
        packet *pkt = make_pkt_data(msg, curr_seq_id);
        curr_seq_id++;
        remain_size -= maxpayload_size;
        pkt_add_to_sender_buffer(pkt);
        DPRINTF("add pkt to buffer, seq_id is %d\n", pkt_get_seq_id(pkt));
    }

    send_fill_window();
}

/* event handler, called when a packet is passed from the lower layer at the 
   sender */
void Sender_FromLowerLayer(struct packet *pkt)
{
    if (!pkt_is_valid(pkt)) {
        DPRINTF("receive invalid pkt\n");
        return;
    } else if (!pkt_is_ack(pkt)) {
        DPRINTF("receive non-ack pkt\n");
        return;
    }

    int seq_id = pkt_get_seq_id(pkt);
    if (seq_id != get_first_waiting_ack_seq_id()) {
        DPRINTF("receive ack %d, expect ack %d, need resend\n",
                seq_id, get_first_waiting_ack_seq_id());
        resend_all_from_on_air_list();
        return;
    }
    else {
        DPRINTF("receive ack %d, remove pkt %d from on_air_list\n", seq_id, seq_id);
        packet *pkt = on_air_list.front();
        delete pkt;
        on_air_list.pop_front();
        on_air_pkts--;
        DPRINTF("reset timer due to ack %d arrive\n", seq_id);
        Sender_StartTimer(DEFAULT_TIMEOUT);
        send_fill_window();
    }
}

/* event handler, called when the timer expires */
void Sender_Timeout()
{
    DPRINTF("sender time out while waiting for ack %d\n",
                get_first_waiting_ack_seq_id());
    resend_all_from_on_air_list();
}
