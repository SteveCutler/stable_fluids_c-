#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "../include/Grid.hpp"

class Renderer
{
private:
    unsigned int m_width;
    unsigned int m_height;
    sf::Texture m_texture;
    sf::VertexArray m_arrows;
    sf::Vector2f m_scale;
    sf::Text fpsText;
    sf::Text msText;
    sf::Text noise;
    sf::Text vel;
    sf::Text divergence;
    sf::Text pressure;
    sf::Text diffuse;
    sf::Text advect;
    sf::Text advectVel;
    sf::Text project;
    sf::Text addSource;   
    sf::Clock m_clock;
    sf::Clock m_renderClock;
    sf::Text render;
    sf::Sprite m_sprite;
    

public:
    Renderer(unsigned int width, unsigned int height, sf::Font font);
    void update(const Grid& grid, bool arrow_viz, bool paused, float dt);
    void draw(sf::RenderWindow& window, bool arrow_viz);
    

};
