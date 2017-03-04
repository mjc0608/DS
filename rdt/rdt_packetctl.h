#ifndef _RDT_RELIABLE_H_
#define _RDT_RELIABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <iostream>
#include <vector>
#include <map>

#include "rdt_struct.h"
#include "rdt_sender.h"
#include "rdt_receiver.h"

#define RDT_PKT_HEADER_SIZE 8
#define RDT_PKT_DATA_SIZE 120
#define RDT_PKT_HASH_SIZE 2
#define RDT_WINDOW_SIZE 10


packet* make_pkt_data(message* mess, int seq_id);
packet* make_pkt_ack(int seq_id);
packet* make_pkt_nak(int seq_id);
bool pkt_is_valid(packet* pkt);
bool pkt_is_ack(packet* pkt);
int pkt_get_seq_id(packet* pkt);
int pkt_get_data_size(packet* pkt);
message* pkt_to_message(packet* pkt);



#endif
