#include "Space.h"
#include "Die.h"
#include <stdio.h>
#include "Player.h"
#include "assert.h"

#define MAGIC 0xcdcdcdcd

extern int textureID;

Space::Space():magic(MAGIC),die(NULL){
	neighbors[NORTH] = this;
	neighbors[SOUTH] = this;
	neighbors[EAST] = this;
	neighbors[WEST] = this;
}

Space* Space::neighbor(int dir){
	checkme;
	return neighbors[dir];
}

void Space::setNeighbor(int dir, Space *s){
	checkme;
	//iprintf("Space %d n %d set to %d\n", this, dir, s );
	neighbors[dir] = s;
}

void Space::setDie(Die *d){
	checkme;
	//Move players from old die onto space
	if ( die != NULL && d != NULL){
		delete(die);
		die = NULL;
	}
	die = d;
	//Move players from space to die
	if( die != NULL ){
		while( getCount() ){
			Player *p = player(0);
			remove( p );
			die->add( p );
			p->locChanged(NULL, d);
		}
	}
}

Die* Space::getDie(){
	checkme;
	return die;
}

void Space::tick(){
	checkme;
	if ( die )
		die->tick();
}

void Space::draw(){
	checkme;
	
	if ( die ){
		die->draw();
	}
	
	for ( int i = 0; i < getCount(); i++ ){
		glPushMatrix();
		Player* p = player(i);
		glTranslate3f32(floattof32(p->getX()), 0, floattof32(p->getZ()));
		p->draw();
		glPopMatrix(1);
	}
	
	glBindTexture(0, textureID);

	glBegin(GL_QUAD);
		glNormal(NORMAL_PACK(0,inttov10(1),0));

		glTexCoord1i(TEXTURE_PACK(0,inttot16(192)));
		glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(0.5) );
		
		glTexCoord1i(TEXTURE_PACK(inttot16(64), inttot16(192)));
		glVertex3v16(floattov16(0.5), floattov16(-0.5), floattov16(0.5) );

		glTexCoord1i(TEXTURE_PACK(inttot16(64),inttot16(256)));
		glVertex3v16(floattov16(0.5), floattov16(-0.5), floattov16(-0.5) );
		
		glTexCoord1i(TEXTURE_PACK(0, inttot16(256)));
		glVertex3v16(floattov16(-0.5),	floattov16(-0.5), floattov16(-0.5) );

	
	glEnd();
	

}
