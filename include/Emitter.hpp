#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

class Emitter
{

public:
Emitter(sf::Vector2f pos, float rad, sf::Color clr);

sf::Vector2f m_pos;
float m_rad;
sf::Color m_clr;
};



