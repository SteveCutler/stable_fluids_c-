#pragma once

#include <cstddef>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
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

        //timing getters
        sf::Clock m_performance_clock;
        float time_noise() const;
        float time_vel() const;
        float time_divergence() const;
        float time_pressure() const;
        float time_diffuse() const;
        float time_advect() const;


    private:
        size_t calcPos(size_t x, size_t y);

        void advectVel(float dt);

        void diffuseVel(float dt);

        void decayVel();

        void swapVel();

        void clearPressure();
        
        void calcNoise(float dt);

        void calcVel(float dt);

        void calcDivergence();

        void solvePressure();

        void velBoundaries();

        void pressureBoundaries();

        void project();

        void projectStep();
        
        void addSource(size_t x, size_t y, float size);

        void spawn(std::size_t width, std::size_t height);
        
        void swapDensity();

        void diffuse(float dt);
        
        void advect_decay(float dt);

        FastNoiseLite m_noise_gen;




    
    private:

        //Global variables
        int m_width;
        int m_height;
        float m_diff_co;
        float m_decay;
        float m_vel_decay;
        float m_source;
        float m_elapsed;
        float m_curl_mult;
        float m_viscosity;

        //Timing Debug variables
        float m_noise_ms;
        float m_vel_ms;
        float m_div_ms;
        float m_pressure_ms;
        float m_advect_ms;
        float m_diffuse_ms;

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
