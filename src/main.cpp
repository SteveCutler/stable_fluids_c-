#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
//#include "../include/Renderer.hpp"
#include "../include/Grid.hpp"
#include "../include/Slider.hpp"




int main()
{
    sf::Font font;
    bool loaded = font.openFromFile("/Users/stevecutler/Library/Fonts/digital-7 (italic).ttf");
    
    bool paused = false;
   
    //SET WIDTH AND HEIGHT
    unsigned int width = 512;
    unsigned int height = 512;    
    //velocity arrow visualization
    bool arrow_viz = false;

    //OBJECT CREATION
    Grid grid{width, height};
    //Renderer Renderer(width, height);
    
    //slider placement from right boundary
    float slider_right = 130.f;
    float slider_gap = 30.f;
    
    
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
    
    
    //grabbing slider controlled variable references
    float* buoyancy = &grid.m_buoyancy;
    float* diffusion = &grid.m_diff_co;
    float* decay = &grid.m_decay;
    float* curl_mult = &grid.m_curl_mult;
    float* noise_freq = &grid.m_noise_freq;


    //Sliders
    Slider buoyancy_slider("Buoyancy", {width-slider_right, 20.f},0.f, 500.f, buoyancy);
    Slider diffusion_slider("Diffusion", {width-slider_right, 50.f},0.f,1.75f, diffusion);
    Slider decay_slider("Decay", {width-slider_right, 80.f},0.8f, 1.f, decay);
    Slider curl_mult_slider("Noise Strength", {width-slider_right, 110.f},0.f, 2000.f, curl_mult);
    Slider noise_freq_slider("Noise Freq.", {width-slider_right, 140.f},0.01f, .07f, noise_freq);
    
    //Sliders vector
    std::vector<Slider*> Sliders{
        &buoyancy_slider,
        &diffusion_slider,
        &decay_slider,
        &curl_mult_slider,
        &noise_freq_slider

    };

    Slider* active_slider = nullptr;

    //initialize Arrow render buffer

    sf::VertexArray arrows(sf::PrimitiveType::Lines, width/16 * height/16 * 6);

    //Performance debug text

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
                if (key->code == sf::Keyboard::Key::P) {
                    paused = !paused;
                    }
                }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::R) {
                    grid.reset_density();
                    }
                }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::M) {
                    grid.m_mult_threaded = !grid.m_mult_threaded;
                    }
                }

            if(event->getIf<sf::Event::MouseButtonPressed>()){
                sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
                sf::Vector2f mousePos = window.mapPixelToCoords(mousePixel);

                for(auto& slider : Sliders){
                    if(slider->contains(mousePos)){
                        active_slider = slider;
                        break;
                    }
                
                }
            }

            if (event->getIf<sf::Event::MouseButtonReleased>()) {
                if (active_slider) {
                    active_slider->dragging = false;
                    active_slider = nullptr;
                }
            }
            
        }
        
        if(active_slider){
            active_slider->dragging = true;
            active_slider->updateFromMouse(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
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
            "\nNoise ms: " + std::to_string(grid.time_noise())
        );

        vel.setString(
            "\nVel ms: " + std::to_string(grid.time_vel())
        );

        divergence.setString(
            "\nDivergence ms: " + std::to_string(grid.time_divergence())
        );

        pressure.setString(
            "\nPressure ms: " + std::to_string(grid.time_pressure())
        );

        diffuse.setString(
            "\nDiffuse ms: " + std::to_string(grid.time_diffuse())
        );

        advect.setString(
            "\nAdvect ms: " + std::to_string(grid.time_advect())
        );

        advectVel.setString(
            "\nAdv Vel ms: " + std::to_string(grid.time_advectVel())
        );

        project.setString(
            "\nProject ms: " + std::to_string(grid.time_project())
        );

        addSource.setString(
            "\nAdd Source ms: " + std::to_string(grid.time_addSource())
        );





        // update
        if(!paused){
            grid.update(dt);
        }

        // DRAW BLOCK

        renderClock.restart();

        const auto& density = grid.density();
        const auto& u_velocity = grid.u_velocity();
        const auto& v_velocity = grid.v_velocity();
        
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

        
        const auto& pixels = grid.get_pixels();

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
        for( auto& slider : Sliders){
            slider->draw(window);
        }


        // display
        window.display();
    }
}