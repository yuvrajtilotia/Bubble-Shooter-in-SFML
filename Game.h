#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <ctime>
#include <vector>

#include "Bubble.h"

class Game
{
private:
	int m_bubbleSize = 20;
	int m_windowWidth = 1200;
	int m_windowHeight = 600;

	int m_player1Score = 0;
	int m_player2Score = 0;

	std::vector<Bubble> m_allPlayer1Bubbles;
	std::vector<Bubble> m_allPlayer2Bubbles;

	const sf::Color colours[5] = {sf::Color::Red, sf::Color(80, 190, 80), sf::Color::Blue, sf::Color(192, 192, 127), sf::Color(127, 192, 192)};

	int m_currentP1BubbleIndex = 0;
	int m_currentP2BubbleIndex = 0;

	bool m_isCannon1Loaded = false;
	bool m_isCannon2Loaded = false;

	bool m_serverCreated = false;

	sf::IpAddress m_address;
	sf::UdpSocket m_udpSocket;
	unsigned short m_portNumber;
	std::shared_ptr<sf::TcpSocket> socket = std::make_shared<sf::TcpSocket>();

	sf::Clock laserClock1;
	sf::Clock laserClock2;
	sf::Clock adderClock1;
	sf::Clock adderClock2;

	int m_playerNumber = 0;

	UINT32 m_seed = 0;

	int bubbleOffset1 = 0;
	int bubbleOffset2 = 0;

	bool m_cannon1Fired = false;
	bool m_cannon2Fired = false;

	bool m_cannon1Left = false;
	bool m_cannon1Right = false;
	bool m_cannon2Left = false;
	bool m_cannon2Right = false;

	bool m_laser1Active = false;
	bool m_laser2Active = false;

	bool m_bubbleAdder1 = false;
	bool m_bubbleAdder2 = false;

	bool m_adder1PickedUp = false;
	bool m_adder2PickedUp = false;

	bool m_player1Win = false;
	bool m_player2Win = false;

	bool m_mainMenuActive = true;
	bool m_player2Connected = false;
	bool m_spacePressed = false;
	bool m_enterPressed = false;

	sf::Clock m_gameTimer;
	int m_maxGameTime = 60;
	int m_currentGameTime = m_maxGameTime + 1;

public:
	Game();

	void generateBubbles();
	void run();

	void udpServer();

	void checkCollisions();
	void checkConsecutiveCollisions(int currentIndex, int playerNumber);

	void resetGame();

	void displayMainMenu(sf::RenderWindow& window, sf::Font font);
	void displayWinScreen(sf::RenderWindow& window, sf::Font font);

	void render(sf::RenderWindow& window);
};

