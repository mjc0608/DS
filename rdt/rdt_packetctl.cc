#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "rdt_struct.h"
#include "rdt_sender.h"
#include "rdt_receiver.h"
#include "rdt_packetctl.h"

/* hashing to 16bit value */
static uint16_t hash_to_16bit(void *addr, int data_bytes) {
    uint16_t *p = (uint16_t*)addr;
    uint16_t hash_result = 0;

    for (int i=0; i<data_bytes/2; i++) {
        hash_result += p[i];
    }

    return (~hash_result)+1;
}

/* make a packet with a packet */
/* the pkt is structured as follow:
 * |<----2---->|<----1---->|<----1---->|<----4---->|<----120---->|
 * |hash_value |  ack/nak  | data_size |   seq_id  |    data     |
 * if byte 4-8 is 0x00, it's not uesd; else if it's 0x0f, it's ack;
 * else if it's 0xff, it's nak
 */
packet* make_pkt_data(message* mess, int seq_id) {
    packet *pkt = new packet;
    memset(pkt, 0, RDT_PKTSIZE);
    char *pkt_ptr = pkt->data;

    int cursor = seq_id * RDT_PKT_DATA_SIZE;
    int remain_data_size = mess->size - cursor;
    uint8_t data_size = remain_data_size > RDT_PKT_DATA_SIZE
                        ? RDT_PKT_DATA_SIZE : (uint8_t)remain_data_size;
    ASSERT(data_size > 0);

    pkt_ptr[2] = 0x00;
    pkt_ptr[3] = data_size;

    int *seid = (int*)&pkt_ptr[4];
    *seid = seq_id;

    char *dst_data_start = &pkt_ptr[8];
    char *src_data_start = &mess->data[cursor];
    memcpy(dst_data_start, src_data_start, data_size);

    uint16_t hash_value = hash_to_16bit(&pkt_ptr[2], RDT_PKTSIZE-RDT_PKT_HASH_SIZE);
    *((uint16_t*)pkt_ptr) = hash_value;

    return pkt;
}

packet* make_pkt_ack(int seq_id) {
    packet *pkt = new packet;
    memset(pkt, 0, RDT_PKTSIZE);
    char *pkt_ptr = pkt->data;

    pkt_ptr[2] = 0x0f;
    pkt_ptr[3] = 0x00;

    int *seid = (int*)&pkt_ptr[4];
    *seid = seq_id;

    uint16_t hash_value = hash_to_16bit(&pkt_ptr[2], RDT_PKTSIZE-RDT_PKT_HASH_SIZE);
    *((uint16_t*)pkt_ptr) = hash_value;

    return pkt;
}

packet* make_pkt_nak(int seq_id) {
    packet *pkt = new packet;
    memset(pkt, 0, RDT_PKTSIZE);
    char *pkt_ptr = pkt->data;

    pkt_ptr[2] = 0xff;
    pkt_ptr[3] = 0x00;

    int *seid = (int*)&pkt_ptr[4];
    *seid = seq_id;

    uint16_t hash_value = hash_to_16bit(&pkt_ptr[2], RDT_PKTSIZE-RDT_PKT_HASH_SIZE);
    *((uint16_t*)pkt_ptr) = hash_value;

    return pkt;
}

bool pkt_is_valid(packet* pkt) {
    uint16_t *pkt_ptr = (uint16_t*)pkt->data;
    int sum = 0;

    for (int i=0; i<RDT_PKTSIZE/2; i++) {
        sum += pkt_ptr[i];
    }

    if (sum==0) return true;
    else return false;
}

bool pkt_is_ack(packet* pkt) {
    uint8_t *pkt_ptr = (uint8_t*)pkt->data;
    uint8_t flag = pkt_ptr[2];

    if (flag == 0x0f) return true;
    else return false;
}

int pkt_get_seq_id(packet* pkt) {
    uint32_t *pkt_ptr = (uint32_t*)pkt->data;

    return pkt_ptr[1];
}

int pkt_get_data_size(packet* pkt) {
    uint8_t *pkt_ptr = (uint8_t*)pkt->data;

    return pkt_ptr[3];
}

static inline char* pkt_get_data_start(packet* pkt) {
    return pkt->data+8;
}

message* pkt_to_message(packet* pkt) {
    message *mess = new message;
    mess->size = pkt_get_data_size(pkt);
    mess->data = new char[mess->size];
    char* pkt_data_start = pkt_get_data_start(pkt);
    memcpy(mess->data, pkt_data_start, mess->size);

    return mess;
}



