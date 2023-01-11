#include "Bubble.h"

Bubble::Bubble() {
}

void Bubble::move() {
    m_position += m_velocity;
}

void Bubble::flipXDirection() {
    m_velocity.x = -m_velocity.x;
}

void Bubble::render(sf::RenderWindow& window, int originoffset) {
    sf::CircleShape circle(m_size);
    circle.setOrigin(originoffset, originoffset);
    circle.setPosition(m_position);
    circle.setFillColor(m_colour);
    window.draw(circle);
}