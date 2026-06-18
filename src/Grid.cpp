#include "../include/Grid.hpp"
#include <vector>
#include<iostream>
#include <random>




Grid::Grid(std::size_t width, std::size_t height):
m_width(width),
m_height(height),
m_density(width*height, 0.0f),
m_v_velocity(width*height, 0.0f),
m_u_velocity(width*height, 0.0f),
m_density_prev(width*height, 0.0f),
m_v_velocity_prev(width*height, 0.0f),
m_u_velocity_prev(width*height, 0.0f),
m_source(50.f),
m_elapsed(0.f),
m_noise(width*height, 0.0f),
m_pressure(width*height, 0.0f),
m_pressure_prev(width*height, 0.0f),
m_divergence(width*height, 0.0f),
m_noise_ms(0.f),
m_vel_ms(0.f),
m_div_ms(0.f),
m_pressure_ms(0.f),
m_advect_ms(0.f),
m_diffuse_ms(0.f),
m_diff_co(1.f),
m_decay(0.999f),
m_vel_decay(0.96f),
m_viscosity(1.f),
m_curl_mult(1000)
{

    //configure random num gen
    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int> distr(1, 100);

    //configure noise generator
    m_noise_gen.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_noise_gen.SetSeed(distr(gen));
    m_noise_gen.SetFrequency(0.04f);

}

//field getters
const std::vector<float>& Grid::density() const{
    return m_density;
}

const std::vector<float>& Grid::u_velocity() const{
    return m_u_velocity;
};

const std::vector<float>& Grid::v_velocity() const{
    return m_v_velocity;
};

const std::vector<float>& Grid::get_noise() const{
    return m_noise;
};

// timing getters

float Grid::time_noise() const {
    return m_noise_ms;
};

float Grid::time_vel() const{
    return m_vel_ms;
};

float Grid::time_divergence() const{
    return m_div_ms;
};

float Grid::time_pressure() const{
    return m_pressure_ms;
};

float Grid::time_diffuse() const{
    return m_diffuse_ms;
};

float Grid::time_advect() const{
    return m_advect_ms;
};


//UPDATE LOOP

void Grid::update(float dt){
    //Create noise scalar field
    m_performance_clock.restart();
    calcNoise(dt*50);
    m_noise_ms = m_performance_clock.restart().asMilliseconds();
    
    
    //Create vel gradient field based on noise
    m_performance_clock.restart();
    calcVel(dt);
    m_vel_ms = m_performance_clock.restart().asMilliseconds();
    
    // apply velocity boundary calculations
    velBoundaries();

    //first pressure solve
   // projectStep();

    //Advect velocity
    swapVel();
    advectVel(dt);

    //apply velocity boundary conditions
    velBoundaries();
    
    
    //Diffuse velocity
    swapVel();
    diffuseVel(dt*30);  
    velBoundaries();

    //second pressure solve
    //projectStep();
    decayVel();  

    //Add density source
    addSource(m_width/2,m_height/2, m_source);
    
    //Diffuse
    swapDensity();
    m_performance_clock.restart();
    diffuse(dt);
    m_diffuse_ms = m_performance_clock.restart().asMilliseconds();
    
    //Advect
    swapDensity();
    m_performance_clock.restart();
    advect(dt);
    m_advect_ms = m_performance_clock.restart().asMilliseconds();
    

    //Decay
    swapDensity();
    decay(dt);
}
    


//HELPERS

void Grid::projectStep(){
    clearPressure();
    m_performance_clock.restart();
    calcDivergence();
    m_div_ms = m_performance_clock.restart().asMilliseconds();
    
    
    m_performance_clock.restart();
    solvePressure();
    m_pressure_ms = m_performance_clock.restart().asMilliseconds();
    pressureBoundaries();
    
    //m_performance_clock.restart();
    project();
    velBoundaries();

}

void Grid::clearPressure(){
    std::fill(m_pressure.begin(),m_pressure.end(),0.f);
    std::fill(m_pressure_prev.begin(),m_pressure_prev.end(),0.f);
};


void Grid::calcNoise(float dt){
    m_elapsed += dt;
    std::size_t index = 0;

    for(std::size_t y = 0; y<m_height; y++){
         for(std::size_t x = 0; x<m_width; x++){
            
            float n = m_noise_gen.GetNoise(x*1.f,y*1.f,m_elapsed);
            m_noise[index] = n;
            index++;
        }
    }
};

void Grid::calcVel(float dt){
    for(std::size_t y = 0; y<m_height; y++){
         for(std::size_t x = 0; x<m_width; x++){

            if(x>0 && x<m_width-1
            && y>0 && y<m_height-1){

                std::size_t index = x+y*m_width;

                float xl = m_noise[(x-1)+y*m_width];
                float xr = m_noise[(x+1)+y*m_width];
                float yt = m_noise[x+(y-1)*m_width];
                float yb = m_noise[x+(y+1)*m_width];

                float dx = (xr-xl)/2;
                float dy = (yb-yt)/2;

                m_u_velocity[index] += -dy*m_curl_mult*dt;
                m_v_velocity[index] += dx*m_curl_mult*dt;
               
            }

        }
    }
}

//DYNAMIC VEL FUNCTIONS

void Grid::advectVel(float dt){
    float new_u_vel = 0.f;
    float new_v_vel = 0.f;

    for(std::size_t i=0; i<m_density.size(); i++){
        //index velocity
        float old_u_vel = m_u_velocity_prev[i];
        float old_v_vel = m_v_velocity_prev[i];

        //xy values for index
        std::size_t x = i%(m_width);
        std::size_t y = std::floor(i/(m_width));

        //backwards location lookup 
        float new_u_pos = x - old_u_vel*dt;
        float new_v_pos = y - old_v_vel*dt;

        //finding 4 corners for bilinear interpolation, clamping in boundaries
        float x_sample = std::clamp(new_u_pos,1.0f,m_width-2.0f);
        float y_sample = std::clamp(new_v_pos,1.0f,m_height-2.0f);

        std::size_t x_floor = std::floor(x_sample);
        std::size_t x_high = x_floor+1;
        std::size_t y_floor = std::floor(y_sample);
        std::size_t y_high = y_floor+1;

        //finding values at corners
        float tl_u = m_u_velocity_prev[calcPos(x_floor,y_floor)];
        float tr_u = m_u_velocity_prev[calcPos(x_high,y_floor)];
        float bl_u = m_u_velocity_prev[calcPos(x_floor,y_high)];
        float br_u = m_u_velocity_prev[calcPos(x_high,y_high)];
        
        float tl_v = m_v_velocity_prev[calcPos(x_floor,y_floor)];
        float tr_v = m_v_velocity_prev[calcPos(x_high,y_floor)];
        float bl_v = m_v_velocity_prev[calcPos(x_floor,y_high)];
        float br_v = m_v_velocity_prev[calcPos(x_high,y_high)];

        //distance from edges
        float dx = x_sample - x_floor;
        float dy = y_sample - y_floor;

        //weighting calculations
        float tl_weight = (1-dx)*(1-dy);
        float tr_weight = dx*(1-dy);
        float bl_weight = (1-dx)*dy;
        float br_weight = dx*dy;

        new_u_vel = tl_u*tl_weight + tr_u*tr_weight + bl_u*bl_weight + br_u*br_weight;
        new_v_vel = tl_v*tl_weight + tr_v*tr_weight + bl_v*bl_weight + br_v*br_weight;
        
        m_u_velocity[i] = new_u_vel;
        m_v_velocity[i] = new_v_vel;

    }
};

void Grid::diffuseVel(float dt){

    float left_u = 0.f;
    float right_u = 0.f;
    float up_u = 0.f;
    float down_u = 0.f;
    float center_u = 0.f;

    float left_v = 0.f;
    float right_v = 0.f;
    float up_v = 0.f;
    float down_v = 0.f;
    float center_v = 0.f;

    float lap_u = 0.f;
    float lap_v = 0.f;

    float new_u_vel = 0.f;
    float new_v_vel = 0.f;

    for (std::size_t i = 0; i<m_density.size(); i++){
        //get xy coordinates
        std::size_t x = i%(m_width);
        std::size_t y = std::floor(i/(m_width));

        //checking boundary conditions
        if(x>0 && x < m_width-1 &&
           y>0 && y < m_height-1)
           {
            //getting data for laplacian
            left_u = m_u_velocity_prev[calcPos(x-1,y)];
            right_u = m_u_velocity_prev[calcPos(x+1,y)];
            up_u = m_u_velocity_prev[calcPos(x,y-1)];
            down_u = m_u_velocity_prev[calcPos(x,y+1)];
            center_u = m_u_velocity_prev[i];

            left_v = m_v_velocity_prev[calcPos(x-1,y)];
            right_v = m_v_velocity_prev[calcPos(x+1,y)];
            up_v = m_v_velocity_prev[calcPos(x,y-1)];
            down_v = m_v_velocity_prev[calcPos(x,y+1)];
            center_v = m_v_velocity_prev[i];

            //calculate laplacian
            lap_u = left_u+right_u+up_u+down_u - (4*center_u);
            lap_v = left_v+right_v+up_v+down_v - (4*center_v);

            //calculate and write new density
            new_u_vel = m_u_velocity_prev[i]+ m_viscosity*lap_u*dt;
            new_v_vel = m_v_velocity_prev[i]+ m_viscosity*lap_u*dt;

            }
        else{
            //leave boundary conditions the same
            new_u_vel = m_u_velocity_prev[i];
            new_v_vel = m_v_velocity_prev[i];
            }
    }
};

void Grid::decayVel(){
    for(std::size_t i=0; i<m_density.size(); i++){
        m_u_velocity[i]*=m_vel_decay;
        m_v_velocity[i]*=m_vel_decay;
    }
};

void Grid::swapVel(){
    std::swap(m_u_velocity, m_u_velocity_prev);
    std::swap(m_v_velocity, m_v_velocity_prev);
};

void Grid::velBoundaries(){
    //set vel on left and right boundaries
    for(std::size_t y = 0; y<m_height; y++){
        std::size_t b_l = y*m_width;
        std::size_t b_r = (m_width-1)+y*m_width;

        //horizontal vel 0
        m_u_velocity[b_l] = 0.f;
        m_u_velocity[b_r] = 0.f;
        //vertical vel refers to neighbour
        m_v_velocity[b_l] = m_v_velocity[b_l+1];
        m_v_velocity[b_r] = m_v_velocity[b_r-1];
    }

    //set vel for top and bottom boundaries
    for(std::size_t x = 0; x<m_width; x++){
        std::size_t b_t = x;
        std::size_t b_b = x+(m_height-1)*m_width;

        //vertical vel refers to neighbour
        m_u_velocity[b_t] = m_u_velocity[b_t+m_width];
        m_u_velocity[b_b] = m_u_velocity[b_b-m_width];
        //vertical vel 0
        m_v_velocity[b_t] =0.f;
        m_v_velocity[b_b] = 0.f;
    }
}

void Grid::pressureBoundaries(){
    //set pressure on left and right boundaries
    for(std::size_t y = 0; y<m_height; y++){
        std::size_t b_l = y*m_width;
        std::size_t b_r = (m_width-1)+y*m_width;


        //pressure refers to neighbour
        m_pressure[b_l] = m_pressure[b_l+1];
        m_pressure[b_r] = m_pressure[b_r-1];
    }

    //set pressure for top and bottom boundaries
    for(std::size_t x = 0; x<m_width; x++){
        std::size_t b_t = x;
        std::size_t b_b = x+(m_height-1)*m_width;

        //refers to vertical neighbour
        m_pressure[b_t] = m_pressure[b_t+m_width];
        m_pressure[b_b] = m_pressure[b_b-m_width];

    }
}

void Grid::calcDivergence(){

    float max_div = 0.f;
    
    for(std::size_t y = 0; y<m_height; y++){
            for(std::size_t x = 0; x<m_width; x++){

                if(x>0 && x<m_width-1
                && y>0 && y<m_height-1){

                    std::size_t index = x+y*m_width;

                    float xl = m_u_velocity[(x-1)+y*m_width];
                    float xr = m_u_velocity[(x+1)+y*m_width];
                    float yt = m_v_velocity[x+(y-1)*m_width];
                    float yb = m_v_velocity[x+(y+1)*m_width];

                    float div =  ((xr-xl)/2)+((yb-yt)/2);

                    // if(x>2 && x<m_width-2
                    // && y>2 && y<m_height-2){
                    // }
                    max_div = std::max(max_div, abs(div));

                    m_divergence.at(index) = div;
                
                }
            }
        }
        std::cout << "max divergence: " << max_div << "\n";
};

void Grid::solvePressure(){

    std::size_t k = 0;
    while(k<20){

        for(std::size_t y = 0; y<m_height; y++){
                for(std::size_t x = 0; x<m_width; x++){

                    if(x>0 && x<m_width-1
                    && y>0 && y<m_height-1){

                        std::size_t index = x+y*m_width;

                        float xl = m_pressure_prev[(x-1)+y*m_width];
                        float xr = m_pressure_prev[(x+1)+y*m_width];
                        float yt = m_pressure_prev[x+(y-1)*m_width];
                        float yb = m_pressure_prev[x+(y+1)*m_width];

                        float pressure =  (xl+xr+yt+yb - m_divergence.at(index))/4.0f;

                        m_pressure.at(index) = pressure;
                    
                    }
                }
            }
            pressureBoundaries();
            std::swap(m_pressure, m_pressure_prev);
            k++;
            //std::cout << "max pressure =" << pressure_max << "\n" << "min pressure =" << pressure_min << "\n\n";
        }
        std::swap(m_pressure, m_pressure_prev);
        pressureBoundaries();

};

void Grid::project(){

        for(std::size_t y = 0; y<m_height; y++){
            for(std::size_t x = 0; x<m_width; x++){

                if(x>0 && x<m_width-1
                && y>0 && y<m_height-1){

                    std::size_t index = x+y*m_width;

                    float xl = m_pressure[(x-1)+y*m_width];
                    float xr = m_pressure[(x+1)+y*m_width];
                    float yt = m_pressure[x+(y-1)*m_width];
                    float yb = m_pressure[x+(y+1)*m_width];

                    float pres_x =  (xr-xl)/2.f;
                    float pres_y =  (yb-yt)/2.f;

                    m_u_velocity[index] -= pres_x;
                    m_v_velocity[index] -= pres_y;
                
                }
            }
        }

};

void Grid::swapDensity(){
    std::swap(m_density, m_density_prev);
}

size_t Grid::calcPos(size_t x, size_t y){
    size_t pos = m_width*y+x;
    return pos;
}

std::pair<std::size_t,std::size_t> Grid::get_xy(std::size_t i){
    size_t x = i%m_width;
    size_t y = i/m_width;
    return std::pair(x,y);
}

float Grid::distance(sf::Vector2f first, sf::Vector2f second){
    float x = second.x-first.x;
    float y = second.y-first.y;
    float dist = std::sqrt((x*x + y*y));
    return dist;
}

void Grid::addSource(size_t x, size_t y, float size){

    for(int dx = -size; dx<=size; dx++){
        for (int dy = -size; dy <=size; dy++){


            if(x+dx >0 && x+dx<m_width
            && y+dy>0 && y+dy<m_height){

                float dist = abs(distance(sf::Vector2f(x,y), sf::Vector2f(x+dx,y+dy)));
                if(dist < abs(distance(sf::Vector2f(x,y), sf::Vector2f(x-size,y)))){

                    size_t pos = calcPos(x+dx,y+dy);
                    m_density[pos] = 1;
                }
            }

        }
    }
}



void Grid::diffuse(float dt){

    //5 points diffusion kernel
    //std::pair<std::size_t,std::size_t> xy = std::pair(0,0);
    float left = 0.f;
    float right = 0.f;
    float up = 0.f;
    float down = 0.f;
    float center = 0.f;
    float lap = 0.f;
    float new_dens = 0.f;

    for (std::size_t i = 0; i<m_density.size(); i++){
        //get xy coordinates
       // xy = get_xy(i);
        std::size_t x=i%m_width;
        std::size_t y=i/m_width;

        //checking boundary conditions
        if(x>0 && x < m_width-1 &&
           y>0 && y < m_height-1)
           {
            //getting data for laplacian
            left = m_density_prev[calcPos(x-1,y)];
            right = m_density_prev[calcPos(x+1,y)];
            up = m_density_prev[calcPos(x,y-1)];
            down = m_density_prev[calcPos(x,y+1)];
            center = m_density_prev[i];

            //calculate laplacian
            lap = left+right+up+down - (4*center);

            //calculate and write new density
            new_dens = m_density_prev[i]+ m_diff_co*lap*dt;
            m_density[i] = new_dens;

            }
        else{
            //leave boundary conditions the same
            m_density[i] = m_density_prev[i];
            }
    }
}

void Grid::advect(float dt){

    std::pair<std::size_t,std::size_t> xy = std::pair(0,0);
    float new_dens = 0.f;

    for(std::size_t i=0; i<m_density.size(); i++){
        //index velocity
        float v_vel = m_v_velocity[i];
        float u_vel = m_u_velocity[i];

        //xy values for index
        xy = get_xy(i);
        float x = static_cast<float>(xy.first);
        float y = static_cast<float>(xy.second);

        //backwards location lookup 
        float new_x = x - u_vel*dt;
        float new_y = y - v_vel*dt;

        //finding 4 corners for bilinear interpolation, clamping in boundaries
        std::size_t x_low = std::clamp(std::floor(new_x),2.f,m_width-2*1.f);
        std::size_t x_high = std::clamp(std::ceil(new_x),2.f,m_width-2*1.f);
        std::size_t y_low = std::clamp(std::floor(new_y),2.f,m_height-2*1.f);
        std::size_t y_high = std::clamp(std::ceil(new_y),2.f,m_height-2*1.f);

        //finding values at corners
        float tl = m_density_prev[calcPos(x_low,y_low)];
        float tr = m_density_prev[calcPos(x_high,y_low)];
        float bl = m_density_prev[calcPos(x_low,y_high)];
        float br = m_density_prev[calcPos(x_high,y_high)];

        //distance from edges
        float dx = new_x - x_low;
        float dy = new_y - y_low;

        //weighting calculations
        float tl_weight = (1-dx)*(1-dy);
        float tr_weight = dx*(1-dy);
        float bl_weight = (1-dx)*dy;
        float br_weight = dx*dy;

        new_dens = tl*tl_weight + tr*tr_weight + bl*bl_weight + br*br_weight;
        m_density[i] = new_dens;

    }

}

//decay kernel
void Grid::decay(float dt){
    for( std::size_t i = 0; i<m_density.size(); i++){
        //sample density multiply by decay coefficient
        m_density[i] = m_density_prev[i]*m_decay;
    }
}


