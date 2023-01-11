#include "Accepter.h"
#include "Receiver.h"
#include "util.h"
#include <iostream>
#include <sstream>
#include <thread>

Accepter::Accepter(Queue<std::string>& q, List<std::shared_ptr<sf::TcpSocket>>& s) :
    queue_(q),
    socket_(s)
{}

void Accepter::operator()()
{
    //the listener has to listen.
    sf::TcpListener listener;
    if (listener.listen(PORT) != sf::Socket::Done) {
        std::cerr << "Error listening to port\n";
        return;
    }

    std::cout << "Bound to port\n";
    while (true)
    {
        
        std::shared_ptr<sf::TcpSocket> socket = std::make_shared<sf::TcpSocket>();
        std::cout << "Made new socket" << std::endl;

        sf::Socket::Status newStatus;
        std::cout << "Made new status" << std::endl;
        newStatus = listener.accept(*socket);

        //accept a connection on socket
        std::cout << "attempting to accept connection to socket" << std::endl;
        if (newStatus != sf::Socket::Done) {
            std::cerr << "Cant accept connection\n";
            return;
        }
        else {
            std::cout << "accpeted" << std::endl;
        }

        

        socket_.push(socket);
        std::stringstream ss;
        ss << "Accepted a connection from: "
            << socket->getRemoteAddress()
            << ":"
            << socket->getRemotePort()
            << std::endl;
        std::cout << ss.str() << std::endl;
        std::shared_ptr<Receiver> receiver = std::make_shared<Receiver>(socket, queue_);

        //launch a thread to receive with the receiver
        std::thread{ &Receiver::recv_loop, receiver }.detach();
    }
}

