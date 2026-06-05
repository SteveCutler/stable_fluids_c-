#include "../include/Grid.hpp"
#include <vector>


Grid::Grid(std::size_t width, std::size_t height):
m_width(width),
m_height(height),
m_density(width*height, 0.0f)
{
    spawn(width , height);
}

const std::vector<float>& Grid::density() const{
    return m_density;
}

void Grid::update(float dt){
    //update loop
}
    


//HELPERS

size_t Grid::calcPos(size_t x, size_t y){
    size_t pos = m_width*y+x;
    return pos;
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

void Grid::diffuse(){
    //∂d/∂t​=D∇^2*d
    
    //diffuse
}

void Grid::advect(){
    //advect
}



//GLOBAL VARIABLES
int m_width;
int m_height;

// SoA member variables
std::vector<float> m_density;
std::vector<float> m_u_velocity;
std::vector<float> m_v_velocity;

std::vector<float>m_density_prev;
std::vector<float>m_u_velocity_prev;
std::vector<float>m_v_velocity_prev;

;
