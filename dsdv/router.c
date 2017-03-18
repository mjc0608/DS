#include "router.h"
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef DEBUG_ROUTER
#define DPRINTF(fmt, ...) \
    do { fprintf(stderr, "router: " fmt, ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) \
    do { } while (0)
#endif

Router::Router(uint16_t p, string f) {
    port = p;
    file = f;
    curr_seq_id = 0;
}

Router::~Router() {}

void Router::read_file_and_set_peers() {
    ifstream ifs(file.c_str());
    int size;
    string name;
    ifs >> size >> name;
    for (int i=0; i<size; i++) {
        Peer peer;
        ifs >> peer.name >> peer.cost >> peer.port;
        peers.push_back(peer);
    }
    ifs.close();
}

string Router::make_boardcast_message() {
    stringstream ss;
    ss << curr_seq_id << " " << peers.size() << " " << name << endl;
    for (int i=0; i<peers.size(); i++) {
        ss << peers[i].name << " "
           << peers[i].cost << " "
           << peers[i].port << endl;
    }
    return ss.str();
}

void Router::listen() {

}

void Router::boardcast() {
    string message = make_boardcast_message();
    int len = message.length();
    char *msg = message.c_str();

    DPRINTF("length of msg: %d\n", len);
    for (int i=0; i<peers.size(); i++) {
        send_to_port(peers[i].port, msg, len);
    }
}

void Router::send_to_port(uint16_t prt, char* msg, int len) {
    int socket_fd;
    struct sockaddr_in target_addr;

    bzero(&target_addr, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    target_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    inet_pton(AF_INET, "127.0.0.1", &target_addr.sin_addr);

    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    assert(socket_fd >= 0);

    DPRINTF("send to port %d\n", prt);

    int ret = sendto(socket_fd, msg, len, 0, target_addr, sizeof(target_addr));
    if (ret < 0) {
        DPRINTF("error sending udp\n");
    }

    return;
}