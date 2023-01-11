#include <SFML/Network.hpp>
#include <iostream>

#include "Queue.h"
#include "List.h"
#include "util.h"

class Server {
public:
    unsigned short tcp_port;
    unsigned short udp_port;
    Server();
    void run();
};
