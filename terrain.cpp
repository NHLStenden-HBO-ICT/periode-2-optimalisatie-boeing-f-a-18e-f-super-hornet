#include "precomp.h"
#include "terrain.h"

namespace fs = std::filesystem;
namespace Tmpl8
{
    Terrain::Terrain()
    {
        //Load in terrain sprites
        grass_img = std::make_unique<Surface>("assets/tile_grass.png");
        forest_img = std::make_unique<Surface>("assets/tile_forest.png");
        rocks_img = std::make_unique<Surface>("assets/tile_rocks.png");
        mountains_img = std::make_unique<Surface>("assets/tile_mountains.png");
        water_img = std::make_unique<Surface>("assets/tile_water.png");


        tile_grass = std::make_unique<Sprite>(grass_img.get(), 1);
        tile_forest = std::make_unique<Sprite>(forest_img.get(), 1);
        tile_rocks = std::make_unique<Sprite>(rocks_img.get(), 1);
        tile_water = std::make_unique<Sprite>(water_img.get(), 1);
        tile_mountains = std::make_unique<Sprite>(mountains_img.get(), 1);


        //Load terrain layout file and fill grid based on tiletypes
        fs::path terrain_file_path{ "assets/terrain.txt" };
        std::ifstream terrain_file(terrain_file_path);

        if (terrain_file.is_open())
        {
            std::string terrain_line;

            std::getline(terrain_file, terrain_line);
            std::istringstream lineStream(terrain_line);

            int rows;

            lineStream >> rows;

            for (size_t row = 0; row < rows; row++)
            {
                std::getline(terrain_file, terrain_line);

                for (size_t collumn = 0; collumn < terrain_line.size(); collumn++)
                {
                    switch (std::toupper(terrain_line.at(collumn)))
                    {
                    case 'G':
                        tiles.at(row).at(collumn).tile_type = TileType::GRASS;
                        break;
                    case 'F':
                        tiles.at(row).at(collumn).tile_type = TileType::FORREST;
                        break;
                    case 'R':
                        tiles.at(row).at(collumn).tile_type = TileType::ROCKS;
                        break;
                    case 'M':
                        tiles.at(row).at(collumn).tile_type = TileType::MOUNTAINS;
                        break;
                    case 'W':
                        tiles.at(row).at(collumn).tile_type = TileType::WATER;
                        break;
                    default:
                        tiles.at(row).at(collumn).tile_type = TileType::GRASS;
                        break;
                    }
                }
            }
        }
        else
        {
            std::cout << "Could not open terrain file! Is the path correct? Defaulting to grass.." << std::endl;
            std::cout << "Path was: " << terrain_file_path << std::endl;
        }

        //Instantiate tiles for path planning
        for (size_t y = 0; y < tiles.size(); y++)
        {
            for (size_t x = 0; x < tiles.at(y).size(); x++)
            {
                tiles.at(y).at(x).position_x = x;
                tiles.at(y).at(x).position_y = y;

                if (is_accessible(y, x + 1)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x + 1)); }
                if (is_accessible(y, x - 1)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x - 1)); }
                if (is_accessible(y + 1, x)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y + 1).at(x)); }
                if (is_accessible(y - 1, x)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y - 1).at(x)); }
            }
        }
    }

    void Terrain::update()
    {
        //Pretend there is animation code here.. next year :)
    }

    void Terrain::draw(Surface* target) const
    {

        for (size_t y = 0; y < tiles.size(); y++)
        {
            for (size_t x = 0; x < tiles.at(y).size(); x++)
            {
                int posX = (x * sprite_size) + HEALTHBAR_OFFSET;
                int posY = y * sprite_size;

                switch (tiles.at(y).at(x).tile_type)
                {
                case TileType::GRASS:
                    tile_grass->draw(target, posX, posY);
                    break;
                case TileType::FORREST:
                    tile_forest->draw(target, posX, posY);
                    break;
                case TileType::ROCKS:
                    tile_rocks->draw(target, posX, posY);
                    break;
                case TileType::MOUNTAINS:
                    tile_mountains->draw(target, posX, posY);
                    break;
                case TileType::WATER:
                    tile_water->draw(target, posX, posY);
                    break;
                default:
                    tile_grass->draw(target, posX, posY);
                    break;
                }
            }
        }
    }
    //greedy testing
    // ------------------------------------------------------------------------------------------------
    // A helper structure for comparison in priority queue (min-heap)
    struct Node {
        TerrainTile* tile;
        float h_cost; // Only the heuristic cost is considered

        bool operator>(const Node& other) const {
            return h_cost > other.h_cost;
        }
    };
    // Heuristic function: Euclidean distance
    float heuristic(const vec2& a, const vec2& b) {
        return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
    }
    vector<vec2> Terrain::get_route_greedy(const Tank& tank, const vec2& target) {
            // Find start and target tile
            const size_t start_x = tank.position.x / sprite_size;
            const size_t start_y = tank.position.y / sprite_size;
            const size_t target_x = target.x / sprite_size;
            const size_t target_y = target.y / sprite_size;

            TerrainTile* start_tile = &tiles.at(start_y).at(start_x);
            TerrainTile* target_tile = &tiles.at(target_y).at(target_x);

            // Min-heap priority queue for open set
            std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;

            // Push the start tile with its heuristic cost
            float h_start = heuristic(vec2(start_tile->position_x * sprite_size, start_tile->position_y * sprite_size),
                vec2(target_x * sprite_size, target_y * sprite_size));
            open_set.push({ start_tile, h_start });

            // Map to store parent of each tile
            std::unordered_map<TerrainTile*, TerrainTile*> came_from;

            bool route_found = false;
            TerrainTile* current_tile = nullptr;

            // Greedy Best-First Search
            while (!open_set.empty()) {
                Node current_node = open_set.top();
                open_set.pop();
                current_tile = current_node.tile;

                // Check if target is reached
                if (current_tile->position_x == target_x && current_tile->position_y == target_y) {
                    route_found = true;
                    break;
                }

                // Explore neighbors
                for (TerrainTile* neighbor : current_tile->exits) {
                    // Only consider heuristic cost (h_cost), no g_cost
                    float h_cost = heuristic(
                        vec2(neighbor->position_x * sprite_size, neighbor->position_y * sprite_size),
                        vec2(target_x * sprite_size, target_y * sprite_size));

                    // If the neighbor has not been visited, add it to the open set
                    if (came_from.find(neighbor) == came_from.end()) {
                        open_set.push({ neighbor, h_cost });
                        came_from[neighbor] = current_tile;
                    }
                }
            }

            // Reconstruct path if found
            std::vector<vec2> route;
            if (route_found) {
                while (current_tile != start_tile) {
                    route.push_back(vec2((float)current_tile->position_x * sprite_size, (float)current_tile->position_y * sprite_size));
                    current_tile = came_from[current_tile];
                }
                route.push_back(vec2((float)start_tile->position_x * sprite_size, (float)start_tile->position_y * sprite_size));
                std::reverse(route.begin(), route.end());
            }

            return route;
        }
    }
    // ------------------------------------------------------------------------------------------------
    
    //Use Breadth-first search to find shortest route to the destination
    vector<vec2> Terrain::get_route(const Tank& tank, const vec2& target)
    {
        //Find start and target tile
        const size_t pos_x = tank.position.x / sprite_size;
        const size_t pos_y = tank.position.y / sprite_size;

        const size_t target_x = target.x / sprite_size;
        const size_t target_y = target.y / sprite_size;

        //Init queue with start tile
        std::queue<vector<TerrainTile*>> queue;
        queue.emplace();
        queue.back().push_back(&tiles.at(pos_y).at(pos_x));

        std::vector<TerrainTile*> visited;

        bool route_found = false;
        vector<TerrainTile*> current_route;
        while (!queue.empty() && !route_found)
        {
            current_route = queue.front();
            queue.pop();
            TerrainTile* current_tile = current_route.back();

            //Check all exits, if target then done, else if unvisited push a new partial route
            for (TerrainTile * exit : current_tile->exits)
            {
                if (exit->position_x == target_x && exit->position_y == target_y)
                {
                    current_route.push_back(exit);
                    route_found = true;
                    break;
                }
                else if (!exit->visited)
                {
                    exit->visited = true;
                    visited.push_back(exit);
                    queue.push(current_route);
                    queue.back().push_back(exit);
                }
            }
        }

        //Reset tiles
        for (TerrainTile * tile : visited)
        {
            tile->visited = false;
        }

        if (route_found)
        {
            //Convert route to vec2 to prevent dangling pointers
            std::vector<vec2> route;
            for (TerrainTile* tile : current_route)
            {
                route.push_back(vec2((float)tile->position_x * sprite_size, (float)tile->position_y * sprite_size));
            }

            return route;
        }
        else
        {
            return  std::vector<vec2>();
        }

    }

    //TODO: Function not used, convert BFS to dijkstra and take speed into account next year :)
    float Terrain::get_speed_modifier(const vec2& position) const
    {
        const size_t pos_x = position.x / sprite_size;
        const size_t pos_y = position.y / sprite_size;

        switch (tiles.at(pos_y).at(pos_x).tile_type)
        {
        case TileType::GRASS:
            return 1.0f;
            break;
        case TileType::FORREST:
            return 0.5f;
            break;
        case TileType::ROCKS:
            return 0.75f;
            break;
        case TileType::MOUNTAINS:
            return 0.0f;
            break;
        case TileType::WATER:
            return 0.0f;
            break;
        default:
            return 1.0f;
            break;
        }
    }

    bool Terrain::is_accessible(int y, int x)
    {
        //Bounds check
        if ((x >= 0 && x < terrain_width) && (y >= 0 && y < terrain_height))
        {
            //Inaccessible terrain check
            if (tiles.at(y).at(x).tile_type != TileType::MOUNTAINS && tiles.at(y).at(x).tile_type != TileType::WATER)
            {
                return true;
            }
        }

        return false;
    }
