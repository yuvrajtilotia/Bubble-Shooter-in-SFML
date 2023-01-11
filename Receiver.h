#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include <SFML/Network.hpp>
#include "Queue.h"

class Receiver {
public:
    Receiver(std::shared_ptr<sf::TcpSocket> s, Queue<std::string>& queue);
    void recv_loop();
private:
    std::shared_ptr<sf::TcpSocket> socket_;
    Queue<std::string>& queue_;
};
#endif