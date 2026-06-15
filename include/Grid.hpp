#pragma once

#include <cstddef>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../include/FastNoiseLite.h"


class Grid
    {

    public:
        Grid(std::size_t width, std::size_t height);

        void update(float dt);

        std::pair<std::size_t,std::size_t> get_xy(std::size_t i);

        //field getters
        const std::vector<float>& density() const;
        const std::vector<float>& u_velocity() const;
        const std::vector<float>& v_velocity() const;
        const std::vector<float>& get_noise() const;

    private:

        void clearPressure();
        
        size_t calcPos(size_t x, size_t y);

        void calcNoise(float dt);

        void calcVel();

        void calcDivergence();

        void solvePressure();

        void project();
        
        float distance(sf::Vector2f first, sf::Vector2f second);
        
        void addSource(size_t x, size_t y, float size);

        void spawn(std::size_t width, std::size_t height);
        
        void swapDensity();

        void diffuse(float dt);
        
        void advect(float dt);
        
        void decay(float dt);

        FastNoiseLite m_noise_gen;



    
    private:

        //Global variables
        int m_width;
        int m_height;
        float m_diff_co;
        float m_decay;
        float m_source;
        float m_elapsed;
        float m_curl_mult;

        // SoA member variables
        std::vector<float> m_density;
        std::vector<float> m_u_velocity;
        std::vector<float> m_v_velocity;

        std::vector<float>m_density_prev;
        std::vector<float>m_u_velocity_prev;
        std::vector<float>m_v_velocity_prev;

        std::vector<float> m_noise;

        std::vector<float> m_pressure;
        std::vector<float> m_pressure_prev;

        std::vector<float> m_divergence;

    };
