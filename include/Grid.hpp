#pragma once

#include <cstddef>
#include <vector>
#include <SFML/Graphics.hpp>

class Grid
    {

    public:
        Grid(std::size_t width, std::size_t height);

        void update(float dt);

        const std::vector<float>& density() const;

    private:

        size_t calcPos(size_t x, size_t y);

        float distance(sf::Vector2f first, sf::Vector2f second);
        
        void addSource(size_t x, size_t y, int size);

        void spawn(std::size_t width, std::size_t height);
        
        void diffuse(float dt);
        
        void advect(float dt);

        void swap();

        std::pair<std::size_t,std::size_t> get_xy(std::size_t i);

    
    private:

        //Global variables
        int m_width;
        int m_height;
        float m_diff_co;

        // SoA member variables
        std::vector<float> m_density;
        std::vector<float> m_u_velocity;
        std::vector<float> m_v_velocity;

        std::vector<float>m_density_prev;
        std::vector<float>m_u_velocity_prev;
        std::vector<float>m_v_velocity_prev;

    };
