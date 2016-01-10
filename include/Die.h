#define STATE_RISE1		0
#define STATE_RISE2		1
#define STATE_SINK1		2
#define STATE_SINK2		3
#define STATE_UP		4
#define STATE_ROLLOUT	5
#define STATE_ROLLIN	6
#define STATE_SLIDEOUT	7
#define STATE_SLIDEIN	8

#include <nds.h>
#include <stdlib.h>


#include "PlayerLoc.h"

class Space;
class Player;

class Die: public PlayerLoc{
	private:
		unsigned int magic;
		Space* on;
		char state;
		int step;
		int n_up, n_east, n_south;
		m3x3* orientation;
		m3x3* scratch;
		int visited;
		int chain;
		char dir;
		Player* mover;


	public:
		Die(Space *s);
		~Die();
		void draw();
		void tick();
		bool roll(int dir, Player* mover);
		bool push(int dir, Player* mover);
		Space* getSpace();
		char getState();
		char getDir();
		
	private:
		void doRoll();
		void doPush();
		void doSink();
		void rotate(int dir);
		
		void sink(int chain);
		void check();
		int checkR();
		int maxChain();
		
		static void clearVisited();
		static void sinkOnes(Player* moover);
		
		static Die** dice;
		static int numdice;
		static int dicesize;
	
	//Geometry
	private:
		static v16 CubeVectors[];
		static u8 CubeFaces[];
		static u32 uv[];
		static u32 normals[];
};
