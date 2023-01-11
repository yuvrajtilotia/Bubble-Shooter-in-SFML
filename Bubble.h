#include <SFML/Graphics.hpp>
#include <iostream>

class Bubble
{
private:
    sf::Color m_colour = sf::Color::Red;
    sf::Vector2f m_position = sf::Vector2f(0, 0);
    sf::Vector2f m_velocity = sf::Vector2f(0, 0);
    int m_points = rand() % 5;
    int m_size = 20;
    bool m_isAlive = false;
    bool m_hasLaser = false;
    bool m_hasAdder = false;
public:
    Bubble();

    void move();
    void flipXDirection();
    void render(sf::RenderWindow& window, int originOffset);

    sf::Color getColour() { return m_colour; }
    sf::Vector2f getPosition() { return m_position; }
    sf::Vector2f getVelocity() { return m_velocity; }
    int getPoints() { return m_points; }
    bool getIsAlive() { return m_isAlive; }
    bool getHasLaser() { return m_hasLaser; }
    bool getHasAdder() { return m_hasAdder; }

    void setPoints(int points) { m_points = points; }
    void setColour(sf::Color colour) { m_colour = colour; }
    void setPosition(sf::Vector2f position) { m_position = position; }
    void setVelocityX(float x) { m_velocity.x = x; }
    void setVelocityY(float y) { m_velocity.y = y; }
    void setIsAlive(bool alive) { m_isAlive = alive; }
    void setHasLaser(bool laser) { m_hasLaser = laser; }
    void setHasAdder(bool adder) { m_hasAdder = adder; }
};