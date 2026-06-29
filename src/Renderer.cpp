
#include "../include/Renderer.hpp"
#include <vector>

Renderer::Renderer(unsigned int width, unsigned int height, sf::Font font):
m_width(width),
m_height(height),
m_texture(sf::Vector2u(width, height)),
m_scale(sf::Vector2f(1.0f, 1.0f)),
m_sprite(m_texture),
m_arrows(sf::PrimitiveType::Lines, width/16 * height/16 * 6),
fpsText(font),
msText(font),
noise(font),
vel(font),
divergence(font),
pressure(font),
diffuse(font),
advect(font),
advectVel(font),
project(font),
addSource(font),
render(font)
{


    //Render logic initialization
    m_sprite.setScale(m_scale);

    //Performance debug text intializing
    fpsText.setCharacterSize(12);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({5.f, 5.f});
  
    msText.setCharacterSize(12);
    msText.setFillColor(sf::Color::White);
    msText.setPosition({5.f, 15.f});

    noise.setCharacterSize(12);
    noise.setFillColor(sf::Color::White);
    noise.setPosition({5.f, 30.f});

    vel.setCharacterSize(12);
    vel.setFillColor(sf::Color::White);
    vel.setPosition({5.f, 45.f});

    divergence.setCharacterSize(12);
    divergence.setFillColor(sf::Color::White);
    divergence.setPosition({5.f, 60.f});

    pressure.setCharacterSize(12);
    pressure.setFillColor(sf::Color::White);
    pressure.setPosition({5.f, 75.f});

    diffuse.setCharacterSize(12);
    diffuse.setFillColor(sf::Color::White);
    diffuse.setPosition({5.f, 90.f});

    advect.setCharacterSize(12);
    advect.setFillColor(sf::Color::White);
    advect.setPosition({5.f, 105.f});

    advectVel.setCharacterSize(12);
    advectVel.setFillColor(sf::Color::White);
    advectVel.setPosition({5.f, 120.f});

    project.setCharacterSize(12);
    project.setFillColor(sf::Color::White);
    project.setPosition({5.f, 135.f});

    addSource.setCharacterSize(12);
    addSource.setFillColor(sf::Color::White);
    addSource.setPosition({5.f, 150.f});

    render.setCharacterSize(12);
    render.setFillColor(sf::Color::White);
    render.setPosition({5.f, 150.f});

}

void Renderer::update(const Grid& grid, bool arrow_viz, bool paused, float dt){
    // calc elapsed time
        float elapsed = m_clock.restart().asSeconds();
        
        //fixed timestep unless paused
        //float dt = paused ?  0.f : 1.f/15.f;

        // calculate performance values
        float fps = paused ? 0.f : 1.f / elapsed;
        float ms = paused ? 0.f : elapsed;


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



        m_renderClock.restart();

        const auto& density = grid.density();
        const auto& u_velocity = grid.u_velocity();
        const auto& v_velocity = grid.v_velocity();
        
        //Creating arrow vector
        if(arrow_viz){
            std::size_t index=0;
            for (std::size_t y = 0; y<m_height; y+=16){
                std::size_t row = y*m_width;
                
                for(std::size_t x =0; x<m_width; x+=16){
                    std::size_t i = row+x;
                    
                    //extract u and v velocity directions
                    float u = u_velocity[i];
                    float v = v_velocity[i];
                    
                    //create arrow points
                    m_arrows[index].position = sf::Vector2f(static_cast<float>(x),static_cast<float>(y));
                    m_arrows[index+1].position = sf::Vector2f(static_cast<float>(x+u),static_cast<float>(y+v));
                    
                    m_arrows[index+2].position = sf::Vector2f(static_cast<float>(x+u),static_cast<float>(y+v));
                    m_arrows[index+3].position = sf::Vector2f(static_cast<float>(x+u - u*.25 + v*.25),static_cast<float>(y+v - v*.25-u*.25));
                    
                    m_arrows[index+4].position = sf::Vector2f(static_cast<float>(x+u),static_cast<float>(y+v));
                    m_arrows[index+5].position = sf::Vector2f(static_cast<float>(x+u - u*.25 - v*.25),static_cast<float>(y+v - v*.25+u*.25));
                    
                    
                    index += 6;
                }
    
            }
        }

        
        const auto& pixels = grid.get_pixels();

        // Load pixel RGBA data into texture
        m_texture.update(pixels.data());


        //calc frame render time
        float renderTime = paused ? 0.f : m_renderClock.restart().asMicroseconds()/1000.f;
        render.setString(
            "\nRender ms: " + std::to_string(renderTime)
        );

}

void Renderer::draw(sf::RenderWindow& window, bool arrow_viz){
    //render density field
        window.draw(m_sprite);

        //render velocity arrows
        if(arrow_viz){
            window.draw(m_arrows);
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
}