#include "Player.h"
#include <nds.h>
#include "Space.h"
#include "Die.h"
#include "PlayerLoc.h"
#include <stdio.h>
#include "assert.h"

#include "up_raw.h"
#include "down_raw.h"

#define MAGIC 0xfafafafa

extern int textureID2;
extern int textureID;

PlayerLoc Player::allPlayers;

Player::Player(Space* s):magic(MAGIC),x(0.0),z(0.0),speed(0.08),dir(NODIR),space(NULL),die(NULL),points(0),step(0),slowstep(0), fdir(NODIR){
	allPlayers.add(this);

	if ( s->getDie() != NULL ){
		die = s->getDie();
		s->getDie()->add(this);
	} else {
		space = s;
		s->add(this);
	}
}

void Player::tickPlayers(){
	for ( int i = 0; i < allPlayers.getCount(); i++ )
		allPlayers.player(i)->tick();
}

void Player::setDir(char d){
	checkme;
	dir = d;
	if ( dir != NODIR )
		fdir = d;
}

void Player::locChanged( Space* s, Die* d){
	checkme;
	assert( s || d );
	//iprintf("My location changed under me\n");
	die = d;
	space = s;
}


#define CENTER_SPEED 0.97
void Player::tick(){
	checkme;
	
	//iprintf("Ticking Player (%d), ss: %d, s: %d\n", this, slowstep, step);
	
	slowstep++;
	if ( slowstep == 5 ){
		slowstep = 0;
		if ( dir != NODIR || step != 2){
			step++;
			step = step % 5;
		}
	}
		
	if ( die != NULL && dir == die->getDir() ){
			x *= 0.9;
			z *= 0.9;
	} else {
		switch (dir){
			case NODIR:
				//move towards center;
				x *= CENTER_SPEED;
				z *= CENTER_SPEED;
				break;
			case NORTH:
				z -= speed;
				x *= CENTER_SPEED;
				if ( z <= -0.5 ){
					z = -0.5;
					hitEdge(NORTH);
				}
				break;
			case SOUTH:
				z += speed;
				x *= CENTER_SPEED;
				if ( z >= 0.5 ){
					z = 0.5;
					hitEdge(SOUTH);
				}
				break;
			case WEST:
				x -= speed;
				z *= CENTER_SPEED;
				if ( x <= -0.5 ){
					x = -0.5;
					hitEdge(WEST);
				}
				break;
			case EAST:
				x += speed;
				z *= CENTER_SPEED;
				if ( x >= 0.5 ){
					x = 0.5;
					hitEdge(EAST);
				}
				break;
		}
	}
}

float Player::getX(){
	checkme;
	return x;
}

float Player::getZ(){
	checkme;
	return z;
}

void Player::moveTo(char d, bool up){
	checkme;
	Space *ns = NULL;
	Die *nd = NULL;
	/*
	if (up)
		iprintf("Player moving to die\n");
	else
		iprintf("Player moving to space\n");
		*/
	if ( die != NULL )
		ns = die->getSpace()->neighbor(d);
	else
		ns = space->neighbor(d);

	nd = ns->getDie();
	
	if ( space )
		space->remove(this);
	if ( die )
		die->remove(this);
		
	if ( space != NULL && up )
		playGenericSound(up_raw, up_raw_size);
	else if ( die != NULL && !up )
		playGenericSound(down_raw, down_raw_size);
	
	
	if (up){
		assert(nd);
		nd->add(this);
		die = nd;
		space = NULL;
	} else {
		assert(ns);
		ns->add(this);
		space = ns;
		die = NULL;
	}
	
	if ( d == NORTH || d == SOUTH )
		z = z*-1;
	else
		x = x*-1;
}

void Player::hitEdge(char d){
	checkme;
	//iprintf("Player hit edge %d\n", d);
	Space *ns;
	if ( die != NULL )
		ns = die->getSpace()->neighbor(d);
	else
		ns = space->neighbor(d);
	if ( ns == NULL ){
		//iprintf("No neighbor space to move to\n");
		dir = NODIR;
		return;
	}
	Die *nd = ns->getDie();
	
	if ( space ){
		//if the space is empty move there
		if ( !nd )
			return moveTo(d, false);
		//if the die is sinking or just rising
		if ( nd && ( nd->getState() == STATE_SINK2 || nd->getState() == STATE_RISE1 ) )
			return moveTo(d, true);
		if ( nd && nd->getState() == STATE_UP )
			if ( nd->push(d, this) )
				return moveTo(d, false);
		dir = NODIR;
	} else if ( die ){
		if ( nd  ){
			//can I step to another?
			switch( die->getState() ){
				case STATE_UP:
					switch ( nd->getState() ){
						case STATE_UP:
						case STATE_SINK1:
						case STATE_RISE2:
							return moveTo(d, true);
						case STATE_SINK2:
						case STATE_RISE1:
							die->roll(d, this);
					}
					break;
				case STATE_SINK1:
				case STATE_RISE2:
					return moveTo(d, true);
				case STATE_SINK2:
				case STATE_RISE1:
					switch ( nd->getState() ){
						case STATE_SINK1:
						case STATE_SINK2:
						case STATE_RISE1:
						case STATE_RISE2:
							return moveTo(d, true);
					}
			}
		} else {
			//I can step down
			if ( die->getState() == STATE_SINK2 || die->getState() == STATE_RISE1 ){
				return moveTo(d, false);
			} else {
				die->roll(d, this);
				dir = NODIR;
			}
		}
		
	}
}

int fx[6] = {0,29,57,80,103,128};
int fy[2] = {128,92};
int bx[6] = {0,28,56,80,103,128};
int by[2] = {90,56};

void Player::draw(){
	checkme;
	
	glBindTexture(0, textureID2);
	int *tx = fx;
	int *ty = fy;
	//make him backwards
	if (  fdir == NORTH || fdir == WEST ){
		tx = bx;
		ty = by;
	}
	
	//mirror him
	int xa = tx[step];
	int xb = tx[step+1];
	if (fdir == NORTH || fdir == EAST){
		 xa = tx[step+1];
		 xb = tx[step];
	}
	
	int w = tx[step+1]-tx[step];
	
	GLvector s;
	s.x = floattof32(((float)w)/20.0f);
	s.y = inttof32(1);
	s.z = inttof32(1);
	
	glScalev(&s);
	
	glBegin(GL_QUAD);

		glNormal(NORMAL_PACK(0,inttov10(1),0));

		glTexCoord1i(TEXTURE_PACK(inttot16(xa), inttot16(ty[1])));
		glVertex3v16(floattov16(-.3), floattov16(-.5), floattov16(.3) );
		
		glTexCoord1i(TEXTURE_PACK(inttot16(xb), inttot16(ty[1])));
		glVertex3v16(floattov16(.3), floattov16(-.5), floattov16(-.3) );

		glTexCoord1i(TEXTURE_PACK(inttot16(xb), inttot16(ty[0])));
		glVertex3v16(floattov16(.3), floattov16(1), floattov16(-.3) );
		
		glTexCoord1i(TEXTURE_PACK(inttot16(xa), inttot16(ty[0])));
		glVertex3v16(floattov16(-.3),	floattov16(1), floattov16(.3) );

	
	glEnd();
	
	/*
	glBindTexture(0, 0);
	
	glMaterialf(GL_AMBIENT, RGB15(31,0,0));

	
	glBegin(GL_TRIANGLES);
		glColor3b(255,0,0);
		
		glNormal(NORMAL_PACK(floattov10(0.0),floattov10(0.433),floattov10(0.82))); //Front
		glVertex3v16(inttov16(0), floattov16(0.82), inttov16(0)); //1
		glVertex3v16(floattov16(-0.5), inttov16(0), floattov16(0.433)); //2
		glVertex3v16(floattov16(0.5), inttov16(0), floattov16(0.433)); //3
		
		glNormal(NORMAL_PACK(floattov10(0.0),floattov10(-0.999),floattov10(0.0))); //Bottom
		glVertex3v16(inttov16(0), inttov16(0), floattov16(-0.433)); //4
		glVertex3v16(floattov16(0.5), inttov16(0), floattov16(0.433)); //3
		glVertex3v16(floattov16(-0.5), inttov16(0), floattov16(0.433)); //2
		

		glNormal(NORMAL_PACK(floattov10(0.58),floattov10(0.58),floattov10(-0.58)));//RIGHT
		glVertex3v16(floattov16(0.5), inttov16(0), floattov16(0.433)); //3
		glVertex3v16(inttov16(0), inttov16(0), floattov16(-0.433)); //4
		glVertex3v16(inttov16(0), floattov16(0.82), inttov16(0)); //5
		
		glNormal(NORMAL_PACK(floattov10(0.58),floattov10(-0.58),floattov10(-0.58)));//LEFT
		glVertex3v16(floattov16(-0.5), inttov16(0), floattov16(0.433)); //6
		glVertex3v16(inttov16(0), floattov16(0.82), inttov16(0)); //5
		glVertex3v16(inttov16(0), inttov16(0), floattov16(-0.433)); //4
		
	glEnd();
	
	glMaterialf(GL_AMBIENT, RGB15(8,8,8));
	*/
	



}

void Player::award(int p){
	checkme;
	points += p;
	iprintf("Score %d (+%d)\n", points, p);
}
