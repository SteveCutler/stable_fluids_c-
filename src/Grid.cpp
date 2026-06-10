#include "../include/Grid.hpp"
#include <vector>


Grid::Grid(std::size_t width, std::size_t height):
m_width(width),
m_height(height),
m_density(width*height, 0.0f),
m_v_velocity(width*height, 0.0f),
m_u_velocity(width*height, 0.0f),
m_density_prev(width*height, 0.0f),
m_v_velocity_prev(width*height, 0.0f),
m_u_velocity_prev(width*height, 0.0f),
m_diff_co(5.0f)
{
    spawn(width , height);
}

const std::vector<float>& Grid::density() const{
    return m_density;
}

void Grid::update(float dt){
    //update loop
    swap();
    diffuse(dt);
}
    


//HELPERS

void Grid::swap(){
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

void Grid::addSource(size_t x, size_t y, int size){

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

void Grid::spawn(std::size_t width, std::size_t height){
    addSource(m_width/2,m_height/2, 50);
    //create density
}

void Grid::diffuse(float dt){
    //5 points diffusion kernel
    std::pair<std::size_t,std::size_t> xy = std::pair(0,0);
    float left = 0.f;
    float right = 0.f;
    float up = 0.f;
    float down = 0.f;
    float center = 0.f;
    float lap = 0.f;
    float new_dens = 0.f;

    for (std::size_t i = 0; i<m_density.size(); i++){
        xy = get_xy(i);
        std::size_t x=xy.first;
        std::size_t y=xy.second;

        if(x>0 && x < m_width-1 &&
           y>0 && y < m_height-1)
           {
            left = m_density_prev[calcPos(x-1,y)];
            right = m_density_prev[calcPos(x+1,y)];
            up = m_density_prev[calcPos(x,y-1)];
            down = m_density_prev[calcPos(x,y+1)];
            center = m_density_prev[i];

            lap = left+right+up+down - (4*center);
            new_dens = m_density_prev[i]+ m_diff_co*lap*dt;
            m_density[i] = new_dens;
            }
        else{
            m_density[i] = m_density_prev[i];
            }
    }
    
    //diffuse
}

void Grid::advect(float dt){
    //advect
}

