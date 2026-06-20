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
m_vel_decay(0.9f),
m_viscosity(0.01f),
m_pressure_iter(20),
m_advectVel_ms(0.f),
m_project_ms(0.f),
m_addSource_ms(0.f),
m_render_ms(0.f),
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

float Grid::time_advectVel() const{
    return m_advectVel_ms;
};

float Grid::time_project() const{
    return m_project_ms;
};

float Grid::time_addSource() const{
    return m_addSource_ms;
};


//UPDATE LOOP

void Grid::update(float dt){
    //Create noise scalar field
    m_performance_clock.restart();
    calcNoise(dt*20);
    m_noise_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
    
    
    //Create vel gradient field based on noise
    m_performance_clock.restart();
    calcVel(dt);
    m_vel_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
    
    // apply velocity boundary calculations
    velBoundaries();

    //first pressure solve
   // projectStep();

    //Advect velocity
    swapVel();
    m_performance_clock.restart();
    advectVel(dt);
    m_advectVel_ms = m_performance_clock.restart().asMicroseconds()/1000.f;

    //apply velocity boundary conditions
    //velBoundaries();
    
    
    //Diffuse velocity
    // swapVel();
    // diffuseVel(dt*.1);  
    velBoundaries();

    //second pressure solve
    m_performance_clock.restart();
    projectStep();
    m_project_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
    decayVel();  

    //Add density source
    m_performance_clock.restart();
    addSource(m_width/2,m_height/2, m_source);
    m_addSource_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
    
    //Diffuse
    swapDensity();
    m_performance_clock.restart();
    diffuse(dt*5);
    m_diffuse_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
    
    //Advect
    swapDensity();
    m_performance_clock.restart();
    advect_decay(dt);
    m_advect_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
    
}
    


//HELPERS

void Grid::projectStep(){
    //clearPressure();
    m_performance_clock.restart();
    calcDivergence();
    m_div_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
    
    
    m_performance_clock.restart();
    solvePressure(1, m_height-1);
    m_pressure_ms = m_performance_clock.restart().asMicroseconds()/1000.f;
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
    for(std::size_t y = 1; y<m_height-1; y++){
        std::size_t row = y*m_width;

         for(std::size_t x = 1; x<m_width-1; x++){
            std::size_t index = row+x;



                float xl = m_noise[index-1];
                float xr = m_noise[index+1];
                float yt = m_noise[index-m_width];
                float yb = m_noise[index+m_width];

                float dx = (xr-xl)/2;
                float dy = (yb-yt)/2;

                m_u_velocity[index] += -dy*m_curl_mult*dt;
                m_v_velocity[index] += dx*m_curl_mult*dt;
               
            

        }
    }
}


void Grid::swapDensity(){
    std::swap(m_density, m_density_prev);
}


void Grid::addSource(size_t x, size_t y, float size){

    float radius = size;
    for(int dy = -size; dy<=size; dy++){
        //new y
        std::size_t new_y = (y+dy);
        std::size_t row = new_y*m_width;
        
        for (int dx = -size; dx <=size; dx++){
            //new_x
            std::size_t new_x = x+dx;


            if(new_x>1 && new_x<m_width-1
            && new_y>1 && new_y<m_height-1){

                
                float dist = (dx*dx) + (dy*dy);
                float rad_sqr = size*size;
                
                if(dist < rad_sqr){

                    size_t pos = new_x+row;
                    m_density[pos] = 1;
                }
            }

        }
    }
}


//DYNAMIC VEL FUNCTIONS

void Grid::advectVel(float dt){
    float new_u_vel = 0.f;
    float new_v_vel = 0.f;

    //switch to row first y x loop
    for(std::size_t y = 0; y<m_height; y++){
        
        //reducing multiplication calculations by doing it once per row
        std::size_t row = y*m_width;

        for(std::size_t x = 0; x<m_width; x++){

        //addition calculations are cheap for CPU
        std::size_t i = row + x;
    
        //index velocity
        float old_u_vel = m_u_velocity_prev[i];
        float old_v_vel = m_v_velocity_prev[i];

        //removed modulo and multiplicatin operations happening for every cell

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
        std::size_t tl_index = x_floor+y_floor*m_width;
        std::size_t tr_index = x_high+y_floor*m_width;
        std::size_t bl_index = x_floor+y_high*m_width;
        std::size_t br_index = x_high+y_high*m_width;

        float tl_u = m_u_velocity_prev[tl_index];
        float tr_u = m_u_velocity_prev[tr_index];
        float bl_u = m_u_velocity_prev[bl_index];
        float br_u = m_u_velocity_prev[br_index];
        
        float tl_v = m_v_velocity_prev[tl_index];
        float tr_v = m_v_velocity_prev[tr_index];
        float bl_v = m_v_velocity_prev[bl_index];
        float br_v = m_v_velocity_prev[br_index];

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

    for(std::size_t y = 1; y<m_height-1; y++){
        //minimize mult operations
        std::size_t row = y*m_width;

        for(std::size_t x = 1; x<m_width-1; x++){
            //calculate index
            std::size_t i = row+x; 



                //calc corner indices

                std::size_t l_index = i-1;
                std::size_t r_index = i+1;
                std::size_t u_index = i-m_width;
                std::size_t d_index = i+m_width;

                //getting data for laplacian
                left_u = m_u_velocity_prev[l_index];
                right_u = m_u_velocity_prev[r_index];
                up_u = m_u_velocity_prev[u_index];
                down_u = m_u_velocity_prev[d_index];
                center_u = m_u_velocity_prev[i];

                left_v = m_v_velocity_prev[l_index];
                right_v = m_v_velocity_prev[r_index];
                up_v = m_v_velocity_prev[u_index];
                down_v = m_v_velocity_prev[d_index];
                center_v = m_v_velocity_prev[i];

                //calculate laplacian
                lap_u = left_u+right_u+up_u+down_u - (4*center_u);
                lap_v = left_v+right_v+up_v+down_v - (4*center_v);

                //calculate and write new density
                new_u_vel = m_u_velocity_prev[i]+ m_viscosity*lap_u*dt;
                new_v_vel = m_v_velocity_prev[i]+ m_viscosity*lap_v*dt;

                m_u_velocity[i] = new_u_vel;
                m_v_velocity[i] = new_v_vel;

                

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
    
    for(std::size_t y =1; y<m_height-1; y++){
        //calc row once per line
        std::size_t row = y*m_width;

            for(std::size_t x = 1; x<m_width-1; x++){


                    std::size_t index = row+x;

                    float xl = m_u_velocity[index-1];
                    float xr = m_u_velocity[index+1];
                    float yt = m_v_velocity[index-m_width];
                    float yb = m_v_velocity[index+m_width];

                    float div =  ((xr-xl)/2)+((yb-yt)/2);

                    max_div = std::max(max_div, abs(div));

                    m_divergence[index] = div;
                
                
            }
        }
    //    std::cout << "max divergence: " << max_div << "\n";
};

void Grid::project(){
    
    for(std::size_t y = 1; y<m_height-1; y++){
        std::size_t row = y*m_width;
        
        for(std::size_t x = 1; x<m_width-1; x++){
            
            
            std::size_t index = x+row;
            
            float xl = m_pressure[index-1];
            float xr = m_pressure[index+1];
            float yt = m_pressure[index-m_width];
            float yb = m_pressure[index+m_width];
            
            float pres_x =  (xr-xl)/2.f;
            float pres_y =  (yb-yt)/2.f;
            
            m_u_velocity[index] -= pres_x;
            m_v_velocity[index] -= pres_y;
            
            
        }
    }
    
};


void Grid::solvePressure(std::size_t begin, std::size_t end){

    std::size_t k = 0;
    while(k<m_pressure_iter){

        for(std::size_t y = begin; y<end; y++){

            //calc row once per line
            std::size_t row = y*m_width;

                for(std::size_t x = 1; x<m_width-1; x++){



                        std::size_t index = x+row;

                        float xl = m_pressure_prev[index-1];
                        float xr = m_pressure_prev[index+1];
                        float yt = m_pressure_prev[index-m_width];
                        float yb = m_pressure_prev[index+m_width];

                        float pressure =  (xl+xr+yt+yb - m_divergence[index])*0.25f;

                        m_pressure[index] = pressure;
                    
                    
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

    for(std::size_t y = 1; y<m_height-1; y++){

        std::size_t row = y*m_width;

        for(std::size_t x = 1; x<m_width-1; x++){

            std::size_t index = row+x;


                //getting data for laplacian
                left = m_density_prev[index-1];
                right = m_density_prev[index+1];
                up = m_density_prev[index-m_width];
                down = m_density_prev[index+m_width];
                center = m_density_prev[index];

                //calculate laplacian
                lap = left+right+up+down - (4*center);

                //calculate and write new density
                new_dens = m_density_prev[index]+ m_diff_co*lap*dt;
                m_density[index] = new_dens;

                

            }
        }
}

void Grid::advect_decay(float dt){


    float new_dens = 0.f;

    for(std::size_t y = 0; y<m_height; y++){
        std::size_t row = y*m_width;
        for(std::size_t x = 0; x<m_width; x++){
            std::size_t i =row+x;
            //index velocity
            float v_vel = m_v_velocity[i];
            float u_vel = m_u_velocity[i];


            //backwards location lookup 
            float new_x = std::clamp((x - u_vel*dt),1.f,m_width-2.f);
            float new_y = std::clamp((y - v_vel*dt),1.f,m_height-2.f);

            //finding 4 corners for bilinear interpolation, clamping in boundaries
            std::size_t x_low = std::floor(new_x);
            std::size_t x_high = x_low+1;
            std::size_t y_low =std::floor(new_y);
            std::size_t y_high = y_low+1;

            //finding values at corners
            float tl = m_density_prev[x_low+y_low*m_width];
            float tr = m_density_prev[x_high+y_low*m_width];
            float bl = m_density_prev[x_low+y_high*m_width];
            float br = m_density_prev[x_high+y_high*m_width];

            //distance from edges
            float dx = new_x - x_low;
            float dy = new_y - y_low;

            //weighting calculations
            float tl_weight = (1-dx)*(1-dy);
            float tr_weight = dx*(1-dy);
            float bl_weight = (1-dx)*dy;
            float br_weight = dx*dy;

            new_dens = tl*tl_weight + tr*tr_weight + bl*bl_weight + br*br_weight;

            //included decay here to minimize operations
            m_density[i] = new_dens*m_decay;
        }

    }

}



