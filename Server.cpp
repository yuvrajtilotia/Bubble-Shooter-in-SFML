#include "Server.h"
#include "Accepter.h"

Server::Server()
{

}

void Server::run()
{
    Queue<std::string> queue;
    List<std::shared_ptr<sf::TcpSocket>> sockets;
    //launch an accepter thread.
    std::thread(Accepter(queue, sockets)).detach();

    while (true)
    {
        std::string s = queue.pop(); //blocking
        //std::cout << "Main read: \"" << s << "\"\n";
        auto send = [&](std::shared_ptr<sf::TcpSocket> socket) {
            if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                std::cerr << "Error sending data to client\n";
                return;
            }
            return;
        };
        sockets.for_each(send);
    }
}