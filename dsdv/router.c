#include "router.h"
#include <fstream>

Router::Router(uint16_t p, string f) {
    port = p;
    file = f;
    curr_seq_id = 0;
}

Router::~Router() {}

void Router::read_file_and_set_peers() {
    ifstream ifs(file.c_str());
    
}