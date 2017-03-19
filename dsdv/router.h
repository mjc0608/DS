#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <cinttypes>
#include <string>
#include <vector>
#include <map>
#include "util.h"
using namespace std;

#define BUFMAX 1024

struct Peer {
    string name;
    uint16_t port;
    double cost;
};

struct Link {
    double cost;
    string peer_to;
};

struct Routes {
    uint64_t seq_id;
    vector<Link> links;
};

class Router {
public:
    Router(uint16_t port, string file);
    ~Router();
    bool refresh_peers();
    void boardcast();
    void listen();
private:
    uint64_t curr_seq_id;
    uint16_t port;
    string file;
    string name;
    vector<Peer> peers;
    map<string, Routes> peer_routes;

    string make_boardcast_message();
    void read_file_and_set_peers();
    void send_to_port(uint16_t prt, char* msg, int len);
    void deal_with_comein_msg(char *msg);
};

#endif