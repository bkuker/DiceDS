#include <nds.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

//texture_bin.h is created automagicaly from the texture.bin placed in arm9/resources
//texture.bin is a raw 128x128 16 bit image.  I will release a tool for texture conversion 
//later
#include "die_bmp_bin.h"
#include "mario_bmp_bin.h"
#include "billkuker_bmp_bin.h"

#include "Die.h"
#include "Space.h"
#include "Board.h"
#include "Player.h"
#include "lose_raw.h"
#include "assert.h"

#include "up_raw.h"
#include "down_raw.h"

int textureID = 0;
int textureID2 = 0;
int logotex = 0;

void logo()
{
	glResetMatrixStack();
	glMatrixMode(GL_PROJECTION);
	gluPerspective(20, 256.0 / 192.0, 0.1, 40);

	gluLookAt(	0.0, .55, 0.0 ,		//camera possition 
				0.0, 0.0, 0.0,		//look at
				0.0, 0.0, -1.0);		//up
				


	glLight(0, RGB15(31,31,31), 0,	floattov10(1.0)-1, 0);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	
	glMaterialf(GL_AMBIENT, RGB15(8,8,8));
	glMaterialf(GL_DIFFUSE, RGB15(31,31,31));
	glMaterialf(GL_SPECULAR, RGB15(31,31,31));
	glMaterialf(GL_EMISSION, RGB15(8,8,8));
	glMaterialShinyness();
	glPolyFmt( POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 );
		
		
	glGenTextures(1, &logotex);
	glBindTexture(0, logotex);
	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_256 , TEXTURE_SIZE_256, 0, TEXGEN_TEXCOORD , (u8*)billkuker_bmp_bin);
	glBindTexture(0, logotex);

	glBegin(GL_QUAD);
		glNormal(NORMAL_PACK(0,inttov10(1),0));

		glTexCoord1i(TEXTURE_PACK(0,0));
		glVertex3v16(floattov16(-0.5), floattov16(-0.5), floattov16(0.5) );
		
		glTexCoord1i(TEXTURE_PACK(inttot16(256), inttot16(0)));
		glVertex3v16(floattov16(0.5), floattov16(-0.5), floattov16(0.5) );

		glTexCoord1i(TEXTURE_PACK(inttot16(256),inttot16(256)));
		glVertex3v16(floattov16(0.5), floattov16(-0.5), floattov16(-0.5) );
		
		glTexCoord1i(TEXTURE_PACK(0, inttot16(256)));
		glVertex3v16(floattov16(-0.5),	floattov16(-0.5), floattov16(-0.5) );
	glEnd();
	
	glFlush(0);
	
	playGenericSound(down_raw, down_raw_size);
	playGenericSound(up_raw, up_raw_size);
	
	swiWaitForVBlank();
	
	iprintf("Press any button");
	
	while( !keysHeld() )
		scanKeys();
		
	glResetTextures();
	iprintf("\x1b[2J");
}


void credits(){
	iprintf("\x1b[2J");
	iprintf("\x1b[0;0H\n\n-=-=-=-=-= Mario Dice =-=-=-=-=-\n\n");
	iprintf("           Bill Kuker\n\n");
	iprintf("	     www.billkuker.com\n");
	iprintf("\n\n\n");
	iprintf("Sound from Freesound Project:\n");
	iprintf("\nNoiseCollector:\n");
	iprintf("		Boom 2\n");
	iprintf("\nAcclivity:\n");
	iprintf("		BeepRising\n");
	iprintf("		BeepFalling\n");
	iprintf("\nHell's Sound Guy:\n");
	iprintf("		Bubble Pop\n");
}

void loseRender(Board *b, int step){
		glResetMatrixStack();
		if ( step > 1000 )
			return;
		glMatrixMode(GL_PROJECTION);
		gluPerspective(20, 256.0 / 192.0, 0.1, 40);
		
		float d = (float)step / 6.0;
		gluLookAt(	5.0 + d, 5.0 + d, 7.0 + d,		//camera possition 
					1.0, 0.0, 1.5,		//look at
					0.0, 1.0, 0.0);		//up
					
		glRotateY(step*3);

		glLight(0, RGB15(31,31,31), 0,	floattov10(1.0)-1, 0);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glMaterialf(GL_AMBIENT, RGB15(8,8,8));
		glMaterialf(GL_DIFFUSE, RGB15(31,31,31));
		glMaterialf(GL_SPECULAR, RGB15(31,31,31));
		glMaterialf(GL_EMISSION, RGB15(8,8,8));
		glMaterialShinyness();
		glPolyFmt( POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 );

		glBindTexture(0, textureID);
		b->draw();
		glFlush(0);
		swiWaitForVBlank();
}

void lose(Board *b){
	credits();
	
	int s = 0;
	playGenericSound(lose_raw, lose_raw_size);
	//Wait for them to release a key
	while( keysHeld() ){
		scanKeys();
		loseRender(b, s++);
		swiWaitForVBlank();
	}
	//Let them stew for a few seconds
	for ( int f = 0; f < 180; f++ ){
		loseRender(b, s++);
		swiWaitForVBlank();
	}
	//wait for a keypress
	while( !keysHeld() ){
		loseRender(b, s++);
		scanKeys();
		swiWaitForVBlank();
	}
}

void gameLoop(){
	iprintf("\x1b[2J");
	int frame = 0;
	Board b(7);
	Player* p;
	p = new Player(b.space(5,4));
	
	//iprintf("Entering Main Loop.\n");

	int rot = 0;
	while( !b.full() )		
	{
		iprintf("\x1b[22;18HFrame: %d\n\x1b[0;0H", ++frame);
		glResetMatrixStack();

		//any floating point gl call is being converted to fixed prior to being implemented
		glMatrixMode(GL_PROJECTION);
		gluPerspective(20, 256.0 / 192.0, 0.1, 40);
		
		gluLookAt(	5.0, 5.0, 7.0,		//camera possition 
					1.0, 0.0, 1.5,		//look at
					0.0, 1.0, 0.0);		//up

		glLight(0, RGB15(31,31,31), 0,	floattov10(1.0)-1, 0);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);

		glMaterialf(GL_AMBIENT, RGB15(8,8,8));
		glMaterialf(GL_DIFFUSE, RGB15(31,31,31));
		glMaterialf(GL_SPECULAR, RGB15(31,31,31));
		glMaterialf(GL_EMISSION, RGB15(8,8,8));

		//ds uses a table for shinyness..this generates a half-ass one
		glMaterialShinyness();

		//not a real gl function and will likely change
		glPolyFmt( POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 );
		//glPolyFmt(POLY_ALPHA(15) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 );
		
		glRotateY(rot);

		glBindTexture(0, textureID);

		b.draw();

		glFlush(0);

		b.tick();
		Player::tickPlayers();

		scanKeys();
		u16 keys = keysHeld();

		p->setDir(NODIR);
		if((keys & KEY_UP)) p->setDir(NORTH);
		if((keys & KEY_DOWN)) p->setDir(SOUTH);
		if((keys & KEY_LEFT)) p->setDir(WEST);
		if((keys & KEY_RIGHT)) p->setDir(EAST);


		if (( keys & KEY_R )) rot++;
		if (( keys & KEY_L )) rot--;
		
		swiWaitForVBlank();
	}
	lose(&b);
}

	
int main()
{	
	struct timeval tv;
	
	powerON(POWER_ALL);

	//set mode 0, enable BG0 and set it to 3D
	videoSetMode(MODE_0_3D);
	
	//Use console
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);	//sub bg 0 will be used to print text
	vramSetBankC(VRAM_C_SUB_BG); 
	SUB_BG0_CR = BG_MAP_BASE(31);
	BG_PALETTE_SUB[255] = RGB15(31,31,31);	//by default font will be rendered with color 255
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
	
	//iprintf("Starting up...\n");
	
	// install the default exception handler
	defaultExceptionHandler();


	//irqs are nice
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	// set the generic sound parameters
	setGenericSound(	11025,	/* sample rate */
						127,	/* volume */
						64,		/* panning */
						1 );	/* sound format*/
						

	glInit();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
		
	//this should work the same as the normal gl call
	glViewPort(0,0,255,191);
	
	glClearColor(0,0,0,0);
	glClearDepth(0x7FFF);
	
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	
	logo();

	glGenTextures(1, &textureID);
	glBindTexture(0, textureID);
	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_256 , TEXTURE_SIZE_256, 0, TEXGEN_TEXCOORD , (u8*)die_bmp_bin);
	
	
	glGenTextures(1, &textureID2);
	glBindTexture(0, textureID2);
	glTexImage2D(0, 0, GL_RGBA, TEXTURE_SIZE_128 , TEXTURE_SIZE_128, 0, TEXGEN_TEXCOORD, (u8*)mario_bmp_bin);
	
	gettimeofday(&tv, NULL);
	//iprintf("Seeding with %d\n", tv.tv_usec);
    srand(tv.tv_usec);

	while(1){
		gameLoop();
	}

	return 0;
}//end main 
