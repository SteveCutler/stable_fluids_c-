#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include "../include/Renderer.hpp"
#include "../include/Grid.hpp"
#include "../include/Slider.hpp"




int main()
{
    
    
    //SET WIDTH AND HEIGHT
    unsigned int width = 512;
    unsigned int height = 512;    
    
    //velocity arrow visualization
    bool arrow_viz = false;
    bool paused = false;
    
    //Font loading
    sf::Font font;
    bool loaded = font.openFromFile("/Users/stevecutler/Library/Fonts/digital-7 (italic).ttf");

    //OBJECT CREATION
    Grid grid{width, height};
    Renderer renderer(width, height, font);
    // create the window
    sf::RenderWindow window(sf::VideoMode({width, height}), "Wind Sim");
    // create a clock to track the elapsed time
    sf::Clock clock;
    
    
    //SLIDER UI//
    ////

    //Brabbing slider controlled variable references
    float* buoyancy = &grid.m_buoyancy;
    float* diffusion = &grid.m_diff_co;
    float* decay = &grid.m_decay;
    float* curl_mult = &grid.m_curl_mult;
    float* noise_freq = &grid.m_noise_freq;
    
    std::cout << "decay:" << std::to_string(*decay);
    //Slider placement variables
    float slider_right = 130.f;
    
    //Sliders
    Slider buoyancy_slider("Buoyancy", {width-slider_right, 20.f},0.f, 50.f, buoyancy);
    Slider diffusion_slider("Diffusion", {width-slider_right, 50.f},0.f,5.f, diffusion);
    Slider decay_slider("Decay", {width-slider_right, 80.f},0.95f, 0.9999f, decay);
    Slider curl_mult_slider("Noise Strength", {width-slider_right, 110.f},0.f, 2000.f, curl_mult);
    Slider noise_freq_slider("Noise Freq.", {width-slider_right, 140.f},0.01f, .07f, noise_freq);
    
    //Active slider state buffer
    Slider* active_slider = nullptr;

    //Sliders vector container
    std::vector<Slider*> Sliders{
        &buoyancy_slider,
        &diffusion_slider,
        &decay_slider,
        &curl_mult_slider,
        &noise_freq_slider
        
    };


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
        
        

        //calculate dt
        //float dt = clock.restart().asSeconds();
        float dt = 1/15.f;

        // UPDATE //
        ////
        if(!paused){
            grid.update(dt);
        }
        if(!paused){
            renderer.update(grid,arrow_viz);
        }

        // DRAW BLOCK
        
        // clear window
        window.clear();

        renderer.draw(window, arrow_viz);
        
        //slider drawing
        for( auto& slider : Sliders){
            slider->draw(window);
        }


        // display
        window.display();
    }
}