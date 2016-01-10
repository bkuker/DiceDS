
class PlayerLoc;
class Space;
class Die;

class Player{
	private:
		unsigned int magic;
		static PlayerLoc allPlayers;
		float x, z, speed;
		char dir;
		void hitEdge( char dir );
		void moveTo( char dir, bool up );
		Space* space;
		Die* die;
		int points;
		
		int fdir;
		int step;
		int slowstep;
	public:
		void locChanged( Space* s, Die* d);
		static void tickPlayers();
		Player(Space* s);
		void setDir(char d);
		void draw();
		void tick();
		float getX();
		float getZ();
		void award(int points);
};
