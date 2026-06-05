#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include "../include/Grid.hpp"


int main()
{
   
    //SET WIDTH AND HEIGHT
    unsigned int width = 512;
    unsigned int height = 512;

    //Pixel size
    float pixelSize = 3.0f;

    // create the window
    sf::RenderWindow window(sf::VideoMode({width, height}), "Wind Sim");

    // create Grid
    Grid Grid{width, height};

    //Render logic initialization

    sf::Texture texture(sf::Vector2u(width, height));
    
    sf::Vector2f scale = sf::Vector2f(1.0f, 1.0f);

    sf::Sprite sprite(texture);
    sprite.setScale(scale);

    std::vector<std::uint8_t> pixels(width * height * 4, 0);

    //Performance debug text
    sf::Font font;
    bool loaded = font.openFromFile("/Users/stevecutler/Library/Fonts/digital-7 (italic).ttf");

    sf::Text fpsText(font);
    fpsText.setCharacterSize(12);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({5.f, 5.f});
  
    sf::Text msText(font);
    msText.setCharacterSize(12);
    msText.setFillColor(sf::Color::White);
    msText.setPosition({5.f, 15.f});


    // create a clock to track the elapsed time
    sf::Clock clock;


    // run the main loop
    while (window.isOpen())
    {
        // handle events
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // calc elapsed time
        float dt = clock.restart().asSeconds();

        // calculate performance values
        float fps = 1.f / dt;
        float ms = dt * 1000.f;

        // overlay strings
        fpsText.setString(
            "FPS: " + std::to_string(static_cast<int>(fps))
        );

        msText.setString(
            "\nFrame ms: " + std::to_string(ms)
        );

        // update
        Grid.update(dt);


        // DRAW BLOCK

        const auto& density = Grid.density();

        //Convert density buffer to pixel data
        for (int i =0; i<density.size(); i++){

            //clamp between 0 and 1
            float d = std::clamp(density[i],0.0f, 1.0f);

            //convert to RGB values
            std::uint8_t value = static_cast<std::uint8_t>(d * 255.f);

            //find correct position in pixel array, given each pixel has 1 components
            std::size_t p = i * 4;

            //create greyscale image with alpha of 1
            pixels[p] = value;  //R
            pixels[p+1] = value;//G
            pixels[p+2] = value;//B
            pixels[p+3] = 255;  //Alpha channel
        }

        // Load pixel RGBA data into texture
        texture.update(pixels.data());
        

        // clear window
        window.clear();

        // draw text
        
        window.draw(sprite);
        window.draw(fpsText);
        window.draw(msText);

        // display
        window.display();
    }
}