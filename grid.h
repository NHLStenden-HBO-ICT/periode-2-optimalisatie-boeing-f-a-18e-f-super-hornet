namespace Tmpl8
{
	struct Cell {
		std::vector<Tank*> tanks;
	};

	class Grid {
	public:
		Grid(int width, int height, int cellSize);
		~Grid();

		void addTank(Tank* tank);
		Cell* getCell(int x, int y);
		Cell* getCell(vec2& pos);
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