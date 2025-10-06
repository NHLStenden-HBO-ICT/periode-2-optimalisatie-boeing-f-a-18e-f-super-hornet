namespace Tmpl8
{
	struct Cell {
		std::vector<Tank*> tanks;
	};

	class Grid {
		friend class Game;
	public:
		Grid(int width, int height, int cellSize);
		~Grid();

		void add_tank(Tank* tank);// add tank to cell and compute what cell contains it
		void add_tank(Tank* tank,Cell* cell);// add tank but we already know where it is
		Cell* get_cell(int x, int y);
		Cell* get_cell(vec2& pos);
		void remove_tank_from_cell(Tank* tank);
	private:
		std::vector<Cell> m_cells;
		int m_cellSize;
		int m_width;
		int m_height;
		int m_numXCells;
		int m_numYCells;
	};
}