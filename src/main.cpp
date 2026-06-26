#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
//#include "../include/Renderer.hpp"
#include "../include/Grid.hpp"

struct Slider{
    sf::RectangleShape track;
    sf::CircleShape knob;

    float* target;
    float min_value;
    float max_value;
    bool dragging =false;


    Slider(sf::Vector2f position, float min_value, float max_value, float* target){
        this->target=target;
        this->min_value=min_value;
        this->max_value=max_value;


        track.setFillColor({100, 100, 100});
        track.setPosition(position);
        track.setSize(sf::Vector2f(120.f,5.f));
        
        float amount =std::clamp(((*target - min_value)/max_value),0.f,1.f);
        
        float knob_pos_x = track.getPosition().x + track.getSize().x*amount;
        float knob_pos_y = track.getPosition().y - track.getSize().y/2.f;

        knob.setPosition({knob_pos_x,knob_pos_y});
        knob.setFillColor(sf::Color::Red);
        knob.setScale({4.f,4.f});
        knob.setRadius(1.f);
    }

    void updateFromMouse(sf::Vector2f pos){
        float percentage = (pos.x-track.getPosition().x)/track.getSize().x; 
        float amount = min_value + (max_value-min_value)*percentage;
        *target = amount;
        float knobPos = track.getPosition().x + track.getSize().x*percentage;

        knob.setPosition({
            knobPos,
            knob.getPosition().y
        }
        );

    
    }

    bool contains(sf::Vector2f pos){
        
        if(track.getGlobalBounds().contains(pos) or knob.getGlobalBounds().contains(pos)){
            return true;

        }
        return false;
    }

    //draw function
    void draw(sf::RenderWindow& window){
        window.draw(track);
        window.draw(knob);
    }

};

int main()
{
   
    //SET WIDTH AND HEIGHT
    unsigned int width = 512;
    unsigned int height = 512;    
    //velocity arrow visualization
    bool arrow_viz = false;

    //OBJECT CREATION
    Grid Grid{width, height};
    //Renderer Renderer(width, height);


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

    float* b = &Grid.m_buoyancy;

    //Sliders
    Slider buoyancy_slider({width-150.f, 20.f},0.f, 500.f, b);


    //initialize render buffers
    
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

    sf::Text advectVel(font);
    advectVel.setCharacterSize(12);
    advectVel.setFillColor(sf::Color::White);
    advectVel.setPosition({5.f, 120.f});

    sf::Text project(font);
    project.setCharacterSize(12);
    project.setFillColor(sf::Color::White);
    project.setPosition({5.f, 135.f});

    sf::Text addSource(font);
    addSource.setCharacterSize(12);
    addSource.setFillColor(sf::Color::White);
    addSource.setPosition({5.f, 150.f});

    sf::Text render(font);
    render.setCharacterSize(12);
    render.setFillColor(sf::Color::White);
    render.setPosition({5.f, 165.f});


    // run the main loop
    while (window.isOpen())
    {

        // handle events
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::V) {
                    arrow_viz = !arrow_viz;
                    }
                }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::R) {
                    Grid.reset_density();
                    }
                }
            if(event->getIf<sf::Event::MouseButtonPressed>()){
                sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
                sf::Vector2f mousePos = window.mapPixelToCoords(mousePixel);

                if(buoyancy_slider.contains(mousePos)){
                    buoyancy_slider.dragging = true;
                    buoyancy_slider.updateFromMouse(mousePos);

                };
            }
           
            }

        // calc elapsed time
        float elapsed = clock.restart().asSeconds();
        
        //fixed timestep
        float dt = 1.f/15.f;

        // calculate performance values
        float fps = 1.f / elapsed;
        float ms = elapsed;


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

        advectVel.setString(
            "\nAdv Vel ms: " + std::to_string(Grid.time_advectVel())
        );

        project.setString(
            "\nProject ms: " + std::to_string(Grid.time_project())
        );

        addSource.setString(
            "\nAdd Source ms: " + std::to_string(Grid.time_addSource())
        );





        // update
        Grid.update(dt);


        // DRAW BLOCK

        renderClock.restart();

        const auto& density = Grid.density();
        const auto& u_velocity = Grid.u_velocity();
        const auto& v_velocity = Grid.v_velocity();
        
        //Creating arrow vector
        if(arrow_viz){
            std::size_t index=0;
            for (std::size_t y = 0; y<height; y+=16){
                std::size_t row = y*width;
                
                for(std::size_t x =0; x<width; x+=16){
                    std::size_t i = row+x;
                    
                    //extract u and v velocity directions
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
        }

        
        const auto& pixels = Grid.get_pixels();

        // Load pixel RGBA data into texture
        texture.update(pixels.data());


        //calc render time
        render.setString(
            "\nRender ms: " + std::to_string(renderClock.restart().asMicroseconds()/1000.f)
        );
        
        // clear window
        window.clear();
        
        //render density field
        window.draw(sprite);

        //render velocity arrows
        if(arrow_viz){
            window.draw(arrows);
        }

        //render debug text
        window.draw(fpsText);
        window.draw(msText);

        //kernel profiling text
        window.draw(addSource);
        window.draw(noise);
        window.draw(vel);
        window.draw(divergence);
        window.draw(pressure);
        window.draw(diffuse);
        window.draw(advect);
        window.draw(advectVel);
        window.draw(project);
        window.draw(render);

        //slider drawing
        buoyancy_slider.draw(window);

        // display
        window.display();
    }
}