#include <cmath>
#include <iostream>
#include <thread>

#include "Game.h"
#include "Queue.h"
#include "Receiver.h"
#include "Server.h"
#include "util.h"

#define M_PI 3.14159

Game::Game() {

}

void Game::run() {
    //Send broadcast message
    sf::Packet packet;
    packet << "discovered server";
    unsigned int remotePort = 55000;

    sf::Socket::Status status = m_udpSocket.send(packet, sf::IpAddress::Broadcast, remotePort);

    if (status != sf::Socket::Done) {
        std::cout << "Failed to send broadcast" << std::endl;
    }

    m_udpSocket.setBlocking(false);

    sf::Clock clock;
    while (clock.getElapsedTime().asSeconds() < 1.0f);

    //receive answer
    sf::IpAddress senderIP;
    unsigned short senderPort;

    packet.clear();
    status = m_udpSocket.receive(packet, senderIP, senderPort);

    //if no answer from the server,
    if (status != sf::Socket::Done) {

        m_seed = time(NULL);

        //make udp server thread
        std::cout << "Udp Server created" << std::endl;
        std::thread(&Game::udpServer, this).detach();

        //make tcp server thread
        std::cout << "Tcp Server Created" << std::endl;
        Server tcpServer;
        std::thread(&Server::run, tcpServer).detach();

        senderIP = sf::IpAddress::getLocalAddress();

        m_playerNumber = 1;

    }
    else {
        std::cout << "Existing Udp Server found" << std::endl;
        m_playerNumber = 2;

        //receive game seed
        packet >> m_seed;
    }

    std::cout << m_seed << std::endl;
    std::cout << "The address of the server is: " << senderIP << std::endl;

    while (clock.getElapsedTime().asSeconds() < 1.0f);

    // MAKE A CLIENT ============================================================================================================================================================
    std::cout << "Making Client" << std::endl;

    //We need to connect here
    if (socket->connect(senderIP, PORT) != sf::Socket::Done) {
        std::cerr << "Error Connecting" << std::endl;
        return;
    }

    std::cout << "Client Connected\n";
    Queue<std::string> queue;

    //launch a receiver thread to receive messages from the server.
    Receiver r{ socket, queue };
    std::thread{ &Receiver::recv_loop, &r }.detach();

    if (m_playerNumber == 2) {
        //connect player 2 to game lobby
        std::string s = "Player2";
        if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
            std::cerr << "Error sending to server\n";
            return;
        }
    }
    

//================================================================================================================================================================================

    srand(m_seed);

    sf::RenderWindow window(sf::VideoMode(m_windowWidth, m_windowHeight), "Bubble Shooter");
    window.setFramerateLimit(60);

    const sf::Color colours[5] = {
        sf::Color::Red,
        sf::Color(80, 190, 80),
        sf::Color::Blue,
        sf::Color(192, 192, 127),
        sf::Color(127, 192, 192),
    };

    generateBubbles();

    //===============================================================================================================================================================================
    //set cannon variables
    int cannonWidth = 20;
    int cannonHeight = 60;
    int maxAngle = 80;
    int minAngle = 280;
    int velocity = 7;

    sf::RectangleShape cannon1(sf::Vector2f(cannonWidth, cannonHeight));
    cannon1.setOrigin(cannonWidth / 2, cannonHeight);
    sf::Vector2f p1_pos(m_windowWidth / 4, (m_windowHeight));
    cannon1.setOrigin(cannonWidth / 2, cannonHeight);
    cannon1.setPosition(p1_pos);
    float angle1{ 0 };

    sf::RectangleShape cannon2(sf::Vector2f(cannonWidth, cannonHeight));
    cannon2.setOrigin(cannonWidth / 2, cannonHeight);
    sf::Vector2f p2_pos(3 * m_windowWidth / 4, (m_windowHeight));
    cannon2.setPosition(p2_pos);
    float angle2 = 0;

    sf::RectangleShape wall(sf::Vector2f(1, m_windowHeight));
    wall.setPosition(m_windowWidth / 2, 0);

    //laser sights
    float laserLength1 = 700;
    float laserLength2 = 700;
    float lasterWidth = 5;
    sf::RectangleShape laser1(sf::Vector2f(lasterWidth, laserLength1));
    laser1.setOrigin(lasterWidth / 2, laserLength1);
    laser1.setPosition(p1_pos);
    laser1.setFillColor(sf::Color(255, 255, 255, 100));

    sf::RectangleShape laser2(sf::Vector2f(lasterWidth, laserLength2));
    laser2.setOrigin(lasterWidth / 2, laserLength2);
    laser2.setPosition(p2_pos);
    laser2.setFillColor(sf::Color(255, 255, 255, 100));


    //===============================================================================================================================================================================

    sf::Font font;
    if (!font.loadFromFile("PressStart2P.ttf"))
    {
        return;
    }

    //===============================================================================================================================================================================
    //main update loop

    while (window.isOpen())
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) {
                window.close();
            }
        }


        if (m_mainMenuActive) {

            //receive message from server
            std::string message = "";
            queue.pop(message);

            if (message == "Player2") {
                m_player2Connected = true;
            }
            else if (message == "SpacePressed") {
                m_spacePressed = true;
            }

            //start game if space is pressed
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && m_player2Connected) {
                std::string s = "SpacePressed";
                if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                    std::cerr << "Error sending to server\n";
                    return;
                }
            }

            if (m_spacePressed) {
                m_spacePressed = false;
                m_mainMenuActive = false;
            }

            window.clear();

            displayMainMenu(window, font);

            window.display();
        }
        else {
            if (!m_player1Win && !m_player2Win) {
                //check if new bubble needs spawning on left
                if (!m_isCannon1Loaded) {
                    for (size_t i = 0; i < m_allPlayer1Bubbles.size(); i++) {
                        if (m_allPlayer1Bubbles[i].getIsAlive() == false) {
                            m_allPlayer1Bubbles[i].setPosition(p1_pos);
                            m_allPlayer1Bubbles[i].setPoints(rand() % 5);
                            m_allPlayer1Bubbles[i].setColour(colours[m_allPlayer1Bubbles[i].getPoints()]);
                            m_allPlayer1Bubbles[i].setVelocityX(0);
                            m_allPlayer1Bubbles[i].setVelocityY(0);
                            m_allPlayer1Bubbles[i].setIsAlive(true);
                            m_currentP1BubbleIndex = i;
                            m_isCannon1Loaded = true;
                            break;
                        }
                    }

                }
                //check if new bubble needs spawning on right
                if (!m_isCannon2Loaded) {
                    for (size_t i = 0; i < m_allPlayer2Bubbles.size(); i++) {
                        if (m_allPlayer2Bubbles[i].getIsAlive() == false) {
                            m_allPlayer2Bubbles[i].setPosition(p2_pos);
                            m_allPlayer2Bubbles[i].setColour(colours[m_allPlayer2Bubbles[i].getPoints()]);
                            m_allPlayer2Bubbles[i].setVelocityX(0);
                            m_allPlayer2Bubbles[i].setVelocityY(0);
                            m_allPlayer2Bubbles[i].setIsAlive(true);
                            m_currentP2BubbleIndex = i;
                            m_isCannon2Loaded = true;
                            break;
                        }
                    }

                }

                //==============================================================================================================================================================================================
                //get inputs for the game

                //angle cannons
                angle1 = cannon1.getRotation();
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && (angle1 > minAngle + 1 || angle1 < maxAngle + 1) && m_playerNumber == 1) {
                    //send messages to the server
                    std::string s = "Cannon1Left";
                    if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                        std::cerr << "Error sending to server\n";
                        return;
                    }
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && (angle1 > minAngle - 1 || angle1 < maxAngle - 1) && m_playerNumber == 1) {
                    //send messages to the server
                    std::string s = "Cannon1Right";
                    if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                        std::cerr << "Error sending to server\n";
                        return;
                    }
                }

                angle2 = cannon2.getRotation();
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && (angle2 > minAngle + 1 || angle2 < maxAngle + 1) && m_playerNumber == 2) {
                    //send messages to the server
                    std::string s = "Cannon2Left";
                    if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                        std::cerr << "Error sending to server\n";
                        return;
                    }
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && (angle2 > minAngle - 1 || angle2 < maxAngle - 1) && m_playerNumber == 2) {
                    //send messages to the server
                    std::string s = "Cannon2Right";
                    if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                        std::cerr << "Error sending to server\n";
                        return;
                    }
                }


                //Shoot cannons on both screens
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && m_isCannon1Loaded && m_playerNumber == 1)
                {
                    //send messages to the server
                    std::string s = "Cannon1Fired";
                    if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                        std::cerr << "Error sending to server\n";
                        return;
                    }

                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && m_isCannon2Loaded && m_playerNumber == 2)
                {
                    //send messages to the server
                    std::string s = "Cannon2Fired";
                    if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                        std::cerr << "Error sending to server\n";
                        return;
                    }
                }

                //==================================================================================================================================================================================
                //receive message from server
                std::string message = "";
                queue.pop(message);


                //Decode Message =======================================================
                if (message == "Cannon1Fired") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_cannon1Fired = true;
                }
                else if (message == "Cannon2Fired") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_cannon2Fired = true;
                }
                else if (message == "Cannon1Left") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_cannon1Left = true;
                }
                else if (message == "Cannon1Right") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_cannon1Right = true;
                }
                else if (message == "Cannon2Left") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_cannon2Left = true;
                }
                else if (message == "Cannon2Right") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_cannon2Right = true;
                }
                else if (message == "Laser1Activate") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_laser1Active = true;
                }
                else if (message == "Laser2Activate") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_laser2Active = true;
                }
                else if (message == "BubbleAdder1") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_bubbleAdder1 = true;
                    m_adder1PickedUp = true;
                }
                else if (message == "BubbleAdder2") {
                    //std::cout << "Player received: " << message << std::endl;
                    m_bubbleAdder2 = true;
                    m_adder2PickedUp = true;
                }


                //fire player 1 cannon
                if (m_cannon1Fired) {
                    angle1 = cannon1.getRotation();
                    m_allPlayer1Bubbles[m_currentP1BubbleIndex].setVelocityX(-cos((angle1 + 90) * M_PI / 180) * velocity);
                    m_allPlayer1Bubbles[m_currentP1BubbleIndex].setVelocityY(-sin((angle1 + 90) * M_PI / 180) * velocity);

                    m_cannon1Fired = false;
                }
                //fire player 2 cannon
                if (m_cannon2Fired) {
                    angle2 = cannon2.getRotation();
                    m_allPlayer2Bubbles[m_currentP2BubbleIndex].setVelocityX(-cos((angle2 + 90) * M_PI / 180) * velocity);
                    m_allPlayer2Bubbles[m_currentP2BubbleIndex].setVelocityY(-sin((angle2 + 90) * M_PI / 180) * velocity);

                    m_cannon2Fired = false;
                }
                //move player 1 cannon left
                if (m_cannon1Left) {
                    cannon1.rotate(-1);
                    laser1.rotate(-1);
                    m_cannon1Left = false;
                }
                //move player 1 cannon right
                if (m_cannon1Right) {
                    cannon1.rotate(1);
                    laser1.rotate(1);
                    m_cannon1Right = false;
                }
                //move player 2 cannon left
                if (m_cannon2Left) {
                    cannon2.rotate(-1);
                    laser2.rotate(-1);
                    m_cannon2Left = false;
                }
                //move player 2 cannon right
                if (m_cannon2Right) {
                    cannon2.rotate(1);
                    laser2.rotate(1);
                    m_cannon2Right = false;
                }
                //add bubbles to player 2
                if (m_bubbleAdder1) {
                    //move p2 bubbles down
                    for (auto& bubble : m_allPlayer2Bubbles) {
                        if (bubble.getIsAlive() && bubble.getPosition() != p2_pos) {
                            bubble.setPosition(sf::Vector2f(bubble.getPosition().x, bubble.getPosition().y + (m_bubbleSize * 2) - 6));
                        }
                    }

                    //spawn new bubble row at top of p2
                    for (size_t j = 0; j < (m_windowWidth / 2) / 40 - (bubbleOffset1 % 2); j++) {
                        Bubble bubble;
                        bubble.setPosition(sf::Vector2f((m_windowWidth / 2 + m_bubbleSize * 2 * j + (bubbleOffset1 % 2) * m_bubbleSize) + m_bubbleSize, 33));
                        bubble.setColour(colours[bubble.getPoints()]);
                        bubble.setIsAlive(true);
                        m_allPlayer2Bubbles.push_back(bubble);
                    }

                    bubbleOffset1++;
                    m_bubbleAdder1 = false;
                }
                //add bubbles to player 1
                if (m_bubbleAdder2) {
                    //move p1 bubbles down
                    for (auto& bubble : m_allPlayer1Bubbles) {
                        if (bubble.getIsAlive() && bubble.getPosition() != p1_pos) {
                            bubble.setPosition(sf::Vector2f(bubble.getPosition().x, bubble.getPosition().y + (m_bubbleSize * 2) - 6));
                        }
                    }

                    //spawn new bubble row at top of p1
                    for (size_t j = 0; j < (m_windowWidth / 2) / 40 - (bubbleOffset2 % 2); j++) {
                        Bubble bubble;
                        bubble.setPosition(sf::Vector2f((m_bubbleSize * 2 * j + (bubbleOffset2 % 2) * m_bubbleSize) + m_bubbleSize, 33));
                        bubble.setColour(colours[bubble.getPoints()]);
                        bubble.setIsAlive(true);
                        m_allPlayer1Bubbles.push_back(bubble);
                    }

                    bubbleOffset2++;
                    m_bubbleAdder2 = false;
                }

                //===========================================================================================================================================================================================
                //Ball moving and collision

                //if ball is moving and hits a wall, reverse x direction.
                if (m_allPlayer1Bubbles[m_currentP1BubbleIndex].getVelocity() != sf::Vector2f(0, 0))
                {
                    m_allPlayer1Bubbles[m_currentP1BubbleIndex].move();
                    if (m_allPlayer1Bubbles[m_currentP1BubbleIndex].getPosition().x < m_bubbleSize || m_allPlayer1Bubbles[m_currentP1BubbleIndex].getPosition().x > m_windowWidth / 2 - m_bubbleSize) {
                        m_allPlayer1Bubbles[m_currentP1BubbleIndex].flipXDirection();
                    }
                }

                if (m_allPlayer2Bubbles[m_currentP2BubbleIndex].getVelocity() != sf::Vector2f(0, 0))
                {
                    m_allPlayer2Bubbles[m_currentP2BubbleIndex].move();
                    if (m_allPlayer2Bubbles[m_currentP2BubbleIndex].getPosition().x < m_windowWidth / 2 + m_bubbleSize || m_allPlayer2Bubbles[m_currentP2BubbleIndex].getPosition().x > m_windowWidth - m_bubbleSize) {
                        m_allPlayer2Bubbles[m_currentP2BubbleIndex].flipXDirection();
                    }
                }

                //check bubble collisions
                checkCollisions();

                //shorten laser 1
                if (m_laser1Active) {
                    if (angle1 < 1 || angle1 > 270) {
                        laserLength1 = 700;
                        laser1.setSize(sf::Vector2f(lasterWidth, laserLength1));
                        laser1.setOrigin(lasterWidth / 2, laserLength1);
                        laser1.setPosition(p1_pos);
                    }
                    else {
                        float radAngle1 = ((90.0f - angle1) * M_PI) / 180;

                        float laserDistance1 = ((m_windowWidth / 2) - p1_pos.x) / cos(radAngle1);

                        if (laserLength1 != laserDistance1) {
                            laser1.setSize(sf::Vector2f(lasterWidth, laserDistance1));
                            laser1.setOrigin(lasterWidth / 2, laserDistance1);
                            laser1.setPosition(p1_pos);
                        }
                    }
                }
                //shorten laser 2
                if (m_laser2Active) {
                    if (angle2 >= 0 && angle2 < 270) {
                        laserLength2 = 700;
                        laser2.setSize(sf::Vector2f(lasterWidth, laserLength2));
                        laser2.setOrigin(lasterWidth / 2, laserLength2);
                        laser2.setPosition(p2_pos);
                    }
                    else {
                        float radAngle2 = ((angle2 - 270.0f) * M_PI) / 180;

                        float laserDistance2 = (p2_pos.x - (m_windowWidth / 2)) / cos(radAngle2);

                        if (laserLength2 != laserDistance2) {
                            laser2.setSize(sf::Vector2f(lasterWidth, laserDistance2));
                            laser2.setOrigin(lasterWidth / 2, laserDistance2);
                            laser2.setPosition(p2_pos);
                        }
                    }
                }

                if (m_gameTimer.getElapsedTime().asMilliseconds() >= 1000) {
                    m_currentGameTime--;
                    m_gameTimer.restart();
                }

                //========================================================================================================================================================================================
                //check win conditions

                int player1AliveBubbles = 0;
                for (auto& bubble : m_allPlayer1Bubbles) {
                    if (bubble.getIsAlive()) {
                        player1AliveBubbles++;
                    }
                }

                int player2AliveBubbles = 0;
                for (auto& bubble : m_allPlayer2Bubbles) {
                    if (bubble.getIsAlive()) {
                        player2AliveBubbles++;
                    }
                }

                if (player1AliveBubbles == 0) {
                    m_player1Win = true;
                }
                else if (player2AliveBubbles == 0) {
                    m_player2Win = true;
                }

                if (m_currentGameTime <= 0) {
                    if (m_player1Score > m_player2Score) {
                        m_player1Win = true;
                    }
                    else if (m_player2Score > m_player1Score) {
                        m_player2Win = true;
                    }
                    else {
                        m_player1Win = true;
                        m_player2Win = true;
                    }
                }

            }
            


            window.clear();

            //render

            //make laser ui
            sf::RectangleShape square1(sf::Vector2f(480, 120));
            square1.setFillColor(sf::Color::Black);
            square1.setOutlineColor(sf::Color::White);
            square1.setOutlineThickness(10);
            square1.setPosition(sf::Vector2f(60, 200));

            sf::RectangleShape square2(sf::Vector2f(480, 120));
            square2.setFillColor(sf::Color::Black);
            square2.setOutlineColor(sf::Color::White);
            square2.setOutlineThickness(10);
            square2.setPosition(sf::Vector2f(660, 200));

            sf::Text laser1Text("LASER SIGHT\n PICKED UP", font, 40);
            laser1Text.setPosition(80, 220);

            sf::Text laser2Text("LASER SIGHT\n PICKED UP", font, 40);
            laser2Text.setPosition(680, 220);

            //make adder text
            sf::Text adder1Text("BUBBLES ADDED\n TO PLAYER 2", font, 35);
            adder1Text.setPosition(75, 225);

            sf::Text adder2Text("BUBBLES ADDED\n TO PLAYER 1", font, 35);
            adder2Text.setPosition(675, 225);

            //activate laser 1
            if (m_laser1Active) {
                window.draw(laser1);
            }
            //activate laser 2
            if (m_laser2Active) {
                window.draw(laser2);
            }

            window.draw(cannon1);
            window.draw(cannon2);

            sf::Text score1("SCORE: " + std::to_string(m_player1Score), font, 20);
            score1.setPosition(10, 570);
            window.draw(score1);

            sf::Text score2("SCORE: " + std::to_string(m_player2Score), font, 20);
            score2.setPosition(m_windowWidth - 250, 570);
            window.draw(score2);

            render(window);

            //render power up pop up (laser) player 1
            if (laserClock1.getElapsedTime().asMilliseconds() < 1200 && m_laser1Active) {
                window.draw(square1);
                window.draw(laser1Text);
            }
            //render power up pop up (laser) player 2
            if (laserClock2.getElapsedTime().asMilliseconds() < 1200 && m_laser2Active) {
                window.draw(square2);
                window.draw(laser2Text);
            }
            //render power up pop up (adder) player 1
            if (adderClock1.getElapsedTime().asMilliseconds() < 1200 && m_adder1PickedUp) {
                window.draw(square1);
                window.draw(adder1Text);
            }
            //render power up pop up (adder) player 2
            if (adderClock2.getElapsedTime().asMilliseconds() < 1200 && m_adder2PickedUp) {
                window.draw(square2);
                window.draw(adder2Text);
            }

            window.draw(wall);

            //make timer box
            sf::RectangleShape timerBox(sf::Vector2f(100, 75));
            timerBox.setFillColor(sf::Color::Black);
            timerBox.setOutlineColor(sf::Color::White);
            timerBox.setOutlineThickness(2);
            timerBox.setPosition(sf::Vector2f(550, 525));
            window.draw(timerBox);

            //draw timer
            sf::Text timer(std::to_string(m_currentGameTime), font, 30);
            if (m_currentGameTime < 10) {
                timer.setPosition(sf::Vector2f(585, 550));
            }
            else {
                timer.setPosition(sf::Vector2f(570, 550));
            }
            window.draw(timer);

            //when game is won
            if (m_player1Win || m_player2Win) {
                displayWinScreen(window, font);
                //receive message from server
                std::string message = "";
                queue.pop(message);

                if (message == "EnterPressed") {
                    m_enterPressed = true;
                }

                //start game if space is pressed
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && m_player2Connected) {
                    std::string s = "EnterPressed";
                    if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                        std::cerr << "Error sending to server\n";
                        return;
                    }
                }

                if (m_enterPressed) {
                    //reset the game
                    m_enterPressed = false;

                    resetGame();
                    angle1 = 0;
                    angle2 = 0;
                    cannon1.setRotation(angle1);
                    cannon2.setRotation(angle2);
                    laser1.setRotation(angle1);
                    laser2.setRotation(angle2);

                    m_mainMenuActive = true;;
                }
            }


            window.display();

        }
    }
}


void Game::checkCollisions() {

    int xDist, yDist, currentDistance;

    //player 1 collisions
    for (int i = 0; i < m_allPlayer1Bubbles.size(); i++) {
        if (i != m_currentP1BubbleIndex) {
            xDist = m_allPlayer1Bubbles[m_currentP1BubbleIndex].getPosition().x - m_allPlayer1Bubbles[i].getPosition().x;
            yDist = m_allPlayer1Bubbles[m_currentP1BubbleIndex].getPosition().y - m_allPlayer1Bubbles[i].getPosition().y;
            currentDistance = pow(xDist, 2) + pow(yDist, 2);

            //if distance is the same as 2 of the radiuses, stop the bubble
            if (currentDistance <= (pow((2 * m_bubbleSize), 2) + 2) && m_allPlayer1Bubbles[i].getIsAlive()) {
                //make bubble collide
                m_allPlayer1Bubbles[m_currentP1BubbleIndex].setVelocityX(0);
                m_allPlayer1Bubbles[m_currentP1BubbleIndex].setVelocityY(0);

                //make bubbles snap to grid
                if (m_allPlayer1Bubbles[m_currentP1BubbleIndex].getPosition().x >= m_allPlayer1Bubbles[i].getPosition().x) {
                    m_allPlayer1Bubbles[m_currentP1BubbleIndex].setPosition(sf::Vector2f(m_allPlayer1Bubbles[i].getPosition().x + m_bubbleSize, m_allPlayer1Bubbles[i].getPosition().y + (2 * m_bubbleSize) - 6));
                }
                else if (m_allPlayer1Bubbles[m_currentP1BubbleIndex].getPosition().x < m_allPlayer1Bubbles[i].getPosition().x) {
                    m_allPlayer1Bubbles[m_currentP1BubbleIndex].setPosition(sf::Vector2f(m_allPlayer1Bubbles[i].getPosition().x - m_bubbleSize, m_allPlayer1Bubbles[i].getPosition().y + (2 * m_bubbleSize) - 6));
                }

                //check if any connected bubbles;
                checkConsecutiveCollisions(m_currentP1BubbleIndex, 1);

                //reload connon on hit
                m_isCannon1Loaded = false;
            }

        }
    }

    //player 2 collisions
    for (int i = 0; i < m_allPlayer2Bubbles.size(); i++) {
        if (i != m_currentP2BubbleIndex) {
            xDist = m_allPlayer2Bubbles[m_currentP2BubbleIndex].getPosition().x - m_allPlayer2Bubbles[i].getPosition().x;
            yDist = m_allPlayer2Bubbles[m_currentP2BubbleIndex].getPosition().y - m_allPlayer2Bubbles[i].getPosition().y;
            currentDistance = pow(xDist, 2) + pow(yDist, 2);

            //if distance is the same as 2 of the radiuses, stop the bubble
            if (currentDistance <= (pow((2 * m_bubbleSize), 2) + 2) && m_allPlayer2Bubbles[i].getIsAlive()) {
                //make bubble collide
                m_allPlayer2Bubbles[m_currentP2BubbleIndex].setVelocityX(0);
                m_allPlayer2Bubbles[m_currentP2BubbleIndex].setVelocityY(0);

                //make bubbles snap to grid
                if (m_allPlayer2Bubbles[m_currentP2BubbleIndex].getPosition().x >= m_allPlayer2Bubbles[i].getPosition().x) {
                    m_allPlayer2Bubbles[m_currentP2BubbleIndex].setPosition(sf::Vector2f(m_allPlayer2Bubbles[i].getPosition().x + m_bubbleSize, m_allPlayer2Bubbles[i].getPosition().y + (2 * m_bubbleSize) - 6));
                }
                else if (m_allPlayer2Bubbles[m_currentP2BubbleIndex].getPosition().x < m_allPlayer2Bubbles[i].getPosition().x) {
                    m_allPlayer2Bubbles[m_currentP2BubbleIndex].setPosition(sf::Vector2f(m_allPlayer2Bubbles[i].getPosition().x - m_bubbleSize, m_allPlayer2Bubbles[i].getPosition().y + (2 * m_bubbleSize) - 6));
                }

                //check if any connected bubbles;
                checkConsecutiveCollisions(m_currentP2BubbleIndex, 2);

                //reload connon on hit
                m_isCannon2Loaded = false;
            }

        }
    }

    if (m_allPlayer1Bubbles[m_currentP1BubbleIndex].getPosition().y < 0) {
        m_allPlayer1Bubbles[m_currentP1BubbleIndex].setIsAlive(false);
        m_isCannon1Loaded = false;
    }
    if (m_allPlayer2Bubbles[m_currentP2BubbleIndex].getPosition().y < 0) {
        m_allPlayer2Bubbles[m_currentP2BubbleIndex].setIsAlive(false);
        m_isCannon2Loaded = false;
    }
}


void Game::checkConsecutiveCollisions(int currentIndex, int playerNumber) {

    int xDist, yDist, currentDistance;

    if (playerNumber == 1) {
        for (int i = 0; i < m_allPlayer1Bubbles.size(); i++) {
            if (i != currentIndex) {
                xDist = m_allPlayer1Bubbles[currentIndex].getPosition().x - m_allPlayer1Bubbles[i].getPosition().x;
                yDist = m_allPlayer1Bubbles[currentIndex].getPosition().y - m_allPlayer1Bubbles[i].getPosition().y;
                currentDistance = pow(xDist, 2) + pow(yDist, 2);

                int offset = 0;

                //if distance is the same as 2 of the radiuses, stop the bubble
                if (currentDistance <= (pow((2 * m_bubbleSize), 2)) + offset && m_allPlayer1Bubbles[i].getIsAlive()) {

                    //make colliding bubbles dissappear
                    if (m_allPlayer1Bubbles[i].getPoints() == m_allPlayer1Bubbles[currentIndex].getPoints() || m_allPlayer1Bubbles[i].getColour() == sf::Color::Magenta) {
                        m_allPlayer1Bubbles[i].setIsAlive(false);
                        m_allPlayer1Bubbles[currentIndex].setIsAlive(false);

                        if (m_allPlayer1Bubbles[i].getHasLaser()) {
                            //activate laser powerup
                            laserClock1.restart();
                            if (m_playerNumber == 1) {
                                //send messages to the server
                                std::string s = "Laser1Activate";
                                if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                                    std::cerr << "Error sending to server\n";
                                    return;
                                }
                            }
                            
                        }
                        else if (m_allPlayer1Bubbles[i].getHasAdder()) {
                            //activate adder powerup
                            adderClock1.restart();
                            if (m_playerNumber == 1) {
                                //send messages to the server
                                std::string s = "BubbleAdder1";
                                if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                                    std::cerr << "Error sending to server\n";
                                    return;
                                }
                            }
                            
                        }

                        //incresase score per bubble
                        m_player1Score += 10;

                        //check if any connected bubbles;
                        checkConsecutiveCollisions(i, 1);
                    }

                }

            }
        }
    }
    else if (playerNumber == 2) {
        for (int i = 0; i < m_allPlayer1Bubbles.size(); i++) {
            if (i != currentIndex) {
                xDist = m_allPlayer2Bubbles[currentIndex].getPosition().x - m_allPlayer2Bubbles[i].getPosition().x;
                yDist = m_allPlayer2Bubbles[currentIndex].getPosition().y - m_allPlayer2Bubbles[i].getPosition().y;
                currentDistance = pow(xDist, 2) + pow(yDist, 2);

                int offset = 5;

                //if distance is the same as 2 of the radiuses, stop the bubble
                if (currentDistance <= (pow((2 * m_bubbleSize), 2)) + offset && m_allPlayer2Bubbles[i].getIsAlive()) {

                    //make colliding bubbles dissappear
                    if (m_allPlayer2Bubbles[i].getPoints() == m_allPlayer2Bubbles[currentIndex].getPoints() || m_allPlayer2Bubbles[i].getColour() == sf::Color::Magenta) {
                        m_allPlayer2Bubbles[i].setIsAlive(false);
                        m_allPlayer2Bubbles[currentIndex].setIsAlive(false);

                        //activate laser pickup
                        if (m_allPlayer2Bubbles[i].getHasLaser()) {
                            //activate laser powerup
                            laserClock2.restart();
                            if (m_playerNumber == 2) {
                                //send messages to the server
                                std::string s = "Laser2Activate";
                                if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                                    std::cerr << "Error sending to server\n";
                                    return;
                                }
                            }
                            
                        }
                        else if (m_allPlayer2Bubbles[i].getHasAdder()) {
                            //activate adder powerup
                            adderClock2.restart();
                            if (m_playerNumber == 2) {
                                //send messages to the server
                                std::string s = "BubbleAdder2";
                                if (socket->send(s.c_str(), s.size() + 1) != sf::Socket::Done) {
                                    std::cerr << "Error sending to server\n";
                                    return;
                                }
                            }
                        }
                        
                        //incresase score per bubble
                        m_player2Score += 10;

                        //check if any connected bubbles;
                        checkConsecutiveCollisions(i, 2);
                    }

                }

            }
        }
    }
}

void::Game::generateBubbles() {
    //make extra bubbles to shoot (player 1)
    for (size_t i = 0; i < 10; i++) {
        Bubble bubble;
        bubble.setColour(colours[bubble.getPoints()]);
        bubble.setIsAlive(false);
        m_allPlayer1Bubbles.push_back(bubble);
    }

    //make extra bubbles to shoot (player 2)
    for (size_t i = 0; i < 10; i++) {
        Bubble bubble;
        bubble.setColour(colours[bubble.getPoints()]);
        bubble.setIsAlive(false);
        m_allPlayer2Bubbles.push_back(bubble);
    }

    //make bubble wall at the top of the screen (for player 1)
    for (size_t i = 1; i < 11; i++) {
        for (size_t j = 0; j < (m_windowWidth / 2) / 40 - (i % 2); j++) {
            Bubble bubble;
            bubble.setPosition(sf::Vector2f((m_bubbleSize * 2 * j + (i % 2) * m_bubbleSize) + m_bubbleSize, i * 33));
            bubble.setColour(colours[bubble.getPoints()]);
            bubble.setIsAlive(true);
            m_allPlayer1Bubbles.push_back(bubble);
        }
    }

    //make bubble wall at the top of the screen (for player 2)
    for (size_t i = 1; i < 11; i++) {
        for (size_t j = 0; j < (m_windowWidth / 2) / 40 - (i % 2); j++) {
            Bubble bubble;
            bubble.setPosition(sf::Vector2f((m_windowWidth / 2 + m_bubbleSize * 2 * j + (i % 2) * m_bubbleSize) + m_bubbleSize, i * 33));
            bubble.setColour(colours[bubble.getPoints()]);
            bubble.setIsAlive(true);
            m_allPlayer2Bubbles.push_back(bubble);
        }
    }

    bubbleOffset1 = 0;
    bubbleOffset2 = 0;

    int numberOfPowerUps = 4;

    //place laser powerup for player 1
    int randomPosition = (rand() % (m_allPlayer1Bubbles.size() - 10)) + 10;
    m_allPlayer1Bubbles[randomPosition].setColour(sf::Color::Magenta);
    m_allPlayer1Bubbles[randomPosition].setHasLaser(true);

    //place adder powerups for player 1
    for (int i = 1; i < numberOfPowerUps; i++) {
        randomPosition = (rand() % (m_allPlayer1Bubbles.size() - 10)) + 10;
        while (m_allPlayer1Bubbles[randomPosition].getColour() == sf::Color::Magenta) {
            randomPosition = (rand() % (m_allPlayer1Bubbles.size() - 10)) + 10;
        }
        m_allPlayer1Bubbles[randomPosition].setColour(sf::Color::Magenta);
        m_allPlayer1Bubbles[randomPosition].setHasAdder(true);
    }

    //place laser powerup for player 2
    randomPosition = (rand() % (m_allPlayer2Bubbles.size() - 10)) + 10;
    m_allPlayer2Bubbles[randomPosition].setColour(sf::Color::Magenta);
    m_allPlayer2Bubbles[randomPosition].setHasLaser(true);

    //place adder powerups for player 2
    for (int i = 1; i < numberOfPowerUps; i++) {
        randomPosition = (rand() % (m_allPlayer2Bubbles.size() - 10)) + 10;
        while (m_allPlayer2Bubbles[randomPosition].getColour() == sf::Color::Magenta) {
            randomPosition = (rand() % (m_allPlayer2Bubbles.size() - 10)) + 10;
        }
        m_allPlayer2Bubbles[randomPosition].setColour(sf::Color::Magenta);
        m_allPlayer2Bubbles[randomPosition].setHasAdder(true);
    }
}

void::Game::resetGame() {
    m_currentGameTime = m_maxGameTime + 1;

    m_isCannon1Loaded = false;
    m_isCannon2Loaded = false;

    m_player1Win = false;
    m_player2Win = false;
    
    m_laser1Active = false;
    m_laser2Active = false;

    m_bubbleAdder1 = false;
    m_bubbleAdder2 = false;

    m_adder1PickedUp = false;
    m_adder2PickedUp = false;

    m_cannon1Fired = false;
    m_cannon2Fired = false;

    m_cannon1Left = false;
    m_cannon1Right = false;
    m_cannon2Left = false;
    m_cannon2Right = false;

    m_player1Score = 0;
    m_player2Score = 0;

    //reset bubble arrays
    //player 1
    m_allPlayer1Bubbles.clear();
    m_allPlayer2Bubbles.clear();
    generateBubbles();
}

void Game::displayMainMenu(sf::RenderWindow& window, sf::Font font) {
    //display main menu
    sf::Text title("BUBBLE SHOOTER", font, 70);
    title.setPosition(sf::Vector2f(120, 40));
    window.draw(title);

    sf::RectangleShape lobbyBox(sf::Vector2f(400, 300));
    lobbyBox.setPosition(sf::Vector2f(800, 150));
    lobbyBox.setFillColor(sf::Color::Black);
    lobbyBox.setOutlineColor(sf::Color::White);
    lobbyBox.setOutlineThickness(3);
    window.draw(lobbyBox);

    sf::Text lobbyText("- LOBBY -", font, 30);
    lobbyText.setPosition(sf::Vector2f(880, 175));
    lobbyText.setStyle(sf::Text::Underlined);
    window.draw(lobbyText);

    sf::Text player1Text("-> PLAYER 1", font, 28);
    player1Text.setPosition(sf::Vector2f(850, 250));
    window.draw(player1Text);

    sf::Text winDescText1("Pop All Of Your Bubbles,", font, 25);
    winDescText1.setPosition(sf::Vector2f(110, 200));
    window.draw(winDescText1);

    sf::Text winDescText2("Or Get The Highest Score!", font, 25);
    winDescText2.setPosition(sf::Vector2f(95, 250));
    window.draw(winDescText2);

    sf::Text magentaText("MAGENTA", font, 25);
    magentaText.setFillColor(sf::Color::Magenta);
    magentaText.setPosition(sf::Vector2f(35, 370));
    window.draw(magentaText);

    sf::Text powerUpText("Bubbles Are Power-Ups,", font, 25);
    powerUpText.setPosition(sf::Vector2f(235, 370));
    window.draw(powerUpText);

    sf::Text powerUpText2("Any Colour Can Hit Them!", font, 25);
    powerUpText2.setPosition(sf::Vector2f(90, 420));
    window.draw(powerUpText2);

    if (m_player2Connected) {
        sf::Text player2Text("-> PLAYER 2", font, 28);
        player2Text.setPosition(sf::Vector2f(850, 320));
        window.draw(player2Text);

        sf::Text subTitle("-- Press SPACE To Start --", font, 40);
        subTitle.setPosition(sf::Vector2f(90, 520));
        window.draw(subTitle);
    }
    else {
        sf::Text waitingText("- Waiting For Player 2 -", font, 40);
        waitingText.setPosition(sf::Vector2f(120, 520));
        window.draw(waitingText);
    }
}

void Game::displayWinScreen(sf::RenderWindow& window, sf::Font font) {
    //detect which player wins
    sf::RectangleShape winBox(sf::Vector2f(700, 400));
    winBox.setFillColor(sf::Color::Black);
    winBox.setOutlineColor(sf::Color::White);
    winBox.setOutlineThickness(10);
    winBox.setPosition(sf::Vector2f(250, 75));
    window.draw(winBox);

    if (m_player1Win && !m_player2Win) {
        sf::Text p1WinText("Player 1 Wins!", font, 40);
        p1WinText.setPosition(330, 150);
        window.draw(p1WinText);
    }
    else if (m_player2Win && !m_player1Win) {
        sf::Text p2WinText("Player 2 Wins!", font, 40);
        p2WinText.setPosition(330, 150);
        window.draw(p2WinText);
    }
    else {
        sf::Text tieText("Its A Tie!", font, 40);
        tieText.setPosition(400, 150);
        window.draw(tieText);
    }

    sf::Text continueText("Press -ENTER- To", font, 35);
    continueText.setPosition(310, 350);
    window.draw(continueText);

    sf::Text returneText("Return To Lobby", font, 35);
    returneText.setPosition(330, 410);
    window.draw(returneText);
}

void Game::render(sf::RenderWindow& window) {
    //Render left side bubbles
    for (size_t i = 0; i < m_allPlayer1Bubbles.size(); i++) {
        if (m_allPlayer1Bubbles[i].getIsAlive()) {
            m_allPlayer1Bubbles[i].render(window, m_bubbleSize);
        }
    }

    //render left side bubbles
    for (size_t i = 0; i < m_allPlayer2Bubbles.size(); i++) {
        if (m_allPlayer2Bubbles[i].getIsAlive()) {
            m_allPlayer2Bubbles[i].render(window, m_bubbleSize);
        }
    }
}


void Game::udpServer() {
    sf::UdpSocket udpSocketNew;
    unsigned int localPort = 55000;

    //bind port
    if (udpSocketNew.bind(localPort) != sf::Socket::Done) {
        std::cerr << "Binding Error" << std::endl;
        return;
    }
    std::cout << "Bound to port" << std::endl;

    sf::IpAddress senderIP;
    unsigned short remotePort;

    while (true) {
        sf::Packet packet;
        //receive broadcast
        sf::Socket::Status status = udpSocketNew.receive(packet, senderIP, remotePort);
        std::string s;
        packet >> s;
        std::cout << "Server Received: " << s << std::endl;
        
        //send seed
        sf::Packet seedPacket;
        seedPacket << m_seed;
        udpSocketNew.send(seedPacket, senderIP, remotePort);
    }

}