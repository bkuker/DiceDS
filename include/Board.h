#include "Space.h"

class Board{

	public:
		Board(int sz);
		~Board();
		void draw();
		void tick();
		Space* space(int x, int y);
		int dieCount();
		float fullness();
		bool full();
	private:
		void newDie();
		int size;
		Space** spaces;
};
