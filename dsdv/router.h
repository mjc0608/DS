#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <cinttypes>
#include <string>
#include <vector>
#include "util.h"
using namespace std;

struct Peer {
    string name;
    uint16_t port;
    double cost;
};

struct Route {
    string router_from;
    string router_to;
    double cost;
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
    vector<Peer> peers;
    vector<Route> routes;

    char* make_boardcast_message();
    void read_file_and_set_peers();

};

#endif