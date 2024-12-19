namespace Tmpl8
{
	struct Cell {
		std::vector<Tank*> tanks;
		std::vector<Rocket*> rockets;

	};

	class Grid {
		friend class Game;
	public:
		Grid(int width, int height, int cellSize);
		~Grid();

		void add_tank(Tank* tank);// add tank to cell and compute what cell contains it
		void add_tank(Tank* tank,Cell* cell);// add tank but we already know where it is
		void remove_tank_from_cell(Tank* tank);
		void add_rocket(Rocket* rocket);// add tank to cell and compute what cell contains it
		void add_rocket(Rocket* rocket, Cell* cell);// add tank but we already know where it is
		void remove_rocket_from_cell(Rocket* rocket);

		Cell* getCell(int x, int y);
		Cell* getCell(vec2& pos);
		



	private:
		std::vector<Cell> m_cells;
		int m_cellSize;
		int m_width;
		int m_height;
		int m_numXCells;
		int m_numYCells;
	};
}