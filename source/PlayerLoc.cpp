#include "PlayerLoc.h"
#include <nds.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"

PlayerLoc::PlayerLoc():count(0){
	for ( int i = 0; i < MAX_PLAYERS; i++ )
		players[i] = NULL;
}
	
void PlayerLoc::add(Player* p){
	assert(p);
	//iprintf("Adding player %d\n", p);
	for ( int i = 0; i < count; i++ ){
		if ( players[i] == p ){
			//iprintf( "PlayerLoc: Player already here\n");
			return;
		}
	}
	players[count] = p;
	count++;
}

void PlayerLoc::remove(Player* p){
	assert(p);
	for ( int i = 0; i < count; i++ ){
		if ( players[i] == p ){
			players[i] = players[count-1];
			players[count-1] = NULL;
			count--;
			return;
		}
	}
	//iprintf( "PlayerLoc: Player isn't here\n" );
}

bool PlayerLoc::hasPlayer(Player *p){
	assert(p);
	for ( int i = 0; i < getCount(); i++ )
		if ( players[i] == p )
			return true;
	return false;
}

int PlayerLoc::getCount(){
	return count;
}

Player* PlayerLoc::player(int n){
	if ( n < count && n >= 0 ){
		assert(players[n]); //FAILED
		return players[n];
	}
	iprintf( "PlayerLoc: bad idx (%d) cnt=%d\n", n, count );
	return NULL;
}

