
class Player;
class PlayerLoc;

#ifndef player_loc_h
#define player_loc_h
#define MAX_PLAYERS 16

class PlayerLoc{
	private:
		int count;
		Player* players[MAX_PLAYERS];
	public:
		PlayerLoc();
		void add(Player* p);
		void remove(Player* p);
		bool hasPlayer(Player* p);
		int getCount();
		Player* player(int n);
};
#endif


