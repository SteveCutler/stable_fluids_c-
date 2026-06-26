
#include "../include/Renderer.hpp"
#include <vector>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

Renderer::Renderer(unsigned int width, unsigned int height):
m_width(width),
m_height(height)
{


    // create a clock to track the elapsed time
    sf::Clock clock;
    sf::Clock renderClock;
    // create the window
    sf::RenderWindow window(sf::VideoMode({width, height}), "Wind Sim");
    // create Grid
    //Render logic initialization

    sf::Texture texture(sf::Vector2u(width, height));
    
    sf::Vector2f scale = sf::Vector2f(1.0f, 1.0f);

    sf::Sprite sprite(texture);
    sprite.setScale(scale);


    //initialize render buffers
    
    sf::VertexArray arrows(sf::PrimitiveType::Lines, width/16 * height/16 * 6);

    //Performance debug text
    sf::Font font;
    bool loaded = font.openFromFile("/Users/stevecutler/Library/Fonts/digital-7 (italic).ttf");
}

Renderer::~Renderer()
{
}
