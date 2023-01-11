#include "Receiver.h"
#include "util.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

Receiver::Receiver(std::shared_ptr<sf::TcpSocket> s,
    Queue<std::string>& q) :
    socket_(s),
    queue_(q)
{
}

void Receiver::recv_loop()
{
    char buffer[256];
    {
        std::stringstream ss;
        ss << socket_->getRemoteAddress() << ":" << socket_->getRemotePort() << std::endl;
        std::cout << ss.str();
    }
    while (1)
    {
        std::memset(buffer, 0, 256);
        std::size_t received;
        if (socket_->receive(buffer, 256, received) != sf::Socket::Done) {
            std::cout << "Problem receiving from client" << std::endl;
            return;
        }
        else {
            std::stringstream ss;
            //ss << "Received: \"" << buffer << "\", " << received << "bytes." << std::endl;
            //std::cout << ss.str();
        }
        queue_.push(std::string(buffer));

    }
}