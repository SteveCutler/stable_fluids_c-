#include "../include/Grid.hpp"
#include <vector>


Grid::Grid(std::size_t width, std::size_t height):
m_width(width),
m_height(height),
m_density(width*height, 0.0f),
m_v_velocity(width*height, 10.0f),
m_u_velocity(width*height, 0.0f),
m_density_prev(width*height, 0.0f),
m_v_velocity_prev(width*height, 0.0f),
m_u_velocity_prev(width*height, 0.0f),
m_diff_co(5.0f),
m_decay(0.99f),
m_source(50.f)
{
  // spawn(width , height);
}

const std::vector<float>& Grid::density() const{
    return m_density;
}

void Grid::update(float dt){
    //update loop
    addSource(m_width/2,m_height/2, m_source);
    swap();
    diffuse(dt);
    swap();
    advect(dt);
    swap();
    decay(dt);
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
    std::pair<std::size_t,std::size_t> xy = std::pair(0,0);
    float left = 0.f;
    float right = 0.f;
    float up = 0.f;
    float down = 0.f;
    float center = 0.f;
    float lap = 0.f;
    float new_dens = 0.f;

    for (std::size_t i = 0; i<m_density.size(); i++){
        //get xy coordinates
        xy = get_xy(i);
        std::size_t x=xy.first;
        std::size_t y=xy.second;

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

