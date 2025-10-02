#include "precomp.h" // include (only) this in every .cpp file
#include <iostream>
constexpr auto num_tanks_blue = 2048; //2048
constexpr auto num_tanks_red = 2048;

constexpr auto tank_max_health = 1000;
constexpr auto rocket_hit_value = 60;
constexpr auto particle_beam_hit_value = 50;

constexpr auto tank_max_speed = 1.0;

constexpr auto health_bar_width = 70;

constexpr auto max_frames = 2000;

//Global performance timer
constexpr auto REF_PERFORMANCE = 606798; //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames) MEASURED ON LAPTOP WITHOUT CHARGER 
//81755.2 measured on 02-10-2025 after optimizing tanks and rockets collision
static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static Surface* tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface* tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface* rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface* rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface* particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface* smoke_img = new Surface("assets/Smoke.png");
static Surface* explosion_img = new Surface("assets/Explosion.png");

static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2 tank_size(7, 9);
const static vec2 rocket_size(6, 6);

const static float tank_radius = 3.f;
const static float rocket_radius = 5.f;

// -----------------------------------------------------------
// Initialize the simulation state
// This function does not count for the performance multiplier
// (Feel free to optimize anyway though ;) )
// -----------------------------------------------------------
void Game::init()
{
    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

    //grid
    const int cell_size = 20;
    const int height = 720;
    const int width = 1280;
    m_grid = std::make_unique<Grid>(width, height, cell_size);


    tanks.reserve(num_tanks_blue + num_tanks_red);

    uint max_rows = 24;

    float start_blue_x = tank_size.x + 40.0f;
    float start_blue_y = tank_size.y + 30.0f;

    float start_red_x = 1088.0f;
    float start_red_y = tank_size.y + 30.0f;

    float spacing = 7.5f;

    //Spawn blue tanks
    for (int i = 0; i < num_tanks_blue; i++)
    {
        vec2 position{ start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, BLUE, &tank_blue, &smoke, 1100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
        //add tank to grid
        m_grid->addTank(&tanks.back());
    }
    //Spawn red tanks
    for (int i = 0; i < num_tanks_red; i++)
    {
        vec2 position{ start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, RED, &tank_red, &smoke, 100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
        //add tank to grid
        m_grid->addTank(&tanks.back());
    }

    particle_beams.push_back(Particle_beam(vec2(590, 327), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    particle_beams.push_back(Particle_beam(vec2(64, 64), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::shutdown()
{
}



// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------
Tank& Game::find_closest_enemy(Tank& current_tank)
{
    float closest_distance = numeric_limits<float>::infinity();
    int closest_index = 0;

    for (int i = 0; i < tanks.size(); i++)
    {
        if (tanks.at(i).allignment != current_tank.allignment && tanks.at(i).active)
        {
            float sqr_dist = fabsf((tanks.at(i).get_position() - current_tank.get_position()).sqr_length());
            if (sqr_dist < closest_distance)
            {
                closest_distance = sqr_dist;
                closest_index = i;
            }
        }
    }

    return tanks.at(closest_index);
}

int leftiterator = 0;
//Checks if a point lies on the left of an arbitrary angled line
bool Game::left_of_line(vec2 line_start, vec2 line_end, vec2 point)
{
    bool answer =  ((line_end.x - line_start.x) * (point.y - line_start.y) - (line_end.y - line_start.y) * (point.x - line_start.x)) < 0;
    return answer;
}

//------------------------------------------------------------ segmentated functions from update, might get own files later.
void Game::calc_tank_route(){
    if (frame_count == 0)
    {
        for (Tank& t : tanks)
        {
            t.set_route(background_terrain.get_route_greedy(t, t.target));
        }
    }
}


void Game::check_tank_collision() {
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            for (Tank& other_tank : tanks)
            {
                if (&tank == &other_tank || !other_tank.active) continue;

                vec2 dir = tank.get_position() - other_tank.get_position();
                float dir_squared_len = dir.sqr_length();

                float col_squared_len = (tank.get_collision_radius() + other_tank.get_collision_radius());
                col_squared_len *= col_squared_len;

                if (dir_squared_len < col_squared_len)
                {
                    tank.push(dir.normalized(), 1.f);
                }
            }
        }
    }
}
//----------------------------------------------------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^---------------------------------------------------------
//                         these 2 buggers above seem to be taking up pretty much all the computation time, making them the priority
//                         cause 1: BFS instead of A*, cause 2: *investigating*
//----------------------------------------------------------------------------------------------------------------------------------------------------
void Game::update_tank_collision(Grid* grid) {
    for (int i = 0; i < grid->m_cells.size(); i++) {
        int x = i % grid->m_numXCells;
        int y = i / grid->m_numXCells;

        Cell& cell = grid->m_cells[i];

        for (int j = 0; j < tanks.size(); j++) { //loop through the tanks in a given cell
            if (j >= cell.tanks.size()) break; // Safe guard to ensure j is in range
            Tank* tank = cell.tanks[j];
            //std::cout << "checking " << j << std::endl;
            //std::cout << "size " << cell.tanks.size() << std::endl;
            check_tank_collision_grid(tank, cell.tanks, j+1);
            //std::cout << "succesfully got " << j << std::endl;
            if (x > 0) {
                // Left
                check_tank_collision_grid(tank, grid->getCell(x - 1, y)->tanks, 0);
                if (y > 0) {
                    /// Top left
                    check_tank_collision_grid(tank, grid->getCell(x - 1, y - 1)->tanks, 0);
                }
                if (y < grid->m_numYCells - 1) {
                    // Bottom left
                    check_tank_collision_grid(tank, grid->getCell(x - 1, y + 1)->tanks, 0);
                }
            }
            // Up cell
            if (y > 0) {
                check_tank_collision_grid(tank, grid->getCell(x, y - 1)->tanks, 0);
            }
        }
    }
}    
void Game::check_tank_collision_grid(Tank* tank, std::vector<Tank*>& tanks_to_check, int starting_index) {
    for (int i = starting_index;i < tanks_to_check.size();i++) {
        //std::cout << "col checking for " << i << " " << tanks_to_check.size() << std::endl;
        
        check_tank_collison_compare_two_tanks(*tank, *tanks_to_check[i]);
        //std::cout << "col succesfully got " << i << std::endl;
    } 
}

void Game::check_tank_collison_compare_two_tanks(Tank& tank1, Tank& tank2) {
    // Skip if either tank is inactive
    if (!tank1.active || !tank2.active) return;

    // Calculate the direction vector between the two tanks
    vec2 dir = tank1.get_position() - tank2.get_position();
    float dir_squared_len = dir.sqr_length();

    // Calculate the squared sum of their collision radii
    float col_squared_len = (tank1.get_collision_radius() + tank2.get_collision_radius());
    col_squared_len *= col_squared_len;

    // Check if the tanks are colliding
    if (dir_squared_len < col_squared_len) {
        // Resolve the collision by pushing tank1 away from tank2
        tank1.push(dir.normalized(), 1.f);
    }
}

void Game::update_tanks() {
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            //Move tanks according to speed and nudges (see above) also reload
            tank.tick(background_terrain);

            //Shoot at closest target if reloaded
            if (tank.rocket_reloaded())
            {
                Tank& target = find_closest_enemy(tank);

                rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

                tank.reload_rocket();
            }
        }
    }
}

//notes and changes:
//- no more incrementing first_active every loop, only when found
//- O(n) complexity
vec2 Game::find_first_active_tank(int& first_active) {
    first_active = 0;
    for (size_t i = 0; i < tanks.size(); ++i) {
        if (tanks[i].active) {
            first_active = i;
            return tanks[i].position; //point_on_hull
        }
    }
}


void Game::calculate_convex_hull(vec2 point_on_hull,int first_active) {
    while (true)
    {
        //Add last found point
        forcefield_hull.push_back(point_on_hull);

        //Loop through all points replacing the endpoint with the current iteration every time 
        //it lies left of the current segment formed by point_on_hull and the current endpoint.
        //By the end we have a segment with no points on the left and thus a point on the convex hull.
        vec2 endpoint = tanks.at(first_active).position;
        for (Tank& tank : tanks)
        {
            if (tank.active)
            {
                if ((endpoint == point_on_hull) || left_of_line(point_on_hull, endpoint, tank.position))
                {
                    endpoint = tank.position;
                }
            }
        }

        //Set the starting point of the next segment to the found endpoint.
        point_on_hull = endpoint;

        //If we went all the way around we are done.
        if (endpoint == forcefield_hull.at(0))
        {
            break;
        }
    }
}
//-----------------------------------------------------------
void Game::update_rockets_grid(Grid* grid) {
    for (Rocket& rocket : rockets)
    {
        rocket.tick();
        if (!rocket.active) continue;

        // Compute the rocket’s cell indices directly
        int cellX = (int)(rocket.position.x / grid->m_cellSize);
        int cellY = (int)(rocket.position.y / grid->m_cellSize);

        bool hit = false;

        // Scan current cell + 8 neighbors
        for (int dx = -1; dx <= 1 && !hit; dx++) {
            for (int dy = -1; dy <= 1 && !hit; dy++) {
                int nx = cellX + dx;
                int ny = cellY + dy;

                // Bounds check
                if (nx < 0 || nx >= grid->m_numXCells ||
                    ny < 0 || ny >= grid->m_numYCells) {
                    continue;
                }

                // Get neighbor cell
                Cell* cell = grid->getCell(nx, ny);

                // Check all tanks in this cell
                for (Tank* tank : cell->tanks) {
                    if (tank->active &&
                        (tank->allignment != rocket.allignment) &&
                        rocket.intersects(tank->position, tank->collision_radius))
                    {
                        explosions.push_back(Explosion(&explosion, tank->position));

                        if (tank->hit(rocket_hit_value)) {
                            smokes.push_back(Smoke(smoke, tank->position - vec2(7, 24)));
                        }

                        rocket.active = false;
                        hit = true;
                        break;
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------



void Game::update_rockets() {
    for (Rocket& rocket : rockets)
    {
        rocket.tick();

        //Check if rocket collides with enemy tank, spawn explosion, and if tank is destroyed spawn a smoke plume
        for (Tank& tank : tanks)
        {
            if (tank.active && (tank.allignment != rocket.allignment) && rocket.intersects(tank.position, tank.collision_radius))
            {
                explosions.push_back(Explosion(&explosion, tank.position));

                if (tank.hit(rocket_hit_value))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(7, 24)));
                }

                rocket.active = false;
                break;
            }
        }
    }
}

void Game::disable_outofbounds_rockets() {
    //Hint: A point to convex hull intersection test might be better here? :) (Disable if outside)
    for (Rocket& rocket : rockets)
    {
        if (rocket.active)
        {
            for (size_t i = 0; i < forcefield_hull.size(); i++)
            {
                if (circle_segment_intersect(forcefield_hull.at(i), forcefield_hull.at((i + 1) % forcefield_hull.size()), rocket.position, rocket.collision_radius))
                {
                    explosions.push_back(Explosion(&explosion, rocket.position));
                    rocket.active = false;
                }
            }
        }
    }
}
void Game::update_particle_beam() {
    for (Particle_beam& particle_beam : particle_beams)
    {
        particle_beam.tick(tanks);

        //Damage all tanks within the damage window of the beam (the window is an axis-aligned bounding box)
        for (Tank& tank : tanks)
        {
            if (tank.active && particle_beam.rectangle.intersects_circle(tank.get_position(), tank.get_collision_radius()))
            {
                if (tank.hit(particle_beam.damage))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
                }
            }
        }
    }
}
void Game::update_grid() {
    for (size_t i = 0; i < tanks.size(); i++) {
        Tank& tank = tanks[i];

        // Get the new cell based on the tank's current position
        Cell* newCell = m_grid->getCell(tank.position);

        // Check if the tank has moved to a different cell
        if (newCell != tank.owner_cell) {
            // Remove the tank from its old cell
            if (tank.owner_cell) {
                m_grid->remove_tank_from_cell(&tank);
            }

            // Add the tank to the new cell
            m_grid->addTank(&tank, newCell);

            // Update the tank's owner cell to the new cell
            tank.owner_cell = newCell;
        }
    }
}

// -----------------------------------------------------------
// Update the game state:
// update grid positions
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------
void Game::update(float deltaTime)
{
    //Calculate the route to the destination for each tank using BFS
    //Initializing routes here so it gets counted for performance..
    calc_tank_route();

    //Check tank collision and nudge tanks away from each other
    //check_tank_collision();
    update_tank_collision(m_grid.get());
    //Update tanks
    update_tanks();
    update_grid();
    //Update smoke plumes
    for (Smoke& smoke : smokes)
    {
        smoke.tick();
    }

    //Calculate "forcefield" around active tanks
    forcefield_hull.clear();

    //Find first active tank (this loop is a bit disgusting, fix?)
    int first_active;
    vec2 point_on_hull = find_first_active_tank(first_active);

    //Find left most tank position
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            if (tank.position.x <= point_on_hull.x)
            {
                point_on_hull = tank.position;
            }
        }
    }


    //Calculate convex hull for 'rocket barrier'
    calculate_convex_hull(point_on_hull, first_active);

        //Update rockets
    update_rockets_grid(m_grid.get());
    //update_rockets();

     //Disable rockets if they collide with the "forcefield"
     disable_outofbounds_rockets();
 
     //Remove exploded rockets with remove erase idiom
     rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }), rockets.end());

     //Update particle beams
     update_particle_beam();
 
    //Update explosion sprites and remove when done with remove erase idiom
    for (Explosion& explosion : explosions)
    {
        explosion.tick();
    }

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());
}


// -----------------------------------------------------------
// Draw all sprites to the screen
// (It is not recommended to multi-thread this function)
// -----------------------------------------------------------
void Game::draw()
{
    // clear the graphics window
    screen->clear(0);

    //Draw background
    background_terrain.draw(screen);

    //Draw sprites
    for (int i = 0; i < num_tanks_blue + num_tanks_red; i++)
    {
        tanks.at(i).draw(screen);

        vec2 tank_pos = tanks.at(i).get_position();
    }

    for (Rocket& rocket : rockets)
    {
        rocket.draw(screen);
    }

    for (Smoke& smoke : smokes)
    {
        smoke.draw(screen);
    }

    for (Particle_beam& particle_beam : particle_beams)
    {
        particle_beam.draw(screen);
    }

    for (Explosion& explosion : explosions)
    {
        explosion.draw(screen);
    }

    //Draw forcefield (mostly for debugging, its kinda ugly..)
    for (size_t i = 0; i < forcefield_hull.size(); i++)
    {
        vec2 line_start = forcefield_hull.at(i);
        vec2 line_end = forcefield_hull.at((i + 1) % forcefield_hull.size());
        line_start.x += HEALTHBAR_OFFSET;
        line_end.x += HEALTHBAR_OFFSET;
        screen->line(line_start, line_end, 0x0000ff);
    }

    //Draw sorted health bars
    for (int t = 0; t < 2; t++)
    {
        const int NUM_TANKS = ((t < 1) ? num_tanks_blue : num_tanks_red);

        const int begin = ((t < 1) ? 0 : num_tanks_blue);
        std::vector<const Tank*> sorted_tanks;
        insertion_sort_tanks_health(tanks, sorted_tanks, begin, begin + NUM_TANKS);
        sorted_tanks.erase(std::remove_if(sorted_tanks.begin(), sorted_tanks.end(), [](const Tank* tank) { return !tank->active; }), sorted_tanks.end());

        draw_health_bars(sorted_tanks, t);
    }
}

// -----------------------------------------------------------
// Sort tanks by health value using insertion sort
// -----------------------------------------------------------
void Tmpl8::Game::insertion_sort_tanks_health(const std::vector<Tank>& original, std::vector<const Tank*>& sorted_tanks, int begin, int end)
{
    const int NUM_TANKS = end - begin;
    sorted_tanks.reserve(NUM_TANKS);
    sorted_tanks.emplace_back(&original.at(begin));

    for (int i = begin + 1; i < (begin + NUM_TANKS); i++)
    {
        const Tank& current_tank = original.at(i);

        for (int s = (int)sorted_tanks.size() - 1; s >= 0; s--)
        {
            const Tank* current_checking_tank = sorted_tanks.at(s);

            if ((current_checking_tank->compare_health(current_tank) <= 0))
            {
                sorted_tanks.insert(1 + sorted_tanks.begin() + s, &current_tank);
                break;
            }

            if (s == 0)
            {
                sorted_tanks.insert(sorted_tanks.begin(), &current_tank);
                break;
            }
        }
    }
}

// -----------------------------------------------------------
// Draw the health bars based on the given tanks health values
// -----------------------------------------------------------
void Tmpl8::Game::draw_health_bars(const std::vector<const Tank*>& sorted_tanks, const int team)
{
    int health_bar_start_x = (team < 1) ? 0 : (SCRWIDTH - HEALTHBAR_OFFSET) - 1;
    int health_bar_end_x = (team < 1) ? health_bar_width : health_bar_start_x + health_bar_width - 1;

    for (int i = 0; i < SCRHEIGHT - 1; i++)
    {
        //Health bars are 1 pixel each
        int health_bar_start_y = i * 1;
        int health_bar_end_y = health_bar_start_y + 1;

        screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
    }

    //Draw the <SCRHEIGHT> least healthy tank health bars
    int draw_count = std::min(SCRHEIGHT, (int)sorted_tanks.size());
    for (int i = 0; i < draw_count - 1; i++)
    {
        //Health bars are 1 pixel each
        int health_bar_start_y = i * 1;
        int health_bar_end_y = health_bar_start_y + 1;

        float health_fraction = (1 - ((double)sorted_tanks.at(i)->health / (double)tank_max_health));

        if (team == 0) { screen->bar(health_bar_start_x + (int)((double)health_bar_width * health_fraction), health_bar_start_y, health_bar_end_x, health_bar_end_y, GREENMASK); }
        else { screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x - (int)((double)health_bar_width * health_fraction), health_bar_end_y, GREENMASK); }
    }
}

// -----------------------------------------------------------
// When we reach max_frames print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void Tmpl8::Game::measure_performance()
{
    char buffer[128];
    if (frame_count >= max_frames)
    {
        if (!lock_update)
        {
            duration = perf_timer.elapsed();
            cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
            lock_update = true;
        }

        frame_count--;
    }

    if (lock_update)
    {
        screen->bar(420 + HEALTHBAR_OFFSET, 170, 870 + HEALTHBAR_OFFSET, 430, 0x030000);
        int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
        sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
        frame_count_font->centre(screen, buffer, 200);
        sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
        frame_count_font->centre(screen, buffer, 340);
    }
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::tick(float deltaTime)
{
    if (!lock_update)
    {
        update(deltaTime);
    }
    draw();

    measure_performance();

    // print something in the graphics window
    //screen->Print("hello world", 2, 2, 0xffffff);

    // print something to the text window
    //cout << "This goes to the console window." << std::endl;

    //Print frame count
    frame_count++;
    //std::cout << std::to_string(frame_count) << std::endl;
    string frame_count_string = "FRAME: " + std::to_string(frame_count);
    frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}
