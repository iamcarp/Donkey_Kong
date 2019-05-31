#include "battle_city.h"
#include "map.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>
#include "sprites.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
/*          COLOR PALETTE - base addresses in ram.vhd         */
#define FRAME_COLORS_OFFSET         0
#define LINK_COLORS_OFFSET          8
#define ENEMY_COLORS_OFFSET         35

/*		SCREEN PARAMETERS		 - in this case, "screen" stands for one full-screen picture	 */
#define SCREEN_BASE_ADDRESS			6900	//	old: 6900	,	new: 6992
#define SCR_HEIGHT					30
#define SCR_WIDTH					40
#define SPRITE_SIZE					16

#define LADDERS_NUM 13

/*		FRAME HEADER		*/
#define HEADER_BASE_ADDRESS			7192	//	old: 7192	,	new: 7124
#define HEADER_HEIGHT				5

/*      FRAME       */
#define FRAME_BASE_ADDRESS			7392 // 	old: 7392	,	new: 7284		FRAME_OFFSET in battle_city.vhd
#define SIDE_PADDING				10
#define VERTICAL_PADDING			7
#define INITIAL_FRAME_X				7
#define INITIAL_FRAME_Y				7
#define INITIAL_MARIO_POSITION_X		160
#define INITIAL_MARIO_POSITION_Y		328

/*      LINK SPRITES START ADDRESS - to move to next add 64    */
#define LINK_SPRITES_OFFSET             5648		//	old: 5172	,	new: 5648
#define SWORD_SPRITE                    1727			//6068		//	old: 7192	,	new: 7124
#define LINK_STEP						8

/*      ENEMIE SPRITES START ADDRESS - to move to next add 64    */
#define FLAME_SPRITE_OFFSET_LEFT 	   831
#define FLAME_SPRITE_OFFSET_RIGHT       895
#define ENEMIE_SPRITES_OFFSET          5072			//	old: 4596	,	new: 5072
#define ENEMY_STEP						10
#define GHOST_SPRITES_OFFSET			5072 + 64*5

#define REGS_BASE_ADDRESS               ( SCREEN_BASE_ADDRESS + SCR_WIDTH * SCR_HEIGHT )

#define BTN_DOWN( b )                   ( !( b & 0x01 ) )
#define BTN_UP( b )                     ( !( b & 0x10 ) )
#define BTN_LEFT( b )                   ( !( b & 0x02 ) )
#define BTN_RIGHT( b )                  ( !( b & 0x08 ) )
#define BTN_SHOOT( b )                  ( !( b & 0x04 ) )


/*			these are the high and low registers that store moving sprites - two registers for each sprite		 */
#define LINK_REG_L                     8
#define LINK_REG_H                     9
#define WEAPON_REG_L                   4
#define WEAPON_REG_H                   5
#define ENEMY_2_REG_L                  6
#define ENEMY_2_REG_H                  7
#define ENEMY_3_REG_L                  2
#define ENEMY_3_REG_H                  3
#define ENEMY_4_REG_L                  10
#define ENEMY_4_REG_H                  11
#define ENEMY_5_REG_L                  12
#define ENEMY_5_REG_H                  13
#define ENEMY_6_REG_L                  14
#define ENEMY_6_REG_H                  15
#define ENEMY_7_REG_L                  0
#define ENEMY_7_REG_H                  1
#define GRANDPA_REG_L					16
#define GRANDPA_REG_H	                17

#define MAX_HEALTH						8

#define ENEMY_FRAMES_NUM 			34
/*			contains the indexes of frames in overworld which have enemies  	*/
bool ENEMY_FRAMES[] = {32, 33, 45, 48, 49, 55, 56, 62, 64, 65, 68, 73, 76, 79,
					   84, 85, 86, 87, 88, 90, 95, 99, 100, 101, 102, 103, 104,
					   105, 106, 110, 111, 120, 125, 126};



int ladders_up[13][2] = {{176, 328},{192, 288},{208, 248},{224, 208},{256, 208},{280, 328},{312, 248},{360, 288},{376, 208},{408, 208},{424, 248},{432, 288},{458, 328}};
int ladders_down[13][2] = {{176, 288},{192, 248},{208, 208},{224, 168},{256, 168},{280, 288},{312, 208},{360, 248},{376, 168},{408, 168},{424, 208},{432, 248},{458, 288}};
int end_of_block[24][2] = {{159,328},{175,288},{255,288},{399,288},{191,248},{255,248},{399,248},{207,208},{255,208},{399,208},{223,168},{256,128},{470,328},{232,288},{376,288},{458,288},{232,248},{376,248},{440,248},{232,208},{376,208},{424,208},{408,168},{376,128}};
#define EDGE_NUM 16
int edges[16][2] = {{166,288},{182,248},{198,208},{214, 168},{246,128},{244, 288},{244, 248},{244, 208},{386, 288},{386, 248},{386, 208},{466, 288},{448, 248},{432, 208},{416, 168},{376, 128}};
bool on_ladder = false;
int ladder_count = 0;
int enemy_obstackles1[5][2] = {{175, 288},{231,288},{192, 288}, {192, 248}, {231,248}};
int enemy_obstackles2[2][2] = {{256, 248},{384, 248}};
int enemy_obstackles3[2][2] = {{256, 288},{384, 288}};
int enemy_obstackles4[6][2] = {{400, 288},{464, 288},{432, 288}, {400, 248},{464,248}, {432, 248}};

unsigned short fire1 = FIRE_0;
unsigned short fire2 = FIRE_1;
int HEALTH;
int pos = 1;
int counter = 0;
int last = 0; //last state link was in before current iteration (if he is walking it keeps walking)
/*For testing purposes - values for last - Link sprites
	0 - down stand
	1 - down walk
	2 - up walk
	3 - right walk
	4 - right stand
	5 - down stand shield
	6 - down walk shield
	7 - right walk shield
	8 - right stand shield
	9 - down attack
	10 - up attack
	11 - right attack
	12 - item picked up
	13 - triforce picked up
	14 - sword
	15 - up flipped
	16 - left walk
	17 - left stand
	18 - left walk shield
	19 - left stand shield
	20 - left attack
*/

/*		 ACTIVE FRAME		*/
unsigned short* frame;

characters mario = {
		INITIAL_MARIO_POSITION_X,		// x
		INITIAL_MARIO_POSITION_Y,		// y
		DIR_DOWN, 	             		// dir
		0x06BF,							// type - sprite address in ram.vhdl
		true,                			// active
		LINK_REG_L,            			// reg_l
		LINK_REG_H             			// reg_h
		};

characters sword = {
		INITIAL_MARIO_POSITION_X,		// x
		INITIAL_MARIO_POSITION_Y,		// y
		DIR_LEFT,              			// dir
		SWORD_SPRITE,  					// type
		false,                			// active
		WEAPON_REG_L,            		// reg_l
		WEAPON_REG_H             		// reg_h
		};

characters grandpa = {
		256,								// x
		288,								// y
		DIR_LEFT,              			// dir
		GRANDPA_SPRITE,  					// type
		true,                			// active
		GRANDPA_REG_L,            		// reg_l
		GRANDPA_REG_H             		// reg_h
		};

characters flame1 = {
		176,								// x
		288,								// y
		DIR_RIGHT,              			// dir
		FLAME_SPRITE_OFFSET_RIGHT,  					// type
		true,                			// active
		ENEMY_2_REG_L,            		// reg_l
		ENEMY_2_REG_H             		// reg_h
		};


characters flame2 = {
		374,								// x
		248,								// y
		DIR_RIGHT,              			// dir
		FLAME_SPRITE_OFFSET_RIGHT,  					// type
		true,                			// active
		ENEMY_3_REG_L,            		// reg_l
		ENEMY_3_REG_H             		// reg_h
		};

characters flame3 = {
		256,								// x
		288,								// y
		DIR_RIGHT,              			// dir
		FLAME_SPRITE_OFFSET_RIGHT,  					// type
		true,                			// active
		ENEMY_4_REG_L,            		// reg_l
		ENEMY_4_REG_H             		// reg_h
		};

characters flame4 = {
		400,								// x
		288,								// y
		DIR_RIGHT,              			// dir
		FLAME_SPRITE_OFFSET_RIGHT,  					// type
		true,                			// active
		ENEMY_5_REG_L,            		// reg_l
		ENEMY_5_REG_H             		// reg_h
		};

characters ghost = {
		0,								// x
		0,								// y
		DIR_LEFT,              			// dir
		GHOST_SPRITES_OFFSET + 64,  	// type 64*5 base
		true,                			// active
		ENEMY_6_REG_L,            		// reg_l
		ENEMY_6_REG_H             		// reg_h
		};


int walkables[21] = {0, 2, 6, 10, 22, 27, 28, 29, 33, 34, 35, 39, 40, 41, 42, 43, 44, 45, 46, 47, 49};

/*      indexes of the active frame in overworld        */
int overw_x;
int overw_y;

int enemy_exists = 0;
int enemy_step = 0;

bool inCave = false;
/*      the position of the door so link could have the correct position when coming out of the cave    */
int door_x, door_y;
int rupees = 0, bombs = 0;

unsigned int random_number() {
	static unsigned int i,j;
	int p,q;
	unsigned int rnd;
	srand(time(NULL));
	i +=5;
	j +=3;

	p = rand()%1000;
	q = rand()%1000;
	rnd = ((p+1)%36)*((q+j)%35);

	return rnd;
}

unsigned short char_to_addr(char c) {
    switch(c) {
		case 'a':
			return CHAR_A;
		case 'd':
			return CHAR_D;
		case 'e':
			return CHAR_E;
		case 'f':
			return CHAR_F;
		case 'g':
			return CHAR_G;
		case 'h':
			return CHAR_H;
		case 'i':
			return CHAR_I;
		case 'k':
			return CHAR_K;
		case 'l':
			return CHAR_L;
		case 'n':
			return CHAR_N;
		case 'o':
			return CHAR_O;
		case 'r':
			return CHAR_R;
		case 's':
			return CHAR_S;
		case 't':
			return CHAR_T;
		case 'u':
			return CHAR_U;
		case ',':
			return CHAR_COMA;
		case '\'':
			return CHAR_APOSTROPHE;
		case '.':
			return CHAR_DOT;
		default:
			return SPRITES[2];
    }
}

void write_line(char* text, int len, long int addr) {
    int i, j;
    unsigned short c;
    for (i = 0; i < len; i++) {
    	c = char_to_addr(text[i]);
		Xil_Out32(addr+4*i, c);
        for (j = 0; j<800000; j++);               //      delay
    }
    return;
}

static void write_introduction() {
    char text[] = "it's dangerous";
    short len = 14;
    long int addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_BASE_ADDRESS + SCR_WIDTH*2 + 1 );
    write_line(text , len, addr);

    strcpy(text, "to go alone.");
    len = 12;
    addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_BASE_ADDRESS + SCR_WIDTH*3 + 1 );
    write_line(text , len, addr);
    int d = 2000000;
    while(d) {d--;}

    strcpy(text, "take this.");
    len = 10;
    addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_BASE_ADDRESS + SCR_WIDTH*4 + 1 );
    write_line(text , len, addr);
    return;
}

void load_frame( direction_t dir ) {
	frame = overworld;
	int x,y;
	long int addr;
	for ( y = 0; y < FRAME_HEIGHT; y++ ) {
		for ( x = 0; x < FRAME_WIDTH; x++ ) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (y+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + x + SIDE_PADDING);
			Xil_Out32( addr, frame[ y * FRAME_WIDTH + x ] );
		}
	}
	//chhar_delete();
	//initialize_enemy(overw_y * overw_x);
	/*if( !inCave ) {
		switch( dir ) {
			case DIR_LEFT:
				overw_x = ( --overw_x < 0 ) ? 0 : overw_x;
				break;
			case DIR_RIGHT:
				overw_x = ( ++overw_x < OVERWORLD_HORIZONTAL ) ? overw_x : OVERWORLD_HORIZONTAL - 1;
				break;
			case DIR_UP:
				overw_y = ( --overw_y < 0 ) ? 0 : overw_y;
				break;
			case DIR_DOWN:
				overw_y = ( ++overw_y < OVERWORLD_VERTICAL )? overw_y : OVERWORLD_VERTICAL - 1;
				break;
			default:
				overw_x = overw_x;
				overw_y = overw_y;
		}

		frame = overworld[ overw_y * OVERWORLD_HORIZONTAL + overw_x ];
		set_minimap();
	} else {
		if ( dir == DIR_DOWN ) {
			frame = overworld[ overw_y * OVERWORLD_HORIZONTAL + overw_x ];
			inCave = false;
			delete_sword(&grandpa);
			chhar_delete();
			if ( overw_x == INITIAL_FRAME_X && overw_y == INITIAL_FRAME_Y ) {
				delete_sword(&sword);
			}
		} else {
			frame = CAVE;
		}
	}*/

    /*    checking if there should be enemies on the current frame     */
    /*int i;
    if (!inCave) {
    	int frame_index = overw_y * OVERWORLD_HORIZONTAL + overw_x;
		for ( i = 0; i < ENEMY_FRAMES_NUM; i++ ){
			if( frame_index == ENEMY_FRAMES[i] ){
				enemy_exists = 1;
				initialize_enemy(frame_index);
				break;
			} else {
				enemy_exists = 0;
			}
		}
    }*/

    /*      loading next frame into memory      */
	//set_frame_palette();



}

/*      setting the correct palette for the current frame     */
void set_frame_palette() {
	long int addr_fill, addr_floor;
	addr_fill = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_COLORS_OFFSET );
	addr_floor = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_COLORS_OFFSET + 1 );

    if( inCave ) {
		/*    red/green/gray -> red    */
		Xil_Out32( addr_fill,  0x0C4CC8 );       
		/*    sand/gray -> black    */
		Xil_Out32( addr_floor, 0x000000 );
        
        return;
    }

	if ((overw_y==2 && (overw_x<3 || overw_x==15)) || (overw_y == 3 && (overw_x<2 || overw_x==3)) || (overw_y==4 && overw_x<2) || (overw_y==5 && overw_x ==0) ) {
		/*    red/green -> white    */
		Xil_Out32( addr_fill, 0x00FCFCFC );
		/*    sand -> gray    */
		Xil_Out32( addr_floor, 0x747474 );
	} else if ((overw_y==3 && (overw_x==12 || overw_x==13)) || (overw_y==4 && (overw_x>5 && overw_x<15)) || ((overw_y==4 || overw_y==5) && (overw_x>3 && overw_x<15)) || (overw_y==7 && (overw_x>3 && overw_x<9)) || (overw_y==6 && overw_x > 3 && overw_x <15) ) {
		/*    red/white -> green    */
		Xil_Out32( addr_fill, 0x00A800 );
		/*    gray -> sand    */
		Xil_Out32( addr_floor, 0xA8D8FC ); 
	} else {
		/*    green/white -> red    */
		Xil_Out32( addr_fill, 0x0C4CC8 );
		/*    gray -> sand    */
		Xil_Out32( addr_floor, 0xA8D8FC ); 
	}

}

void set_grandpa() {
	grandpa.x = (SIDE_PADDING + FRAME_WIDTH / 2 - 1) * SPRITE_SIZE + SPRITE_SIZE / 2;
	grandpa.y = (VERTICAL_PADDING + HEADER_HEIGHT + FRAME_HEIGHT / 2) * SPRITE_SIZE;
	chhar_spawn(&grandpa, 0);
}

/*		set sword in cave		*/
void set_sword() {
	int sword_rotation = 2;			//

	sword.x = (SIDE_PADDING + FRAME_WIDTH / 2 - 1) * SPRITE_SIZE + SPRITE_SIZE / 2;
	sword.y = (VERTICAL_PADDING + HEADER_HEIGHT + FRAME_HEIGHT / 2 + 1) * SPRITE_SIZE + SPRITE_SIZE / 2;

	chhar_spawn( &sword, sword_rotation );
}

void pick_up_sword() {
	sword.active = true;
	//sword.x = link.x + 13;
	//sword.y = link.y;
	delete_sword(&sword);
}

void set_minimap() {
	int i,j, pos = HEADER_BASE_ADDRESS + 2*SCR_WIDTH + 1;
	int m_x, m_y;
	unsigned long addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
	for(j = 0 ; j < 2 ; j++ ) {
		for(i = 0; i < 4; i++) {
			Xil_Out32(addr+4*(j * SCR_WIDTH + i),	MINIMAP_BLANK);
		}
	}


	/*		reset sprite	*/
	for (i = 0; i < 64; i++) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (MINIMAP_BLANK + 64 + i);
		Xil_Out32(addr,	0x07070707);
	}

	/*		relative position inside ine sprite		*/
	m_x = overw_x % 4;
	m_y = overw_y % 4;

	for (i = 0; i < 4; i++) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (MINIMAP_BLANK + 64 + m_y*16 + (i)*4 + m_x);
		Xil_Out32(addr,	0x18181818);
	}

	/*		the position of the sprite in minimap		*/
	m_x = overw_x / 4;
	m_y = overw_y / 4;

	pos = HEADER_BASE_ADDRESS + (2 + m_y)*SCR_WIDTH + 1 + m_x;
	addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
	Xil_Out32(addr,	MINIMAP_BLANK + 64);
}

void set_pickups() {

	int i, pos = HEADER_BASE_ADDRESS + 2*SCR_WIDTH + 5;
	unsigned long addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;

	if ( rupees < 5 ) {
		for (i = 0; i < rupees; i++) {
			pos++;
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
			Xil_Out32(addr,	RUPEE_SPRITE);
		}
	}

	pos = HEADER_BASE_ADDRESS + 3*SCR_WIDTH + 5;
	addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
	if ( bombs < 5 ) {
		for (i = 0; i < bombs; i++) {
			pos++;
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
			Xil_Out32(addr,	BOMB_SPRITE);
		}
	}

}

void set_health(int health) {
	int i;
	int pos = HEADER_BASE_ADDRESS + 3*SCR_WIDTH + FRAME_WIDTH - 6;
	unsigned long addr;
	if (health < 0) {
		health = 0;
		HEALTH = 0;
	}
	for(i = 0; i < MAX_HEALTH/2; i++, pos++) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
		if (i < health/2) {
			Xil_Out32(addr,	HEART_FULL);
		} else {
			Xil_Out32(addr,	HEART_EMPTY);
		}
	}

	if (health % 2) {
		pos -= ((MAX_HEALTH - health) / 2);
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
		Xil_Out32(addr,	HEART_HALF);
	}
}

void set_header() {
	set_minimap();
	set_pickups();

	/*			print "LIFE"		*/		//		use write line
	int pos = HEADER_BASE_ADDRESS + 2*SCR_WIDTH + FRAME_WIDTH - 6;
	unsigned long addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
    write_line("life", 4, addr);

	/*			put hearts under life		*/
    set_health(HEALTH);

}

bool initialize_enemy( int frame_index) {

	//TODO:	define function
	/* set enemy on a random position 
	 * check if there is an obstacle on that position
	 * this function is only for initializing the enemy!
	 * enemy movement logic will be defined in other function
	 * the enemy's position should depend on the frame
	 * in other words, it will use overw_x and overw_y
*/
	/*flame.active = true;
	octorok2.active = true;
	octorok3.active = true;
	octorok4.active = true;
	if(frame_index == 120 || frame_index == 102 || frame_index == 100){
		ghost.active = false;
		flame.x = 240;
		flame.y = 220;
		flame.dir = DIR_LEFT;
		chhar_spawn(&flame, 0);

		octorok2.x = 320;
		octorok2.y = 240;
		octorok2.dir = DIR_UP;
		chhar_spawn(&octorok2, 3);

		octorok3.x = 320;
		octorok3.y = 290;
		octorok3.dir = DIR_DOWN;
		chhar_spawn(&octorok3, 0);

		octorok4.x = 355;
		octorok4.y = 250;
		octorok4.dir = DIR_RIGHT;
		chhar_spawn(&octorok4, 0);
		return 1;
	} else if ( frame_index == 104 || frame_index == 103) {
		ghost.active = false;
		flame.x = 250;
		flame.y = 220;
		flame.dir = DIR_DOWN;
		chhar_spawn(&flame, 0);

		octorok2.x = 320;
		octorok2.y = 240;
		octorok2.dir = DIR_LEFT;
		chhar_spawn(&octorok2, 3);

		octorok3.x = 320;
		octorok3.y = 290;
		octorok3.dir = DIR_DOWN;
		chhar_spawn(&octorok3, 0);

		octorok4.x = 350;
		octorok4.y = 250;
		octorok4.dir = DIR_UP;
		chhar_spawn(&octorok4, 0);
		return 1;
	} else if (frame_index == 64 || frame_index == 65 || frame_index == 48 ||
			frame_index == 49 || frame_index == 32 || frame_index == 33) {
		flame.active = false;
		octorok2.active = false;
		octorok3.active = false;
		octorok4.active = false;
		ghost.active = true;

		ghost.x = 220;
		ghost.y = 220;
		ghost.dir = DIR_LEFT;
		chhar_spawn(&ghost,0);
		return 1;
	}else {
		enemy_exists = 0;
		return 0;
	}
*/

}


direction_t random_direction(direction_t dir, int divider){
	 divider = random_number();
	 int rnd = random_number() % 2; //returns 0-100 000

	 if (rnd % 4 == 0){
		 dir = dir == DIR_LEFT ? DIR_UP : DIR_LEFT;
	 }else{

	 }

	 return dir;
}

void ghost_move(characters* chhar, int divider){
	int x,y;
	static int delay;
	x = chhar->x;
	y = chhar->y;

	if (delay == 3) {
		delay = 0;
		if( chhar->y < (HEADER_HEIGHT + VERTICAL_PADDING + 2) * SPRITE_SIZE){
			chhar->dir = DIR_DOWN;
		} else if (chhar->y > (VERTICAL_PADDING + FRAME_HEIGHT + HEADER_HEIGHT - 1 - 2) * SPRITE_SIZE) {
			chhar->dir = DIR_UP;
		} else if (chhar->x > (( SIDE_PADDING + FRAME_WIDTH - 3) * SPRITE_SIZE  - SPRITE_SIZE - 10)){
			chhar->dir = DIR_LEFT;
		} else if ( chhar->x < (SIDE_PADDING + 2) * SPRITE_SIZE ){
			chhar->dir = DIR_RIGHT;
		}

		if(chhar->dir == DIR_DOWN) {
					y++;
		} else if (chhar->dir == DIR_UP){
					y--;
		} else if (chhar->dir == DIR_LEFT){
					x--;
		} else if (chhar->dir == DIR_RIGHT){
					x++;
		}

		chhar->y = y;
		chhar->x = x;

		if (chhar->dir == DIR_RIGHT)
			chhar->sprite = GHOST_SPRITES_OFFSET + 64;
		else
			chhar->sprite = GHOST_SPRITES_OFFSET;

		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F000000 | (unsigned int) chhar->sprite);

		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
				( chhar->y << 16) | chhar->x);
		return;
	} else {
		delay++;
	}
}

void enemy_move(characters* chhar, int divider){
	int x,y;
	int obstackle,rnd;
	static int a1=0,a2=0,b1=0, c2=0;
	x = chhar->x;
	y = chhar->y;
	if(chhar->dir == DIR_DOWN) {
				y++;
	} else if (chhar->dir == DIR_UP){
				y--;
	} else if (chhar->dir == DIR_LEFT){
				x--;
	} else if (chhar->dir == DIR_RIGHT){
				x++;
	}

	switch(divider){
		case 1: //first enemy
			if(x==enemy_obstackles1[2][0] && y==enemy_obstackles1[2][1]){ //bottom of ladder
				if(a1 == 0){
					chhar->dir = DIR_RIGHT;
					chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
					a1++;
				}else if(a1 == 1){
					chhar->dir = DIR_UP;

					a1++;
				}else if(a1 == 2){
					a1 = 0;
					chhar->dir = DIR_LEFT;
					chhar->sprite = FLAME_SPRITE_OFFSET_LEFT;
				}
			}else if((x==enemy_obstackles1[1][0] && y==enemy_obstackles1[1][1]) || (x==enemy_obstackles1[4][0] && y==enemy_obstackles1[4][1])){//right edge
				chhar->dir = DIR_LEFT;
				chhar->sprite = FLAME_SPRITE_OFFSET_LEFT;
			}else if(x==enemy_obstackles1[0][0] && y==enemy_obstackles1[0][1]){//left edge
				chhar->dir = DIR_RIGHT;
				chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
			}else if(x==enemy_obstackles1[3][0] && y==enemy_obstackles1[3][1]){
				if(a2==0){
					chhar->dir = DIR_RIGHT;
					chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
					a2++;
				}else if(a2 == 1){
					chhar->dir = DIR_DOWN;
					a2 = 0;
				}
			}
			break;
		case 2:
			if(x==enemy_obstackles2[0][0] && y==enemy_obstackles2[0][1]){
				chhar->dir = DIR_RIGHT;
				chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
			}else if(x==enemy_obstackles2[1][0] && y==enemy_obstackles2[1][1]){
				chhar->dir = DIR_LEFT;
				chhar->sprite = FLAME_SPRITE_OFFSET_LEFT;
			}
			break;
		case 3:
			if(x==enemy_obstackles3[0][0] && y==enemy_obstackles3[0][1]){
				chhar->dir = DIR_RIGHT;
				chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
			}else if(x==enemy_obstackles3[1][0] && y==enemy_obstackles3[1][1]){
				chhar->dir = DIR_LEFT;
				chhar->sprite = FLAME_SPRITE_OFFSET_LEFT;
			}
			break;
		case 4:
			if((x==enemy_obstackles4[0][0] && y==enemy_obstackles4[0][1]) || (x==enemy_obstackles4[3][0] && y==enemy_obstackles4[3][1])){//right edge
				chhar->dir = DIR_RIGHT;
				chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
			}else if((x==enemy_obstackles4[1][0] && y==enemy_obstackles4[1][1]) || (x==enemy_obstackles4[4][0] && y==enemy_obstackles4[4][1])){//left edge
				chhar->dir = DIR_LEFT;
				chhar->sprite = FLAME_SPRITE_OFFSET_LEFT;
			}else if(x==enemy_obstackles4[2][0] && y==enemy_obstackles4[2][1]){//bottom of ladder
				if(b1 == 0){
					chhar->dir = DIR_RIGHT;
					chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
					b1++;
				}else if(b1 == 1){
					chhar->dir = DIR_UP;
					b1++;
				}else if(b1 == 2){
					b1 = 0;
					chhar->dir = DIR_LEFT;
					chhar->sprite = FLAME_SPRITE_OFFSET_LEFT;
				}
			}else if(x==enemy_obstackles4[5][0] && y==enemy_obstackles4[5][1]){
				if(c2 == 0){
					chhar->dir = DIR_RIGHT;
					chhar->sprite = FLAME_SPRITE_OFFSET_RIGHT;
					c2++;
				}else if(c2 == 1){
					chhar->dir = DIR_LEFT;
					chhar->sprite = FLAME_SPRITE_OFFSET_LEFT;
					c2++;
				}else if(c2 == 2){
					c2 = 0;
					chhar->dir = DIR_DOWN;
				}
			}//int enemy_obstackles4[6][2] = {{400, 288},{464, 288},{432, 288}, {400, 248},{464,248}, {432, 248}};

	}






	//obstackle = obstackles_detection(x, y, frame, chhar->dir, false);



	chhar->x = x;
	chhar->y = y;
	chhar_spawn(chhar, 0);
	/*if(chhar->dir == DIR_DOWN){
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F000000 | (unsigned int) chhar->sprite);
	} else if (chhar->dir == DIR_RIGHT){
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F010000 | (unsigned int) chhar->sprite);
	} else if (chhar->dir == DIR_UP){
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F020000 | (unsigned int) chhar->sprite);
	} else {
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F100000 | (unsigned int) chhar->sprite);
	}

	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			( chhar->y << 16) | chhar->x);*/

}

void chhar_delete(){
	delete_sword(&flame1);
	delete_sword(&flame2);
	delete_sword(&flame3);
	delete_sword(&flame4);
	delete_sword(&ghost);
}

void chhar_spawn( characters * chhar, int rotation ) {
	/*if ( rotation == 1 ) {																			 //rotate 90degrees clockwise
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F100000 | (unsigned int) chhar->sprite );
	} else if ( rotation == 2 ) { 																	//rotate 90degrees aniclockwise
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F010000 | (unsigned int) chhar->sprite );
	} else if ( rotation == 3 ) { 																	//rotate 180degrees
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int) 0x8F020000 | (unsigned int) chhar->sprite );
	}else {					    																	//no rotation
		Xil_Out32(
						XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
						(unsigned int) 0x8F000000 | (unsigned int) chhar->sprite );
	}*/

	Xil_Out32(
					XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
					(unsigned int) 0x8F000000 | (unsigned int) chhar->sprite );
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x );      //  the higher 2 bytes represent the row (y)
}

void delete_sword( characters* chhar ){
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
			(unsigned int) 0x80000000 | (unsigned int) chhar->sprite );
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x );      //  the higher 2 bytes represent the row (y)
}

/*  cleaning the registers used for moving characters sprites; two registers are used for each sprite   */
void reset_memory() {
	unsigned int i;
	long int addr;

	for( i = 0; i < SCR_WIDTH*SCR_HEIGHT; i++ ) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( SCREEN_BASE_ADDRESS + i );
		Xil_Out32( addr, SPRITES[6] );             // SPRITES[6] is a black square
	}

	/*for ( i = 0; i <= 20; i += 2 ) {
		Xil_Out32( XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ), (unsigned int) 0x0F000000);
	}*/
}
bool acceptable_position_up(int x, int y){

	int i;
	for(i=0; i<LADDERS_NUM; i++){
		if(x>=ladders_up[i][0]-4 && x<=ladders_up[i][0] + 4 && y==ladders_up[i][1]){
			return true;
		}
	}
	return false;
}

bool acceptable_position_down(int x, int y){

	int i;
	for(i=0; i<LADDERS_NUM; i++){
		if(x>=ladders_down[i][0]-4 && x<=ladders_down[i][0] + 4 && y==ladders_down[i][1]){
			return true;
		}
	}
	return false;
}

bool check_edges(int x, int y){
	int i;
		for(i=0; i<EDGE_NUM; i++){

			if(x>=160 && x<=edges[i][0] && y==edges[i][1] && i<5){
				return true;
			}else if(x>=edges[i][0] && x<=470 && y==edges[i][1] && i>=11 ){
				return true;
			}else if(x>=edges[i][0]-2 && x<=edges[i][0]+2 && y==edges[i][1]){
				return true;
			}
		}
		return false;
}
bool link_move(characters * link, characters* sword, direction_t dir) {
	unsigned int x;
	unsigned int y;
	int sword_rotation = 0;
	int lasting_attack = 0;
	int i,j;
	int blocked_sword = 0; //0 false - not blocked, 1 true - blocked
	unsigned int buttons;

	x = link->x;
	y = link->y;
	if(on_ladder){
		if ( dir == DIR_UP ) {
				y--;
				ladder_count++;

			    if(ladder_count == 40 || ladder_count == 0){
			    	on_ladder = false;
			    	link->sprite =  1791;
			    	ladder_count = 0;
			    	pos = 1;
			    }else{
			    	if(link->sprite ==  2239)
						link->sprite =  2303;
					else if(link->sprite ==  2303)
						link->sprite =  2239;
					counter++;

			    }


		} else if ( dir == DIR_DOWN) {
				y++;
				ladder_count--;
				if(ladder_count == 0 || ladder_count == -40){
					on_ladder = false;
					link->sprite =  1791;
					ladder_count = 0;
					pos = 1;
				}else{
					if(link->sprite ==  2239)
						link->sprite =  2303;
					else if(link->sprite ==  2303)
						link->sprite =  2239;
					counter++;
				}

		}
	}else if ( dir == DIR_LEFT ) {
			if(x==160){
				x=160;
			}else{
				x--;
			}
			if(pos == 1){
				pos = 0;
				link->sprite =  2047;
			}
			if(link->sprite ==  2047)
				link->sprite =  2111;
			else if(link->sprite ==  2111)
				link->sprite =  2047;

			while(check_edges(x,y)){
				for(i =0; i<40; i++){
					y++;
					for(j = 0; j<100000; j++);
					buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );
					enemy_move(&flame1, 1);
					enemy_move(&flame2, 2);
					enemy_move(&flame3, 3);
					enemy_move(&flame4, 4);
					direction_t d = DIR_STILL;
					if ( BTN_LEFT(buttons) ) {
						d = DIR_LEFT;
					} else if ( BTN_RIGHT(buttons) ) {
						d = DIR_RIGHT;
					} else if ( BTN_UP(buttons) ) {
						d = DIR_UP;
					} else if ( BTN_DOWN(buttons) ) {
						d = DIR_DOWN;
					} else if ( BTN_SHOOT(buttons) ) {
						d = DIR_ATTACK;
					}
					if ( d == DIR_LEFT ) {

								if(x==160){
									x=160;
								}else{
									x--;
								}
								if(pos == 1){
									pos = 0;
									link->sprite =  2047;
								}
								if(link->sprite ==  2047)
									link->sprite =  2111;
								else if(link->sprite ==  2111)
									link->sprite =  2047;
								counter++;
					} else if ( d == DIR_RIGHT ) {
								x++;
								/*if ( counter % LINK_STEP == 0 ) {
									last = (last == 3) ? 4 : 3;
								}
								lasting_attack = 0;*/
								if(pos == 0){
									pos = 1;
									link->sprite =  1791;
								}
								if(link->sprite ==  1727)
									link->sprite =  1791;
								else if(link->sprite ==  1791)
									link->sprite =  1727;
								counter++;
					}
					link->x = x;
					link->y = y;

					chhar_spawn( link, 0 );
				}
			}
	} else if ( dir == DIR_RIGHT) {
			if(x==470){
				x=470;
			}else{
				x++;
			}

			if(pos == 0){
				pos = 1;
				link->sprite =  1791;
			}
			if(link->sprite ==  1727)
				link->sprite =  1791;
			else if(link->sprite ==  1791)
				link->sprite =  1727;

			while(check_edges(x,y)){
					for(i =0; i<40; i++){
						y++;
						for(j = 0; j<100000; j++);
						buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );
						enemy_move(&flame1, 1);
						enemy_move(&flame2, 2);
						enemy_move(&flame3, 3);
						enemy_move(&flame4, 4);
						direction_t d = DIR_STILL;
						if ( BTN_LEFT(buttons) ) {
							d = DIR_LEFT;
						} else if ( BTN_RIGHT(buttons) ) {
							d = DIR_RIGHT;
						} else if ( BTN_UP(buttons) ) {
							d = DIR_UP;
						} else if ( BTN_DOWN(buttons) ) {
							d = DIR_DOWN;
						} else if ( BTN_SHOOT(buttons) ) {
							d = DIR_ATTACK;
						}
						if ( d == DIR_LEFT ) {
									x--;

									if(pos == 1){
										pos = 0;
										link->sprite =  2047;
									}
									if(link->sprite ==  2047)
										link->sprite =  2111;
									else if(link->sprite ==  2111)
										link->sprite =  2047;
									counter++;
						} else if ( d == DIR_RIGHT ) {
									if(x==470){
										x=470;
									}else{
										x++;
									}
									if(pos == 0){
										pos = 1;
										link->sprite =  1791;
									}
									if(link->sprite ==  1727)
										link->sprite =  1791;
									else if(link->sprite ==  1791)
										link->sprite =  1727;
									counter++;
						}
						link->x = x;
						link->y = y;

						chhar_spawn( link, 0 );
					}
				}
	} else if ( dir == DIR_UP && !acceptable_position_up(x,y)) {
			for(i =0; i<16; i++){
				y--;
				for(j = 0; j<100000; j++);
				enemy_move(&flame1, 1);
				enemy_move(&flame2, 2);
				enemy_move(&flame3, 3);
				enemy_move(&flame4, 4);
				buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

				direction_t d = DIR_STILL;
				if ( BTN_LEFT(buttons) ) {
					d = DIR_LEFT;
				} else if ( BTN_RIGHT(buttons) ) {
					d = DIR_RIGHT;
				} else if ( BTN_UP(buttons) ) {
					d = DIR_UP;
				} else if ( BTN_DOWN(buttons) ) {
					d = DIR_DOWN;
				} else if ( BTN_SHOOT(buttons) ) {
					d = DIR_ATTACK;
				}
				if ( d == DIR_LEFT ) {
							x--;
							/*if ( counter % LINK_STEP == 0 ) {
								last = ( last == 16 ) ? 17 : 16;
							}
							lasting_attack = 0;*/
							if(pos == 1){
								pos = 0;
								link->sprite =  2047;
							}
							if(link->sprite ==  2047)
								link->sprite =  2111;
							else if(link->sprite ==  2111)
								link->sprite =  2047;
							counter++;
				} else if ( d == DIR_RIGHT ) {
							x++;
							/*if ( counter % LINK_STEP == 0 ) {
								last = (last == 3) ? 4 : 3;
							}
							lasting_attack = 0;*/
							if(pos == 0){
								pos = 1;
								link->sprite =  1791;
							}
							if(link->sprite ==  1727)
								link->sprite =  1791;
							else if(link->sprite ==  1791)
								link->sprite =  1727;
							counter++;
				}
				for(j = 0; j<100000; j++);
				enemy_move(&flame1, 1);
				enemy_move(&flame2, 2);
				enemy_move(&flame3, 3);
				enemy_move(&flame4, 4);
				link->x = x;
				link->y = y;

				if(collision(&link, flame1, flame2, flame3, flame4)){
					mario.x = INITIAL_MARIO_POSITION_X;
					mario.y = INITIAL_MARIO_POSITION_Y;
					mario.dir = DIR_LEFT;
					mario.sprite = 1791;
					pos = 1;
				}


				chhar_spawn( link, 0 );
			}

			for(i =0; i<16; i++){
				y++;
				for(j = 0; j<100000; j++);
				enemy_move(&flame1, 1);
				enemy_move(&flame2, 2);
				enemy_move(&flame3, 3);
				enemy_move(&flame4, 4);
				buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );


				direction_t d = DIR_STILL;
				if ( BTN_LEFT(buttons) ) {
					d = DIR_LEFT;
				} else if ( BTN_RIGHT(buttons) ) {
					d = DIR_RIGHT;
				} else if ( BTN_UP(buttons) ) {
					d = DIR_UP;
				} else if ( BTN_DOWN(buttons) ) {
					d = DIR_DOWN;
				} else if ( BTN_SHOOT(buttons) ) {
					d = DIR_ATTACK;
				}
				if ( d == DIR_LEFT ) {
							x--;
							/*if ( counter % LINK_STEP == 0 ) {
								last = ( last == 16 ) ? 17 : 16;
							}
							lasting_attack = 0;*/
							if(pos == 1){
								pos = 0;
								link->sprite =  2047;
							}
							if(link->sprite ==  2047)
								link->sprite =  2111;
							else if(link->sprite ==  2111)
								link->sprite =  2047;
							counter++;
				} else if ( d == DIR_RIGHT ) {
							x++;
							/*if ( counter % LINK_STEP == 0 ) {
								last = (last == 3) ? 4 : 3;
							}
							lasting_attack = 0;*/
							if(pos == 0){
								pos = 1;
								link->sprite =  1791;
							}
							if(link->sprite ==  1727)
								link->sprite =  1791;
							else if(link->sprite ==  1791)
								link->sprite =  1727;
							counter++;
				}
				for(j = 0; j<100000; j++);
				enemy_move(&flame1, 1);
				enemy_move(&flame2, 2);
				enemy_move(&flame3, 3);
				enemy_move(&flame4, 4);
				link->x = x;
				link->y = y;

				if(collision(&link, flame1, flame2, flame3, flame4)){
									mario.x = INITIAL_MARIO_POSITION_X;
									mario.y = INITIAL_MARIO_POSITION_Y;
									mario.dir = DIR_LEFT;
									mario.sprite = 1791;
									pos = 1;
				}

				chhar_spawn( link, 0 );
			}

			while(check_edges(x,y)){
				for(i =0; i<40; i++){
					y++;
					for(j = 0; j<100000; j++);
					buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );
					enemy_move(&flame1, 1);
					enemy_move(&flame2, 2);
					enemy_move(&flame3, 3);
					enemy_move(&flame4, 4);
					direction_t d = DIR_STILL;
					if ( BTN_LEFT(buttons) ) {
						d = DIR_LEFT;
					} else if ( BTN_RIGHT(buttons) ) {
						d = DIR_RIGHT;
					} else if ( BTN_UP(buttons) ) {
						d = DIR_UP;
					} else if ( BTN_DOWN(buttons) ) {
						d = DIR_DOWN;
					} else if ( BTN_SHOOT(buttons) ) {
						d = DIR_ATTACK;
					}
					if ( d == DIR_LEFT ) {
								x--;

								if(pos == 1){
									pos = 0;
									link->sprite =  2047;
								}
								if(link->sprite ==  2047)
									link->sprite =  2111;
								else if(link->sprite ==  2111)
									link->sprite =  2047;
								counter++;
					} else if ( d == DIR_RIGHT ) {
								if(x==470){
									x=470;
								}else{
									x++;
								}
								if(pos == 0){
									pos = 1;
									link->sprite =  1791;
								}
								if(link->sprite ==  1727)
									link->sprite =  1791;
								else if(link->sprite ==  1791)
									link->sprite =  1727;
								counter++;
					}
					link->x = x;
					link->y = y;

					chhar_spawn( link, 0 );
				}
			}

			/*if ( counter % LINK_STEP == 0 ) {
				last = (last == 2) ? 15 : 2;
			}
			lasting_attack = 0;*/

			/*link->sprite = 1791;
			link->sprite =  2047;*/
			//counter++;
	} else if (  dir == DIR_UP && acceptable_position_up(x,y) ) {
			y--;
			on_ladder = true;
			ladder_count++;
			link->sprite =  2239;
	}else if(dir == DIR_DOWN && acceptable_position_down(x,y) ){
		y++;
		ladder_count--;
		on_ladder = true;
		link->sprite =  2239;
	}
	link->x = x;
	link->y = y;

	chhar_spawn( link, 0 );

	for(i =0; i<100000; i++);

	//+/-28 instead of 32 because of sprite graphic
	/*if ((link->x >= (( SIDE_PADDING + FRAME_WIDTH ) * SPRITE_SIZE - 28) && link->sprite ==  LINK_SPRITES_OFFSET + 64 * 3) || //if right edge and link is right faced
		(link->x >= (( SIDE_PADDING + FRAME_WIDTH ) * SPRITE_SIZE - 28) && link->sprite ==  LINK_SPRITES_OFFSET + 64 * 4) ||
		((link->x < SIDE_PADDING * SPRITE_SIZE + 20) && link->sprite == LINK_SPRITES_OFFSET + 64 * 16) || //if left edge and link is left faced
		((link->x < SIDE_PADDING * SPRITE_SIZE + 20) && link->sprite == LINK_SPRITES_OFFSET + 64 * 17) ||
		((link->y > ( VERTICAL_PADDING + FRAME_HEIGHT + HEADER_HEIGHT - 1) * SPRITE_SIZE  - 16) && link->sprite ==  LINK_SPRITES_OFFSET + 64 * 0) || //if down edge and link is up faced
		((link->y > ( VERTICAL_PADDING + FRAME_HEIGHT + HEADER_HEIGHT - 1) * SPRITE_SIZE  - 16) && link->sprite ==  LINK_SPRITES_OFFSET + 64 * 1) ||
		((link->y < SIDE_PADDING * SPRITE_SIZE + 16) && link->sprite == LINK_SPRITES_OFFSET + 64 * 2) || //if up edge and link is up faced
		((link->y < SIDE_PADDING * SPRITE_SIZE + 16) && link->sprite == LINK_SPRITES_OFFSET + 64 * 15) )
	{
		blocked_sword = 1;
	}*/

	/*if(!sword->active && inCave && ((sword->x-8 < link->x) && (link->x < sword->x+8))
			&& ((sword->y-8 < link->y)&& (link->y < sword->y))
			&&  overw_x == INITIAL_FRAME_X && overw_y == INITIAL_FRAME_Y )
	{
		pick_up_sword();
	}*/

	/*      change frame if on the edge     */
    /*if (link->x > ( ( SIDE_PADDING + FRAME_WIDTH ) * SPRITE_SIZE  - SPRITE_SIZE)){
    	link->x = overw_x == OVERWORLD_HORIZONTAL - 1? link->x-1 : SIDE_PADDING * SPRITE_SIZE;
    	load_frame( DIR_RIGHT );
    	return false;
	}
    if ( link->y > ( VERTICAL_PADDING + FRAME_HEIGHT + HEADER_HEIGHT - 1 ) * SPRITE_SIZE ) {
    	if( inCave ) {
    		link->x = ( SIDE_PADDING + door_x ) * SPRITE_SIZE;
    		link->y = ( VERTICAL_PADDING + HEADER_HEIGHT + door_y ) * SPRITE_SIZE + 15;
    	} else {
    		link->y = overw_y == OVERWORLD_VERTICAL - 1 ? link->y - 1 : ( HEADER_HEIGHT + VERTICAL_PADDING ) * SPRITE_SIZE;
    	}
    	load_frame( DIR_DOWN );
    	return false;
    }
    if ( link->y < SIDE_PADDING * SPRITE_SIZE ) {
    	link->y = overw_y == 0 ? link->y+1 : ( HEADER_HEIGHT + VERTICAL_PADDING + FRAME_HEIGHT - 1 ) * SPRITE_SIZE;
    	load_frame( DIR_UP );
		return false;
    }
    if ( link->x < SIDE_PADDING * SPRITE_SIZE ) {
    	link->x = overw_x == 0 ? link->x + 1 : ( SIDE_PADDING + FRAME_WIDTH - 1 ) * SPRITE_SIZE;
    	load_frame( DIR_LEFT );
		return false;
	}*/

    /*      get the current position of link    */
	/*x = mario->x;
	y = mario->y;*/

	/*      movement animation      */


		//kill enemy
		/*if(((flame.x-8 < sword->x) && (sword->x < flame.x+8))
			&& ((flame.y-8 < sword->y)&& (sword->y < flame.y+8)) && flame.active)
		{
				flame.active = false;
				delete_sword(&flame);
				rupees++;
				set_pickups();
		} else if(((octorok2.x-8 < sword->x) && (sword->x < octorok2.x+8))
				&& ((octorok2.y-8 < sword->y)&& (sword->y < octorok2.y+8)) && octorok2.active)
		{
				octorok2.active = false;
				delete_sword(&octorok2);
				rupees++;
				set_pickups();
		} else if(((octorok3.x-8 < sword->x) && (sword->x < octorok3.x+8))
				&& ((octorok3.y-8 < sword->y)&& (sword->y < octorok3.y+8)) && octorok3.active)
		{
				octorok3.active = false;
				delete_sword(&octorok3);
				bombs++;
				set_pickups();
		} else if(((octorok4.x-8 < sword->x) && (sword->x < octorok4.x+8))
				&& ((octorok4.y-8 < sword->y)&& (sword->y < octorok4.y+8)) && octorok4.active)
		{
				octorok4.active = false;
				delete_sword(&octorok4);
				bombs++;
				set_pickups();
		}*/


		/*if ( lasting_attack != 1 ){
			Xil_Out32(
					XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_l ),
					(unsigned int) 0x8F000000 | (unsigned int) link->sprite);
			Xil_Out32(
					XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_h ),
					 ( link->y << 16) | link->x);      //  the higher 2 bytes represent the row (y)
		}*/

		/*if (blocked_sword == 0) {
			for ( i =0; i <90000; i++ );             //      delay

			if ( lasting_attack != 1 ){
				chhar_spawn( sword, sword_rotation );
			}
			if ( dir == DIR_ATTACK) {
				lasting_attack = 1;
			}

			for ( i =0; i <15000; i++);             //      delay
		}*/
		/*   After a short break (representing the attack animation), go back to standing sprite facing the same direction    */
		/*if ( last ==16 || last == 17 ) { 						//left
			link->sprite = LINK_SPRITES_OFFSET + 64 * 17;
		} else if ( last == 3 || last == 4 ) { 				//right
			link->sprite = LINK_SPRITES_OFFSET + 64 * 4;
		} else if ( last == 2 || last == 15 ) {				//up
			link->sprite = LINK_SPRITES_OFFSET + 64 * 15;
		} else if ( last == 0 || last == 1 ) {				//down
			link->sprite = LINK_SPRITES_OFFSET + 64 * 1;
		}
	}*/

    /*if( !inCave && isDoor(x, y) ) {
        link->x = ( SIDE_PADDING + (int) FRAME_WIDTH/2 - 1 ) * SPRITE_SIZE + SPRITE_SIZE / 2;                         // set to the middle of the frame
        link->y = ( VERTICAL_PADDING + HEADER_HEIGHT + FRAME_HEIGHT - 1 ) * SPRITE_SIZE;        //set to the bottom of the cave
        chhar_spawn(link, 0);
        inCave = true;
        load_frame( DIR_UP );
        if(overw_x == INITIAL_FRAME_X && overw_y == INITIAL_FRAME_Y) {
        	set_fire();
        	if(!sword->active){
				set_sword();
		        set_grandpa();
				write_introduction();
			}
		}*/
	    /*		skip collision detection if on the bottom of the frame 			*/
    /*} else if( dir == DIR_DOWN && y == (( VERTICAL_PADDING + FRAME_HEIGHT + HEADER_HEIGHT - 1 ) * SPRITE_SIZE + 1)) {
		link->x = x;
		link->y = y;
	} else {
		if ( !obstackles_detection(x, y, frame, dir, true ) ) {
			link->x = x;
			link->y = y;
		}
		if (inCave) {
			if (link->y < (VERTICAL_PADDING + HEADER_HEIGHT + FRAME_HEIGHT / 2 + 1) * SPRITE_SIZE ) {
				link->y++;
			}
		}
	}

	if ( lasting_attack != 1 ){
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_l ),
				(unsigned int) 0x8F000000 | (unsigned int) link->sprite );
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_h ),
				(link->y << 16) | link->x );      //  the higher 2 bytes represent the row (y)
		for ( i = 0; i < 100000; i++ );         //  delay
		if (sword->active) {
			//for ( i = 0; i < 100000; i++ );         //  delay
			delete_sword( sword );
		}
	}

	for (i = 0; i < 1000; i++);          //     delay = 1000 <- good speed

	if(inCave) {
		set_fire();
		for ( i = 0; i < 10000; i++ );         //  delay
	}

	return false;*/
}

bool isDoor(int x, int y) {
    /*      calculate the index of the position in the frame     */
	x = x + 10 - SIDE_PADDING*SPRITE_SIZE;
	y = y + 14 - (VERTICAL_PADDING + HEADER_HEIGHT) * SPRITE_SIZE;
    x /= SPRITE_SIZE;
    y /= SPRITE_SIZE;
    
    /*      check if link reached the door      */
    if (  frame[y * FRAME_WIDTH + x] == SPRITES[10] ) {
        door_x = x;
    	door_y = y;
        return true;
    }

    return false;
}

bool tile_walkable(int index, unsigned short* map_frame) {
	int i;

	for ( i = 0; i < 20; i++) {
        /*      check if the current sprite is walkable     */
		if (  map_frame[index] == SPRITES[walkables[i]] ){ 
			return true;
		}
	}

	return false;
}

int obstackles_detection(int x, int y, unsigned short* f, int dir, bool isLink) {
	int i;

	for(i=0; i<LADDERS_NUM; i++){
		if(x==ladders_up[i][0] && y == ladders_up[i][1]){
			return 0; //donji deo merdevina
		}else if(x == ladders_down[i][0] && y == ladders_down[i][1]){
			return 1; //gornji deo merdevina
		}
	}

	for(i=0; i<24; i++){

		if(x==end_of_block[i][0] && y==end_of_block[i][1] && i<12){
			return 2; //leva ivica
		}
		else if(x==end_of_block[i][0] && y==end_of_block[i][1] && i>=12){
			return 3; //desna ivica
		}
	}

	return 4;



	/*if (isLink) {
		x_left = x + 3 - SIDE_PADDING * SPRITE_SIZE;
		x_right = x + 12 - SIDE_PADDING * SPRITE_SIZE;
		y_top = y + 11 - (VERTICAL_PADDING + HEADER_HEIGHT) * SPRITE_SIZE;
		y_bot = y + 15 - (VERTICAL_PADDING + HEADER_HEIGHT) * SPRITE_SIZE;
	} else {
		x_left = x + 1 - SIDE_PADDING * SPRITE_SIZE;
		x_right = x + 15 - SIDE_PADDING * SPRITE_SIZE;
		y_top = y + 1 - (VERTICAL_PADDING + HEADER_HEIGHT) * SPRITE_SIZE;
		y_bot = y + 15 - (VERTICAL_PADDING + HEADER_HEIGHT) * SPRITE_SIZE;

	}

	x_left /= SPRITE_SIZE;
	x_right /= SPRITE_SIZE;
	y_top /= SPRITE_SIZE;
	y_bot /= SPRITE_SIZE;

	if ( dir == DIR_UP ) {
		return !( tile_walkable(x_left + y_top * FRAME_WIDTH, f) && tile_walkable(x_right + y_top * FRAME_WIDTH, f) );
	} else if ( dir == DIR_DOWN ) {
		return !( tile_walkable(x_left + y_bot * FRAME_WIDTH, f) && tile_walkable(x_right + y_bot * FRAME_WIDTH, f) );
	} else if ( dir == DIR_LEFT ) {
		return !( tile_walkable(x_left + y_top * FRAME_WIDTH, f) && tile_walkable(x_left + y_bot * FRAME_WIDTH, f) );
	} else if ( dir == DIR_RIGHT ) {
		return !( tile_walkable(x_right + y_top * FRAME_WIDTH, f) && tile_walkable(x_right + y_bot * FRAME_WIDTH, f) );
	}

	return false;*/
}

void set_fire() {
	static unsigned long addr;
	unsigned int pos = FRAME_BASE_ADDRESS + 5*SCR_WIDTH + 4;

	if(fire1 == FIRE_0) {
		fire1 = FIRE_1;
		fire2 = FIRE_0;
	} else {
		fire1 = FIRE_0;
		fire2 = FIRE_1;
	}

	addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * pos;
	Xil_Out32(addr,	fire1);
	addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (pos + 7);
	Xil_Out32(addr,	fire2);
}
bool collision(characters * mario, characters flame1, characters flame2, characters flame3, characters flame4){


	if(mario->x+2 >= flame1.x+4 && mario->x+2 <= flame1.x+12 && mario->y+16 >= flame1.y + 6 && mario->y+16 <= flame1.y + 16){
		return true;
	}else if(mario->x+14 >= flame1.x+4 && mario->x+14 <= flame1.x+12 && mario->y+16 >= flame1.y + 6 && mario->y+16 <= flame1.y + 16){
		return true;
	}

	if(mario->x+2 >= flame2.x+4 && mario->x+2 <= flame2.x+12 && mario->y+16 >= flame2.y + 6 && mario->y+16 <= flame2.y + 16){
			return true;
	}else if(mario->x+14 >= flame2.x+4 && mario->x+14 <= flame2.x+12 && mario->y+16 >= flame2.y + 6 && mario->y+16 <= flame2.y + 16){
		return true;
	}

	if(mario->x+2 >= flame3.x+4 && mario->x+2 <= flame3.x+12 && mario->y+16 >= flame3.y + 6 && mario->y+16 <= flame3.y + 16){
			return true;
	}else if(mario->x+14 >= flame3.x+4 && mario->x+14 <= flame3.x+12 && mario->y+16 >= flame3.y + 6 && mario->y+16 <= flame3.y + 16){
		return true;
	}

	if(mario->x+2 >= flame4.x+4 && mario->x+2 <= flame4.x+12 && mario->y+16 >= flame4.y + 6  && mario->y+16 <= flame4.y + 16){
			return true;
	}else if(mario->x+14 >= flame4.x+4 && mario->x+14 <= flame4.x+12 && mario->y+16 >= flame4.y + 6 && mario->y+16 <= flame4.y + 16){
		return true;
	}

	return false;


}
void battle_city() {
	unsigned int buttons;

    /*      initialization      */
	reset_memory();
	overw_x = INITIAL_FRAME_X;
	overw_y = INITIAL_FRAME_Y;
    load_frame( DIR_STILL );
    HEALTH = MAX_HEALTH;
    //set_header();

	mario.x = INITIAL_MARIO_POSITION_X;
	mario.y = INITIAL_MARIO_POSITION_Y;
	mario.sprite = 1791;


	chhar_spawn(&mario, 0);
	chhar_spawn(&flame1, 0);

	while (1) {
		int rnd =  random_number() % 100;
		int rnd1 = (random_number() % 1000) / 10;
		int rnd2 = (random_number() % 10000) / 100;
		int rnd3 = (random_number() % 100000) / 1000;
		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

		direction_t d = DIR_STILL;
		if ( BTN_LEFT(buttons) ) {
			d = DIR_LEFT;
		} else if ( BTN_RIGHT(buttons) ) {
			d = DIR_RIGHT;
		} else if ( BTN_UP(buttons) ) {
			d = DIR_UP;
		} else if ( BTN_DOWN(buttons) ) {
			d = DIR_DOWN;
		} else if ( BTN_SHOOT(buttons) ) {
			d = DIR_ATTACK;
		}

		/*if(enemy_exists == 1 && !inCave) {
			if(flame.active)
				enemy_move(&flame, rnd);
			if (octorok2.active)
				enemy_move(&octorok2, rnd1);
			if (octorok3.active)
				enemy_move(&octorok3, rnd2);
			if (octorok4.active)
				enemy_move(&octorok4, rnd3);
			if (ghost.active)
				ghost_move(&ghost, rnd);
		}*/

		link_move(&mario, &sword, d);
		enemy_move(&flame1, 1);
		enemy_move(&flame2, 2);
		enemy_move(&flame3, 3);
		enemy_move(&flame4, 4);

		if(collision(&mario, flame1, flame2, flame3, flame4)){
			mario.x = INITIAL_MARIO_POSITION_X;
			mario.y = INITIAL_MARIO_POSITION_Y;
			mario.sprite = 1791;
			mario.dir = DIR_LEFT;
			pos = 1;
			on_ladder = false;
			ladder_count = 0;
		}

	}
}
