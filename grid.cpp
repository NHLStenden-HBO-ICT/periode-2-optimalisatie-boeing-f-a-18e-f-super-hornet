#include "precomp.h"

Grid::Grid(int width, int height, int cellSize):
    m_width(width),
    m_height(height),
    m_cellSize(cellSize) {
        m_numXCells = ceil((float)m_width / cellSize);
        m_numYCells = ceil((float)m_height / cellSize);

        //list of cells
        m_cells.resize(m_numYCells * m_numXCells);
    }


Grid::~Grid() = default;

void Grid::add_tank(Tank* tank) {
    Cell* cell = get_cell(tank->position);
    cell->tanks.push_back(tank);
    tank->owner_cell = cell;
    tank->cell_vector_index = cell->tanks.size() - 1;
} 

void Grid::add_tank(Tank* tank, Cell* cell) {
    cell->tanks.push_back(tank);
    tank->owner_cell = cell;
    tank->cell_vector_index = cell->tanks.size() - 1;
}

Cell* Grid::get_cell(int x, int y) {
    if (x < 0) x = 0;
    if (x >= m_numXCells) x = m_numXCells - 1; 
    if (y < 0) y = 0;
    if (y >= m_numYCells) y = m_numYCells - 1;

    return &m_cells[y * m_numXCells + x];
}

Cell* Grid::get_cell(vec2& pos) {
    int cellX = (int)(pos.x / m_cellSize);
    int cellY = (int)(pos.y / m_cellSize);

    return get_cell(cellX, cellY);
}

void Grid::remove_tank_from_cell(Tank* tank) {
    std::vector<Tank*>& tanks = tank->owner_cell->tanks;

    tank->owner_cell->tanks[tank->cell_vector_index] = tank->owner_cell->tanks.back();
    tanks.pop_back();
    if (tank->cell_vector_index < tanks.size()) {
        tanks[tank->cell_vector_index]->cell_vector_index = tank->cell_vector_index;
    }
    tank->cell_vector_index = -1;
    tank->owner_cell = nullptr;
}