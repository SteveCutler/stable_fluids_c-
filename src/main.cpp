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

    // create a clock to track the elapsed time
    sf::Clock clock;

    // create the window
    sf::RenderWindow window(sf::VideoMode({width, height}), "Wind Sim");

    // create Grid
    Grid Grid{width, height};

    //Render logic initialization

    sf::Texture texture(sf::Vector2u(width, height));
    
    sf::Vector2f scale = sf::Vector2f(1.0f, 1.0f);

    sf::Sprite sprite(texture);
    sprite.setScale(scale);


    //initialize render buffers
    std::vector<std::uint8_t> pixels(width * height * 4, 0);
    sf::VertexArray arrows(sf::PrimitiveType::Lines, width/16 * height/16 * 6);

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

    //Kernel debugging text
    sf::Text noise(font);
    noise.setCharacterSize(12);
    noise.setFillColor(sf::Color::White);
    noise.setPosition({5.f, 30.f});

    sf::Text vel(font);
    vel.setCharacterSize(12);
    vel.setFillColor(sf::Color::White);
    vel.setPosition({5.f, 45.f});

    sf::Text divergence(font);
    divergence.setCharacterSize(12);
    divergence.setFillColor(sf::Color::White);
    divergence.setPosition({5.f, 60.f});

    sf::Text pressure(font);
    pressure.setCharacterSize(12);
    pressure.setFillColor(sf::Color::White);
    pressure.setPosition({5.f, 75.f});

    sf::Text diffuse(font);
    diffuse.setCharacterSize(12);
    diffuse.setFillColor(sf::Color::White);
    diffuse.setPosition({5.f, 90.f});

    sf::Text advect(font);
    advect.setCharacterSize(12);
    advect.setFillColor(sf::Color::White);
    advect.setPosition({5.f, 105.f});


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
        float elapsed = clock.restart().asSeconds();
        
        //fixed timestep
        float dt = 1.f/15.f;

        // calculate performance values
        float fps = 1.f / elapsed;
        float ms = dt * 1000.f;


        // overlay strings
        fpsText.setString(
            "FPS: " + std::to_string(static_cast<int>(fps))
        );

        msText.setString(
            "\nFrame ms: " + std::to_string(ms)
        );

        noise.setString(
            "\nNoise ms: " + std::to_string(Grid.time_noise())
        );

        vel.setString(
            "\nVel ms: " + std::to_string(Grid.time_vel())
        );

        divergence.setString(
            "\nDivergence ms: " + std::to_string(Grid.time_divergence())
        );

        pressure.setString(
            "\nPressure ms: " + std::to_string(Grid.time_pressure())
        );

        diffuse.setString(
            "\nDiffuse ms: " + std::to_string(Grid.time_diffuse())
        );

        advect.setString(
            "\nAdvect ms: " + std::to_string(Grid.time_advect())
        );



        // update
        Grid.update(dt);


        // DRAW BLOCK

        const auto& density = Grid.density();
        const auto& u_velocity = Grid.u_velocity();
        const auto& v_velocity = Grid.v_velocity();

        //Convert density buffer to pixel data
        for (int i =0; i<density.size(); i++){

            //clamp between 0 and 1
            float d = std::clamp(density[i],0.0f, 1.0f);
            

            //convert density to RGB values
            std::uint8_t value = static_cast<std::uint8_t>(d * 255.f);

            //find correct position in pixel array, given each pixel has 1 components
            std::size_t p = i * 4;

            //create greyscale image with alpha of 1
            pixels[p] = value;  //R
            pixels[p+1] = value;//G
            pixels[p+2] = value;//B
            pixels[p+3] = 255;  //Alpha channel
        }
        std::size_t index = 0;
        for (std::size_t x = 0; x<width; x+=16){

            for(std::size_t y =0; y<height; y+=16){

                //extract u and v velocity directions
                std::size_t i = x+y*width;
                float u = u_velocity[i];
                float v = v_velocity[i];
    
                //create arrow points
                arrows[index].position = sf::Vector2f(static_cast<float>(x),static_cast<float>(y));
                arrows[index+1].position = sf::Vector2f(static_cast<float>(x+u),static_cast<float>(y+v));

                arrows[index+2].position = sf::Vector2f(static_cast<float>(x+u),static_cast<float>(y+v));
                arrows[index+3].position = sf::Vector2f(static_cast<float>(x+u - u*.25 + v*.25),static_cast<float>(y+v - v*.25-u*.25));

                arrows[index+4].position = sf::Vector2f(static_cast<float>(x+u),static_cast<float>(y+v));
                arrows[index+5].position = sf::Vector2f(static_cast<float>(x+u - u*.25 - v*.25),static_cast<float>(y+v - v*.25+u*.25));


                index += 6;
            }

        }

        // Load pixel RGBA data into texture
        texture.update(pixels.data());
        
        // clear window
        window.clear();
        
        //render density field
        window.draw(sprite);

        //render velocity arrows
       // window.draw(arrows);

        //render debug text
        window.draw(fpsText);
        window.draw(msText);

        //kernel profiling text
        window.draw(noise);
        window.draw(vel);
        window.draw(divergence);
        window.draw(pressure);
        window.draw(diffuse);
        window.draw(advect);

        // display
        window.display();
    }
}