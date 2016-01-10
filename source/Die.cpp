#include "Die.h"
#include "Space.h"
#include "Player.h"
#include <stdio.h>
#include "slide_raw.h"
#include "roll_raw.h"
#include "new_raw.h"
#include "assert.h"

#define MAGIC 0xbabababa

#define ROLLSTEP 4
#define SLIDESTEP 4
#define SINKTIME 250
#define RISETIME SINKTIME
#define CHAINSTEP 50

#define checkme assert(this != NULL && this->magic == MAGIC && this->on != NULL)

extern int textureID;

int Die::numdice = 0;
int Die::dicesize = 0;
Die** Die::dice = NULL;

//verticies for the cube
v16 Die::CubeVectors[] = {
 		floattov16(-0.5), floattov16(-0.5), floattov16(0.5), 
		floattov16(0.5),  floattov16(-0.5), floattov16(0.5),
		floattov16(0.5),  floattov16(-0.5), floattov16(-0.5),
		floattov16(-0.5), floattov16(-0.5), floattov16(-0.5),
		floattov16(-0.5), floattov16(0.5),  floattov16(0.5), 
		floattov16(0.5),  floattov16(0.5),	floattov16(0.5),
		floattov16(0.5),  floattov16(0.5),  floattov16(-0.5),
		floattov16(-0.5), floattov16(0.5),  floattov16(-0.5)
};

u32 Die::normals[] =
{
	NORMAL_PACK(0,				inttov10(1),	0),				//1 +y
	NORMAL_PACK(0,				0,				inttov10(1)-1),	//2 -z
	
	NORMAL_PACK(inttov10(1)-1,	0,				0),				//3 -x
	NORMAL_PACK(inttov10(1),	0,				0),				//4 +x
	
	NORMAL_PACK(0,				0,				inttov10(1)),	//5 +z
	NORMAL_PACK(0,				inttov10(1)-1,	0)				//6 -y
};


//polys
u8 Die::CubeFaces[] = {
	5,6,7,4, //1
	2,3,7,6, //2
	3,0,4,7, //3
	1,2,6,5, //4
	0,1,5,4, //5 
	3,2,1,0  //6
};


//texture coordinates
u32 Die::uv[] =
{
	//ONE uv fixed
	TEXTURE_PACK(inttot16(0), inttot16(192)),
	TEXTURE_PACK(inttot16(64),inttot16(192)),
	TEXTURE_PACK(inttot16(64), inttot16(128)),
	TEXTURE_PACK(inttot16(0),inttot16(128)),
	//TWO uv
	TEXTURE_PACK(inttot16(128), inttot16(256)),
	TEXTURE_PACK(inttot16(192),inttot16(256)),
	TEXTURE_PACK(inttot16(192), inttot16(192)),
	TEXTURE_PACK(inttot16(128),inttot16(192)),
	//THREE uv
	TEXTURE_PACK(inttot16(64), inttot16(192)),
	TEXTURE_PACK(inttot16(128),inttot16(192)),
	TEXTURE_PACK(inttot16(128), inttot16(128)),
	TEXTURE_PACK(inttot16(64),inttot16(128)),	
	//FOUR uv
	TEXTURE_PACK(inttot16(192), inttot16(192)),
	TEXTURE_PACK(inttot16(256),inttot16(192)),
	TEXTURE_PACK(inttot16(256), inttot16(128)),
	TEXTURE_PACK(inttot16(192),inttot16(128)),	
	//FIVE uv
	TEXTURE_PACK(inttot16(64), inttot16(256)),
	TEXTURE_PACK(inttot16(128),inttot16(256)),
	TEXTURE_PACK(inttot16(128), inttot16(192)),
	TEXTURE_PACK(inttot16(64),inttot16(192)),	
	//SIX uv
	TEXTURE_PACK(inttot16(128), inttot16(192)),
	TEXTURE_PACK(inttot16(192),inttot16(192)),
	TEXTURE_PACK(inttot16(192), inttot16(128)),
	TEXTURE_PACK(inttot16(128),inttot16(128))
};


void Die::sinkOnes(Player *mover){
	assert(mover);
	
	for ( int i = 0; i < numdice; i++ ){
		if ( dice[i]->n_up == 1 ){
			if ( !dice[i]->hasPlayer(mover) ){
				dice[i]->sink(1);
				if ( mover )
					mover->award(1);
			}
		}
	}
}


void Die::clearVisited(){
	for ( int i = 0; i < numdice; i++ )
		dice[i]->visited = 0;
}

Die::Die(Space* s):
		magic(MAGIC),
		on(s),
		state(STATE_RISE1),
		step(0),
		n_up(1),n_east(4),
		n_south(5),
		orientation(new m3x3()),
		scratch(new m3x3()),
		chain(0),
		dir(NODIR),
		mover(NULL){

	for ( int i = 0; i < 9; i++ )
		orientation->m[i] = 0;
	orientation->m[0] = orientation->m[4] = orientation->m[8] = inttof32(1);
	if ( rand()%2 )
		rotate(NORTH);
	if ( rand()%2 )
		rotate(NORTH);
	if ( rand()%2 )
		rotate(EAST);
	if ( rand()%2 )
		rotate(EAST);
		
	numdice++;
	if ( numdice > dicesize ){
		dicesize = numdice * 2;
		dice = (Die**)realloc(dice, dicesize * sizeof(Die*));
	}
	dice[numdice-1] = this;
	//iprintf("Dice: num %d sz %d\n", numdice, dicesize);
		
	playGenericSound(new_raw, new_raw_size);
}

Die::~Die(){
	checkme;
	//iprintf("Deleting a die\n");
	while( getCount() ){
		Player *p = player(0);
		remove(p);
		on->add(p);
		p->locChanged(on, NULL);
	}
	
	int i = 0;
	while ( dice[i] != this ){
		i++;
	}
	numdice--;
	dice[i] = dice[numdice];
	dice[numdice] = NULL;
	
	delete(orientation);
	delete(scratch);
	//iprintf("Dice: num %d sz %d removed %d\n", numdice, dicesize, i);
}



bool Die::roll(int d, Player* m){
	checkme;
	assert(m);
	if ( state != STATE_UP )
		return false;
	mover = m;
	if ( !on->neighbor(d) ){
		//no neighbor space?
		return false;
	} 
	if ( on->neighbor(d)->getDie() ){
		if ( on->neighbor(d)->getDie()->getState() != STATE_RISE1 && on->neighbor(d)->getDie()->getState() != STATE_SINK2 )
			return false;
	}
	state = STATE_ROLLOUT;
	dir = d;
	step = 0;
	return true;
}

bool Die::push(int d, Player* m){
	checkme;
	assert(m);
	if ( state != STATE_UP )
		return false;
	mover = m;
	if ( !on->neighbor(d) ){
		//no neighbor space?
		return false;
	} 
	if ( on->neighbor(d)->getDie() ){
		//there is a neighbor and a die there
		if ( on->neighbor(d)->getDie()->getState() == STATE_RISE1 || on->neighbor(d)->getDie()->getState() == STATE_SINK2 ){
			//if the die is down get rid of it
			on->neighbor(d)->setDie(NULL);
		} else {
			return false;
		}
	}
	state = STATE_SLIDEOUT;
	dir = d;
	step = 0;
	playGenericSound(slide_raw, slide_raw_size);
	return true;
}

void z90(int32 *m1, int32 *m, int dir) {
	assert(m1);
	assert(m);
	m[0] = m1[1]*dir;
	m[1] = m1[0]*-1*dir;
	m[2] = m1[2];
	m[3] = m1[4]*dir;
	m[4] = m1[3]*-1*dir;
	m[5] = m1[5];
	m[6] = m1[7]*dir;
	m[7] = m1[6]*-1*dir;
	m[8] = m1[8];
}


void x90(int32 *m1, int32 *m, int dir) {
	assert(m1);
	assert(m);
	m[0] = m1[0];
	m[1] = m1[2]*dir;
	m[2] = m1[1]*-1*dir;
	m[3] = m1[3];
	m[4] = m1[5]*dir;
	m[5] = m1[4]*-1*dir;
	m[6] = m1[6];
	m[7] = m1[8]*dir;
	m[8] = m1[7]*-1*dir;
}

void Die::rotate(int d)
{
	checkme;
	int o_up = n_up;
	int o_east = n_east;
	int o_south = n_south;
	m3x3* tmp;
	switch (d){
	case NORTH:
		n_up = o_south;
		n_south = 7 - o_up;
		x90(orientation->m, scratch->m, 1);
		break;	
	case SOUTH:
		n_up = 7 - o_south;
		n_south = o_up;
		x90(orientation->m, scratch->m, -1);
		break;	
	case EAST:
		n_east = o_up;
		n_up = 7-o_east;
		z90(orientation->m, scratch->m, 1);
		break;	
	case WEST:
		n_up = o_east;
		n_east = 7 - o_up;
		z90(orientation->m, scratch->m, -1);
		break;
	}
	tmp = orientation;
	orientation = scratch;
	scratch = tmp;

}

char Die::getDir(){
	checkme;
	return dir;
}

void Die::tick(){
	checkme;
	switch ( state ){
		case STATE_ROLLOUT:
			step += ROLLSTEP;
			if ( step >= 45 ){
				state = STATE_ROLLIN;
				
				on->setDie(NULL);				
				on = on->neighbor(dir);
				on->setDie(this);
				
				rotate(dir);
				step = 0;
			}
			break;
		case STATE_ROLLIN:
			step += ROLLSTEP;
			if ( step >= 45 ){
				playGenericSound(roll_raw, roll_raw_size);
				state = STATE_UP;
				dir = NODIR;
				step = 0;
				check();
				return;
			}
			break;
		case STATE_SLIDEOUT:
			step += SLIDESTEP;
			if ( step >= 45 ){
				state = STATE_SLIDEIN;
				on->setDie(NULL);
				on = on->neighbor(dir);
				on->setDie(this);
				step = 0;
			}
			break;
		case STATE_SLIDEIN:
			step += SLIDESTEP;
			if ( step >= 45 ){
				state = STATE_UP;
				dir = NODIR;
				step = 0;
				check();
				return;
			}
			break;
		case STATE_RISE1:
			step += 1;
			if ( step >= (RISETIME/2) ){
				state = STATE_RISE2;
				step = 0;
				return;
			}
			break;
		case STATE_RISE2:
			step += 1;
			if ( step >= (RISETIME/2) ){
				state = STATE_UP;
				step = 0;
				return;
			};
			break;
		case STATE_SINK1:
			step += 1;
			if ( step >= (SINKTIME/2) ){
				state = STATE_SINK2;
				step = 0;
				return;
			}
			break;
		case STATE_SINK2:
			step += 1;
			if ( step >= (SINKTIME/2) ){
				state = STATE_UP;
				chain = 0;
				step = 0;
				
				on->setDie(NULL);
				//WARNING I am deleted after this line!
				delete(this);
				return;
			};
			break;
	}
}

void Die::doSink(){
	checkme;
	int s = step;
	if ( state == STATE_RISE2 || state == STATE_SINK2 )
		s = s + SINKTIME/2;
	if ( state == STATE_RISE1 || state == STATE_RISE2 )
		s = SINKTIME - s;
	float dist = (float)s / (float)-SINKTIME;
	glTranslate3f32(0, floattof32(dist), 0);
}

void Die::doPush(){
	checkme;
	int d = state == STATE_SLIDEOUT ? dir : dir ^ 1;
	float dist = (float)step / 90.0;
	if ( state == STATE_SLIDEIN )
		dist = 0.5 - dist;

	switch ( d ){
		case SOUTH:
			glTranslate3f32(0, 0, floattof32(dist));
			break;
		case NORTH:
			glTranslate3f32(0, 0, floattof32(-dist));
			break;
		case WEST:
			glTranslate3f32(floattof32(-dist), 0, 0);
			break;
		case EAST:
			glTranslate3f32(floattof32(dist), 0, 0);
			break;
	}
}

Space* Die::getSpace(){
	checkme;
	return on;
}

char Die::getState(){
	checkme;
	return state;
}


void Die::doRoll(){
	checkme;
	int rot = state == STATE_ROLLOUT ? step : 45 - step;
	int d = state == STATE_ROLLOUT ? dir : dir ^ 1;
	
	switch ( d ){
		case SOUTH:
			glTranslate3f32(0, floattof32(-0.5f), floattof32(0.5f));
			glRotateX(rot);
			glTranslate3f32(0, floattof32(0.5f), floattof32(-0.5f));
			break;
		case NORTH:
			glTranslate3f32(0, floattof32(-0.5f), floattof32(-0.5f));
			glRotateX(-1 * rot );
			glTranslate3f32(0, floattof32(0.5f), floattof32(0.5f));
			break;
		case WEST:
			glTranslate3f32(floattof32(-0.5f), floattof32(-0.5f), 0 );
			glRotateZ(rot);
			glTranslate3f32(floattof32(0.5f), floattof32(0.5f), 0);
			break;
		case EAST:
			glTranslate3f32(floattof32(0.5f), floattof32(-0.5f), 0 );
			glRotateZ(-1 * rot);
			glTranslate3f32(floattof32(-0.5f), floattof32(0.5f), 0);
			break;
	}

}

void Die::draw(){	
	checkme;
	glBindTexture(0, textureID);
	
	glPushMatrix();
	
	
	switch( state ){
		case STATE_ROLLOUT:
		case STATE_ROLLIN:
			doRoll();	
			break;
		case STATE_SLIDEOUT:
		case STATE_SLIDEIN:
			doPush();	
			break;
		case STATE_RISE1:
		case STATE_RISE2:
		case STATE_SINK1:
		case STATE_SINK2:
			doSink();
			break;
	}
		
	
	glMultMatrix3x3(orientation);
	
	if ( state < STATE_UP ){
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glTranslate3f32(0, inttof32(-2048), 0);
		glMatrixMode(GL_MODELVIEW);
	}

	glBegin(GL_QUAD);

	
	for(int i = 0; i < 6; i++){
		u32 f1 = CubeFaces[i * 4] ;
		u32 f2 = CubeFaces[i * 4 + 1] ;
		u32 f3 = CubeFaces[i * 4 + 2] ;
		u32 f4 = CubeFaces[i * 4 + 3] ;

		glNormal(normals[i]);

		glTexCoord1i(uv[i * 4 + 0]);
		glVertex3v16(CubeVectors[f1*3], CubeVectors[f1*3 + 1], CubeVectors[f1*3 +  2] );
		
		glTexCoord1i(uv[i * 4 + 1]);
		glVertex3v16(CubeVectors[f2*3], CubeVectors[f2*3 + 1], CubeVectors[f2*3 + 2] );
		
		glTexCoord1i(uv[i * 4 + 2]);
		glVertex3v16(CubeVectors[f3*3], CubeVectors[f3*3 + 1], CubeVectors[f3*3 + 2] );

		glTexCoord1i(uv[i * 4 + 3]);
		glVertex3v16(CubeVectors[f4*3], CubeVectors[f4*3 + 1], CubeVectors[f4*3 + 2] );	

	}
	
	glEnd();
	
	if ( state < STATE_UP ){
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
	}
	glMatrixMode(GL_MODELVIEW);
	
	glPopMatrix(1);
	
	if ( getCount() ){
		glPushMatrix();
		glTranslate3f32(0, inttof32(1), 0 );
		
		char s, rd;
		switch( state ){
			case STATE_ROLLOUT:
			case STATE_ROLLIN:
					s = state;
					state = ( state == STATE_ROLLOUT ? STATE_SLIDEOUT : STATE_SLIDEIN );
					doPush();
					state = s;
					rd = step;
					if ( state == STATE_ROLLIN ){
						rd = 45 - step;
					}
					glTranslate3f32(0, floattof32(rd * 0.005f), 0);
				break;
			case STATE_SLIDEOUT:
			case STATE_SLIDEIN:
				doPush();	
				break;
			case STATE_RISE1:
			case STATE_RISE2:
			case STATE_SINK1:
			case STATE_SINK2:
				doSink();
				break;
		}
		
		for ( int i = 0; i < getCount(); i++ ){
			glPushMatrix();
			Player* p = player(i);
			glTranslate3f32(floattof32(p->getX()), 0, floattof32(p->getZ()));
			p->draw();
			glPopMatrix(1);
		}
		glPopMatrix(1);
	}
	
	
	
}

void Die::sink(int c){
	checkme;
	chain = c;
	visited = 1;
	if( state == STATE_UP )
		state = STATE_SINK1;
		
	//step it up
	step -= CHAINSTEP;
	if ( step < 0 ){
		if ( state == STATE_SINK2 ){
			state = STATE_SINK1;
			step += SINKTIME /2;
		} else {
			step = 0;
		}
	}
	
	for ( char d = 0; d < 4; d++ ){
		Space *s = on->neighbor(d);
		if ( s == NULL )
			continue;
		Die * d = s->getDie();
		if ( d == NULL )
			continue;
		//Recurse if there is a unvisited die
		if ( d->n_up == n_up && !d->visited){
			//TODO d has to be up or sinking
			d->sink(chain);
		}
	}
}

void Die::check(){
	checkme;
	if ( n_up == 1 ){
		for ( char d = 0; d < 4; d++ ){
			Space *s = on->neighbor(d);
			if ( s == NULL )
				continue;
			Die * d = s->getDie();
			if ( d == NULL )
				continue;
			if ( d->state == STATE_SINK1 || d->state == STATE_SINK2 ){
				sinkOnes(mover);
				clearVisited();
			}
		}
		return;
	}
	
	int count = checkR();
	clearVisited();
	if ( count >= n_up ){
			int mc = maxChain();
			clearVisited();
			sink(mc+1);
			clearVisited();
			//iprintf("Count: %d, Chain: %d, Num: %d\n", count, chain, n_up);
			if ( mover )
				mover->award( count * chain * n_up );
	}

}

int Die::maxChain(){
	checkme;
	visited = 1;
	for ( char d = 0; d < 4; d++ ){
		Space *s = on->neighbor(d);
		if ( s == NULL )
			continue;
		Die * d = s->getDie();
		if ( d != NULL && d->n_up == n_up &&
				(d->state == STATE_UP || d->state == STATE_SINK1 || d->state == STATE_SINK2)
			){
			if ( !d->visited )
				d->maxChain();
			if (d->chain > chain){
				chain = d->chain;
			}
		}
	}
	return chain;
}

int Die::checkR(){
	checkme;
	int count = 1;
	visited = 1;

	for ( char d = 0; d < 4; d++ ){
		Space *s = on->neighbor(d);
		if ( s == NULL )
			continue;
		Die * d = s->getDie();
		//Recurse if there is a unvisited die
		if ( d != NULL && d->n_up == n_up &&
				(d->state == STATE_UP || d->state == STATE_SINK1 || d->state == STATE_SINK2)
			){
			if ( !d->visited )
				count += d->checkR();
		}
	}
	return count;
}

