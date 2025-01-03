#pragma once

namespace Tmpl8
{
//forward declarations
class Tank;
class Rocket;
class Smoke;
class Particle_beam;

class Game
{
  public:
    void set_target(Surface* surface) { screen = surface; }
    void init();
    void shutdown();
    void update(float deltaTime);
    void draw();
    void tick(float deltaTime);
    void insertion_sort_tanks_health(const std::vector<Tank>& original, std::vector<const Tank*>& sorted_tanks, int begin, int end);
    void draw_health_bars(const std::vector<const Tank*>& sorted_tanks, const int team);
    void measure_performance();

    void update_grid();
    void calc_tank_route();
    void check_tank_collision();
    void update_tank_collision(Grid* grid);
    void check_tank_collision_grid(Tank* tank, std::vector<Tank*>& tanks_to_check, int starting_index);//start at specific index
    void check_tank_collison_compare_two_tanks(Tank& tank1, Tank& tank2);
    void update_tanks();
    vec2 find_first_active_tank(int& first_active);
    Tank& find_closest_enemy(Tank& current_tank);
    void calculate_convex_hull(vec2 point_on_hull, int first_active);
    void update_rockets();
    void update_rockets_grid(Grid* grid);
    


    void disable_outofbounds_rockets();
    void update_particle_beam();


    void mouse_up(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void mouse_down(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void mouse_move(int x, int y)
    { /* implement if you want to detect mouse movement */
    }

    void key_up(int key)
    { /* implement if you want to handle keys */
    }

    void key_down(int key)
    { /* implement if you want to handle keys */
    }

  private:
    Surface* screen;
    std::unique_ptr<Grid> m_grid; //spatial partioning

    vector<Tank> tanks; //maybe need to be pointers for sptial partioning
    vector<Rocket> rockets;
    vector<Smoke> smokes;
    vector<Explosion> explosions;
    vector<Particle_beam> particle_beams;

    Terrain background_terrain;
    std::vector<vec2> forcefield_hull;

    Font* frame_count_font;
    long long frame_count = 0;

    bool lock_update = false;

    //Checks if a point lies on the left of an arbitrary angled line
    bool left_of_line(vec2 line_start, vec2 line_end, vec2 point);
};

}; // namespace Tmpl8