#include "Board.h"
#include <stdio.h>
#include "Die.h"
#include "assert.h"

Board::Board(int sz):
	size(sz)
{
	//iprintf("Creating %d x %d Board\n", sz, sz);
	spaces = new Space*[size*size];
	for ( int x = 0; x < size; x++){
		for( int y = 0; y < size; y++ ){
			spaces[x+size*y] = new Space();
		}
	}
	//iprintf("Linking spaces...\n");
	for ( int x = 0; x < size; x++){
		for( int y = 0; y < size; y++ ){
			space(x,y)->setNeighbor(SOUTH, space(x, y + 1));
			space(x,y)->setNeighbor(NORTH, space(x, y - 1));
			space(x,y)->setNeighbor(EAST, space(x + 1, y));
			space(x,y)->setNeighbor(WEST, space(x - 1, y));
		}
	}

	for ( int i = 0; i < (sz*sz)/4; i++)
		newDie();
	for ( int i = 0; i < 300; i++ )
		tick();

	//iprintf("done\n");
}

Board::~Board(){
	for ( int x = 0; x < size; x++){
		for( int y = 0; y < size; y++ ){
			delete(space(x,y));
		}
	}
}

void Board::newDie(){
	//iprintf("New die\n");
	for (int tries = 0; tries < 100; tries++){
		Space *s = space(rand()%size, rand()%size);
		if ( s->getDie() == NULL ){
			s->setDie(new Die(s));
			return;
		}
	}
		
}

int Board::dieCount(){
	int ret = 0;
	for ( int x = 0; x < size; x++){
		for( int y = 0; y < size; y++ ){
			if ( space(x, y)->getDie() != NULL && space(x, y)->getDie()->getState() == STATE_UP )
				ret++;
		}
	}
	return ret;
}

bool Board::full(){
	return dieCount() == ( size * size );
}

float Board::fullness(){
	return (float)dieCount() / ( size * size );
}
		
Space* Board::space(int x, int y)
{
	if ( x < 0 || y < 0 || x >= size || y >= size )
		return NULL;
	return spaces[x+size*y];
}

void Board::tick(){
	if ( rand()%200 < 2 )
		newDie();
	for ( int x = 0; x < size; x++ )
			for ( int y = 0; y < size; y++ )
				space(x, y)->tick();
}

void Board::draw(){
	glTranslate3f32(floattof32(-size/2), 0, floattof32(-size/2) );
	for ( int x = 0; x < size; x++ ){
		for ( int y = 0; y < size; y++ ){
			glPushMatrix();
			glTranslate3f32(inttof32(x), 0, inttof32(y) );
			space(x, y)->draw();
			glPopMatrix(1);
		}
	}
}
