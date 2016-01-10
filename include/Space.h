#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3
#define NODIR 5

#include <nds.h>
#include <stdlib.h>

#include "PlayerLoc.h"

class Die;
class Board;
class Space;
class Player;

#ifndef Space_h
#define Space_h
class Space : public PlayerLoc{
	private:
		unsigned int magic;
		Space* neighbors[4];
		Die* die;
	public:
	
		Space();
		Space* neighbor(int dir);
		void setNeighbor(int dir, Space *s);
		Die* getDie();
		void setDie(Die *d);
		void draw();
		void tick();
};
#endif
