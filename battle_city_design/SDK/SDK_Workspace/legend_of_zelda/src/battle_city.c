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
#define INITIAL_MARIO_SPRITE 			1791

/*      LINK SPRITES START ADDRESS - to move to next add 64    */
#define LINK_SPRITES_OFFSET             5648		//	old: 5172	,	new: 5648
#define SWORD_SPRITE                    1727			//6068		//	old: 7192	,	new: 7124
#define LINK_STEP						8

/*      ENEMIE SPRITES START ADDRESS - to move to next add 64    */
#define FLAME_SPRITE_OFFSET_LEFT 	   831
#define FLAME_SPRITE_OFFSET_RIGHT       895


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

#define MONKEY1_TOP_LEFT_L             0
#define MONKEY1_TOP_LEFT_H                 1
#define MONKEY1_BOT_LEFT_L					16
#define MONKEY1_BOT_LEFT_H	                17
#define MONKEY1_TOP_CENTRE_L             18
#define MONKEY1_TOP_CENTRE_H                 19
#define MONKEY1_BOT_CENTRE_L					20
#define MONKEY1_BOT_CENTRE_H	                21
#define MONKEY1_TOP_RIGHT_L             22
#define MONKEY1_TOP_RIGHT_H                 23
#define MONKEY1_BOT_RIGHT_L					24
#define MONKEY1_BOT_RIGHT_H	                25



int ladders_up[13][2] = {{176, 328},{192, 288},{208, 248},{224, 208},{256, 208},{280, 328},{312, 248},{360, 288},{376, 208},{408, 208},{424, 248},{432, 288},{458, 328}};
int ladders_down[13][2] = {{176, 288},{192, 248},{208, 208},{224, 168},{256, 168},{280, 288},{312, 208},{360, 248},{376, 168},{408, 168},{424, 208},{432, 248},{458, 288}};
int end_of_block[24][2] = {{159,328},{175,288},{255,288},{399,288},{191,248},{255,248},{399,248},{207,208},{255,208},{399,208},{223,168},{256,128},{470,328},{232,288},{376,288},{458,288},{232,248},{376,248},{440,248},{232,208},{376,208},{424,208},{408,168},{376,128}};
#define EDGE_NUM 16
int edges[16][2] = {{166,288},{182,248},{198,208},{214, 168},{246,128},{244, 288},{244, 248},{244, 208},{386, 288},{386, 248},{386, 208},{466, 288},{448, 248},{432, 208},{416, 168},{376, 128}};
bool on_ladder = false;

//ladder_count is used to keep track of where mario is on a ladder
int ladder_count = 0;

int enemy_obstackles1[5][2] = {{175, 288},{231,288},{192, 288}, {192, 248}, {231,248}};
int enemy_obstackles2[2][2] = {{256, 248},{375, 248}};
int enemy_obstackles3[2][2] = {{256, 288},{374, 288}};
int enemy_obstackles4[6][2] = {{400, 288},{454, 288},{432, 288}, {400, 248},{440,248}, {432, 248}};

//edges of the tramboline
int tramboline[2] = {320, 176};

//addresses of monkey sprites in ram.vhd
unsigned short MONKEY_RIGHT_LEG_SPRITES[6] = {1151, 1215, 1279, 959, 1023, 1087};
unsigned short MONKEY_LEFT_LEG_SPRITES[6]  = {1343, 1407, 1471, 1535, 1599, 1663};

int mario_lives = 3;


int pos = 1;
int counter = 0;
 //last state link was in before current iteration (if he is walking it keeps walking)
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


monkey main_enemy = {
		{296, 309, 322, 296, 309, 322}, //x position for every monkey sprite
		{152, 152, 152, 168, 168, 168}, //y position for every monkey sprite
		DIR_RIGHT,
		{1151, 1215, 1279, 959, 1023, 1087}, // address of every monkey sprite in ram.vhd
		true,
		{MONKEY1_TOP_LEFT_L, MONKEY1_TOP_CENTRE_L, MONKEY1_TOP_RIGHT_L, MONKEY1_BOT_LEFT_L, MONKEY1_BOT_CENTRE_L, MONKEY1_BOT_RIGHT_L},
		{MONKEY1_TOP_LEFT_H, MONKEY1_TOP_CENTRE_H, MONKEY1_TOP_RIGHT_H, MONKEY1_BOT_LEFT_H, MONKEY1_BOT_CENTRE_H, MONKEY1_BOT_RIGHT_H}
};
characters mario = {
		INITIAL_MARIO_POSITION_X,		// x
		INITIAL_MARIO_POSITION_Y,		// y
		DIR_DOWN, 	             		// dir
		0x06BF,							// type - sprite address in ram.vhdl
		true,                			// active
		LINK_REG_L,            			// reg_l
		LINK_REG_H             			// reg_h
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

//draws the whole map, map is in map.h under overworld
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

}


//drawing mario lives on the map
void set_header() {
	long int addr;
	if(mario_lives==3){
		overworld[ 0 * FRAME_WIDTH + 0] = 1791;
		overworld[ 0 * FRAME_WIDTH + 1] = 1791;
		overworld[ 0 * FRAME_WIDTH + 2] = 1791;
	}else if(mario_lives==2){
		overworld[ 0 * FRAME_WIDTH + 0] = 1791;
		overworld[ 0 * FRAME_WIDTH + 1] = 1791;
		overworld[ 0 * FRAME_WIDTH + 2] = SPRITES[6];
	}else if(mario_lives==1){
		overworld[ 0 * FRAME_WIDTH + 0] = 1791;
		overworld[ 0 * FRAME_WIDTH + 1] = SPRITES[6];
		overworld[ 0 * FRAME_WIDTH + 2] = SPRITES[6];
	}

	addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (0+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + 0 + SIDE_PADDING);
	Xil_Out32( addr, overworld[0 * FRAME_WIDTH + 0]);
	addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (0+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + 1 + SIDE_PADDING);
	Xil_Out32( addr, overworld[0 * FRAME_WIDTH + 1]);
	addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (0+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + 2 + SIDE_PADDING);
	Xil_Out32( addr, overworld[0 * FRAME_WIDTH + 2]);

}

void enemy_move(characters* chhar, int divider){
	int x,y;

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
			}

	}

	chhar->x = x;
	chhar->y = y;
	chhar_spawn(chhar, 0);

}

void chhar_delete(){

	delete_sword(&flame1);
	delete_sword(&flame2);
	delete_sword(&flame3);
	delete_sword(&flame4);

}
void monkey_spawn(monkey chhar, int rotation){
	int i;
	for(i=0; i<6; i++){
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar.reg_l[i] ),
				(unsigned int) 0x8F000000 | (unsigned int) chhar.sprites[i] );
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar.reg_h[i]),
				(chhar.y[i] << 16) | chhar.x[i] );
	}
}

void chhar_spawn( characters * chhar, int rotation ) {

	Xil_Out32(
					XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
					(unsigned int) 0x8F000000 | (unsigned int) chhar->sprite );
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
		Xil_Out32( addr, SPRITES[6] );             // SPRITES[6] is a black sprite
	}

	for ( i = 0; i <= 20; i += 2 ) {
		Xil_Out32( XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ), (unsigned int) 0x0F000000);
	}
}
//mario is under a ladder
bool acceptable_position_up(int x, int y){

	int i;
	for(i=0; i<LADDERS_NUM; i++){
		if(x>=ladders_up[i][0]-4 && x<=ladders_up[i][0] + 4 && y==ladders_up[i][1]){
			return true;
		}
	}
	return false;
}

//mario is above a ladder
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

			//mario is between the left edge of the screen and a platform
			if(x>=150 && x<=edges[i][0] && y==edges[i][1] && i<5){
				return true;
			//mario is between the right edge of the screen and a platform
			}else if(x>=edges[i][0] && x<=470 && y==edges[i][1] && i>=11 ){
				return true;
			//mario is between two platforms
			}else if(x>=edges[i][0]-2 && x<=edges[i][0]+2 && y==edges[i][1]){
				return true;
			}
		}
		return false;
}
bool link_move(characters * link, direction_t dir) {
	unsigned int x;
	unsigned int y;


	int i,j;

	unsigned int buttons;

	x = mario.x;
	y = mario.y;

	//if mario is on a ladder he can move 40 pixels up or down
	if(on_ladder){
		if ( dir == DIR_UP ) {
				y--;
				ladder_count++;

			    if(ladder_count == 40 || ladder_count == 0){
			    	on_ladder = false;
			    	mario.sprite =  INITIAL_MARIO_SPRITE;
			    	ladder_count = 0;
			    	pos = 1;
			    }else{
			    	if(mario.sprite ==  2239)
						mario.sprite =  2303;
					else if(mario.sprite ==  2303)
						mario.sprite =  2239;

			    }


		} else if ( dir == DIR_DOWN) {
				y++;
				ladder_count--;
				if(ladder_count == 0 || ladder_count == -40){
					on_ladder = false;
					mario.sprite =  INITIAL_MARIO_SPRITE;
					ladder_count = 0;
					pos = 1;
				}else{
					if(mario.sprite ==  2239)
						mario.sprite =  2303;
					else if(mario.sprite ==  2303)
						mario.sprite =  2239;
				}

		}
	}else if ( dir == DIR_LEFT ) {
			//the left edge of the screen
			if(x==160){
				x=160;
			}else{
				x--;
			}
			if(pos == 1){
				pos = 0;
				mario.sprite =  2047; //left mario sprite
			}
			if(mario.sprite ==  2047)
				mario.sprite =  2111; //mario right sprite
			else if(mario.sprite ==  2111)
				mario.sprite =  2047;

			//checks if mario reaches the end of a platform and he falls until he falls on a platform
			while(check_edges(x,y)){
				for(i =0; i<40; i++){
					y++;
					for(j = 0; j<100000; j++);
					buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

					enemy_move(&flame1, 1);
					enemy_move(&flame2, 2);
					enemy_move(&flame3, 3);
					enemy_move(&flame4, 4);
					monkey_move(&main_enemy);



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
									mario.sprite =  2047;
								}
								if(mario.sprite ==  2047)
									mario.sprite =  2111;
								else if(mario.sprite ==  2111)
									mario.sprite =  2047;

					} else if ( d == DIR_RIGHT ) {
								x++;

								if(pos == 0){
									pos = 1;
									mario.sprite =  INITIAL_MARIO_SPRITE;
								}
								if(mario.sprite ==  1727)
									mario.sprite =  INITIAL_MARIO_SPRITE;
								else if(mario.sprite ==  INITIAL_MARIO_SPRITE)
									mario.sprite =  1727;
								counter++;
					}
					mario.x = x;
					mario.y = y;

					chhar_spawn( link, 0 );
				}
			}
	} else if ( dir == DIR_RIGHT) {
			//right edge of the screen in pixels
			if(x==470){
				x=470;
			}else{
				x++;
			}

			if(pos == 0){
				pos = 1;
				mario.sprite =  INITIAL_MARIO_SPRITE;
			}
			if(mario.sprite ==  1727)//mario left sprite
				mario.sprite =  INITIAL_MARIO_SPRITE; //mario right sprite
			else if(mario.sprite ==  INITIAL_MARIO_SPRITE)
				mario.sprite =  1727;

			//checks if mario reaches the end of a platfrom, then he falls
			while(check_edges(x,y)){
					for(i =0; i<40; i++){
						y++;
						for(j = 0; j<100000; j++);
						buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

						enemy_move(&flame1, 1);
						enemy_move(&flame2, 2);
						enemy_move(&flame3, 3);
						enemy_move(&flame4, 4);
						monkey_move(&main_enemy);



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
										mario.sprite =  2047;
									}
									if(mario.sprite ==  2047)
										mario.sprite =  2111;
									else if(mario.sprite ==  2111)
										mario.sprite =  2047;
									counter++;
						} else if ( d == DIR_RIGHT ) {
									if(x==470){
										x=470;
									}else{
										x++;
									}
									if(pos == 0){
										pos = 1;
										mario.sprite =  INITIAL_MARIO_SPRITE;
									}
									if(mario.sprite ==  1727)
										mario.sprite =  INITIAL_MARIO_SPRITE;
									else if(mario.sprite ==  INITIAL_MARIO_SPRITE)
										mario.sprite =  1727;
									counter++;
						}
						mario.x = x;
						mario.y = y;

						chhar_spawn( link, 0 );
					}
				}
	//checks if mario reaches a ladder
	} else if ( dir == DIR_UP && !acceptable_position_up(x,y)) {
			jump();
			x = mario.x;
			y = mario.y;

	} else if (  dir == DIR_UP && acceptable_position_up(x,y) ) {
			y--;
			on_ladder = true;
			ladder_count++;
			mario.sprite =  2239;
	}else if(dir == DIR_DOWN && acceptable_position_down(x,y) ){
		y++;
		ladder_count--;
		on_ladder = true;
		mario.sprite =  2239;
	}
	mario.x = x;
	mario.y = y;

	chhar_spawn( link, 0 );

	for(i =0; i<100000; i++);

	return true;
}

void tramboline_simulation(){
	static int move = 0;
	int k,y,x,j;
	x = mario.x;
	y = mario.y;
	for(k=0; k<48; k++){
		y--;
		mario.x = x;
		mario.y = y;
		chhar_spawn( &mario, 0 );

		for(j = 0; j<25000; j++);
		move++;

		//mario is jumping faster than the enemies are moving
		if(move == 3){
			move = 0;
			enemy_move(&flame1, 1);
			enemy_move(&flame2, 2);
			enemy_move(&flame3, 3);
			enemy_move(&flame4, 4);
			monkey_move(&main_enemy);
		}
		if(collision(&mario, flame1, flame2, flame3, flame4)){
			mario.x = INITIAL_MARIO_POSITION_X;
			mario.y = INITIAL_MARIO_POSITION_Y;
			mario.dir = DIR_LEFT;
			mario.sprite = INITIAL_MARIO_SPRITE;
			pos = 1;
		}
	}

	//mario falls slower than he goes up, to simulate a tramboline
	for(k=0; k<16; k++){
		y++;
		mario.x = x;
		mario.y = y;
		chhar_spawn( &mario, 0 );
		for(j = 0; j<100000; j++);
		enemy_move(&flame1, 1);
		enemy_move(&flame2, 2);
		enemy_move(&flame3, 3);
		enemy_move(&flame4, 4);
		monkey_move(&main_enemy);
	}
}
void jump(){
	unsigned int buttons;
	int i,j,x,y;
	x = mario.x;
	y = mario.y;

	//Mario is going up until his head hits the platform above him
	for(i =0; i<16; i++){
		y--;

		for(j = 0; j<100000; j++);
		enemy_move(&flame1, 1);
		enemy_move(&flame2, 2);
		enemy_move(&flame3, 3);
		enemy_move(&flame4, 4);
		monkey_move(&main_enemy);




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

		//when he is going up he can move left and right
		if ( d == DIR_LEFT ) {
					x--;

					if(pos == 1){
						pos = 0;
						mario.sprite =  2047;
					}
					if(mario.sprite ==  2047)
						mario.sprite =  2111;
					else if(mario.sprite ==  2111)
						mario.sprite =  2047;
					counter++;
		} else if ( d == DIR_RIGHT ) {
					x++;
					if(pos == 0){
						pos = 1;
						mario.sprite =  INITIAL_MARIO_SPRITE;
					}
					if(mario.sprite ==  1727)
						mario.sprite =  INITIAL_MARIO_SPRITE;
					else if(mario.sprite ==  INITIAL_MARIO_SPRITE)
						mario.sprite =  1727;
					counter++;
		}

		for(j = 0; j<100000; j++);
		enemy_move(&flame1, 1);
		enemy_move(&flame2, 2);
		enemy_move(&flame3, 3);
		enemy_move(&flame4, 4);
		monkey_move(&main_enemy);
		mario.x = x;
		mario.y = y;

		if(collision(&mario, flame1, flame2, flame3, flame4)){
			mario.x = INITIAL_MARIO_POSITION_X;
			mario.y = INITIAL_MARIO_POSITION_Y;
			mario.dir = DIR_LEFT;
			mario.sprite = INITIAL_MARIO_SPRITE;
			pos = 1;
		}



		mario.x = x;
		mario.y = y;


		chhar_spawn( &mario, 0 );
	}

	//mario is falling
	for(i =0; i<16; i++){
		y++;

		mario.y = y;
		if(tramboline_check(x,y)){
			tramboline_simulation();
			return;
		}
		for(j = 0; j<100000; j++);
		enemy_move(&flame1, 1);
		enemy_move(&flame2, 2);
		enemy_move(&flame3, 3);
		enemy_move(&flame4, 4);
		monkey_move(&main_enemy);
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
		//while falling you can still move left and right
		if ( d == DIR_LEFT ) {
					x--;

					if(pos == 1){
						pos = 0;
						mario.sprite =  2047;
					}
					if(mario.sprite ==  2047)
						mario.sprite =  2111;
					else if(mario.sprite ==  2111)
						mario.sprite =  2047;
					counter++;
		} else if ( d == DIR_RIGHT ) {
					x++;

					if(pos == 0){
						pos = 1;
						mario.sprite =  INITIAL_MARIO_SPRITE;
					}
					if(mario.sprite ==  1727)
						mario.sprite =  INITIAL_MARIO_SPRITE;
					else if(mario.sprite ==  INITIAL_MARIO_SPRITE)
						mario.sprite =  1727;
					counter++;
		}

		for(j = 0; j<100000; j++);
		enemy_move(&flame1, 1);
		enemy_move(&flame2, 2);
		enemy_move(&flame3, 3);
		enemy_move(&flame4, 4);
		monkey_move(&main_enemy);
		mario.x = x;
		mario.y = y;

		if(collision(&mario, flame1, flame2, flame3, flame4)){
							mario.x = INITIAL_MARIO_POSITION_X;
							mario.y = INITIAL_MARIO_POSITION_Y;
							mario.dir = DIR_LEFT;
							mario.sprite = INITIAL_MARIO_SPRITE;
							pos = 1;
		}


		mario.x = x;
		mario.y = y;



		chhar_spawn( &mario, 0 );
	}

	//if mario lands on a hole, he falls until he reaches a platform
	while(check_edges(mario.x,mario.y)){
		for(i =0; i<40; i++){
			y++;
			for(j = 0; j<100000; j++);
			buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

			enemy_move(&flame1, 1);
			enemy_move(&flame2, 2);
			enemy_move(&flame3, 3);
			enemy_move(&flame4, 4);
			monkey_move(&main_enemy);


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
							mario.sprite =  2047;
						}
						if(mario.sprite ==  2047)
							mario.sprite =  2111;
						else if(mario.sprite ==  2111)
							mario.sprite =  2047;
						counter++;
			} else if ( d == DIR_RIGHT ) {
						if(x==470){
							x=470;
						}else{
							x++;
						}
						if(pos == 0){
							pos = 1;
							mario.sprite =  INITIAL_MARIO_SPRITE;
						}
						if(mario.sprite ==  1727)
							mario.sprite =  INITIAL_MARIO_SPRITE;
						else if(mario.sprite ==  INITIAL_MARIO_SPRITE)
							mario.sprite =  1727;
						counter++;
			}
			mario.x = x;
			mario.y = y;

			chhar_spawn(&mario, 0 );
		}
	}
}

int obstackles_detection(int x, int y, unsigned short* f, int dir, bool isLink) {
	int i;


	for(i=0; i<LADDERS_NUM; i++){
		if(x==ladders_up[i][0] && y == ladders_up[i][1]){
			return 0; //the obstackle is a lower part of a ladder
		}else if(x == ladders_down[i][0] && y == ladders_down[i][1]){
			return 1; //the obstackle is an upper part of a ladder
		}
	}

	for(i=0; i<24; i++){

		if(x==end_of_block[i][0] && y==end_of_block[i][1] && i<12){
			return 2; //left edge
		}
		else if(x==end_of_block[i][0] && y==end_of_block[i][1] && i>=12){
			return 3; //right edge
		}
	}

	return 4;

}
//checks if Mario hits any of the enemies
bool collision(characters * mario, characters flame1, characters flame2, characters flame3, characters flame4){


	if(mario->x+2 >= flame1.x+4 && mario->x+2 <= flame1.x+12 && mario->y+16 >= flame1.y + 6 && mario->y+16 <= flame1.y + 16){
		mario_lives--;
		return true;
	}else if(mario->x+14 >= flame1.x+4 && mario->x+14 <= flame1.x+12 && mario->y+16 >= flame1.y + 6 && mario->y+16 <= flame1.y + 16){
		mario_lives--;
		return true;
	}

	if(mario->x+2 >= flame2.x+4 && mario->x+2 <= flame2.x+12 && mario->y+16 >= flame2.y + 6 && mario->y+16 <= flame2.y + 16){
			mario_lives--;
			return true;
	}else if(mario->x+14 >= flame2.x+4 && mario->x+14 <= flame2.x+12 && mario->y+16 >= flame2.y + 6 && mario->y+16 <= flame2.y + 16){
		mario_lives--;
		return true;
	}

	if(mario->x+2 >= flame3.x+4 && mario->x+2 <= flame3.x+12 && mario->y+16 >= flame3.y + 6 && mario->y+16 <= flame3.y + 16){
		mario_lives--;
		return true;
	}else if(mario->x+14 >= flame3.x+4 && mario->x+14 <= flame3.x+12 && mario->y+16 >= flame3.y + 6 && mario->y+16 <= flame3.y + 16){
		mario_lives--;
		return true;
	}

	if(mario->x+2 >= flame4.x+4 && mario->x+2 <= flame4.x+12 && mario->y+16 >= flame4.y + 6  && mario->y+16 <= flame4.y + 16){
		mario_lives--;
			return true;
	}else if(mario->x+14 >= flame4.x+4 && mario->x+14 <= flame4.x+12 && mario->y+16 >= flame4.y + 6 && mario->y+16 <= flame4.y + 16){
		mario_lives--;
		return true;
	}

	if(mario->x+2 >= main_enemy.x[0]+4 && mario->x+2 <= main_enemy.x[2]+12 && mario->y+16 >= main_enemy.y[0]  && mario->y+16 <= main_enemy.y[3] + 16){
		mario_lives--;
		return true;
	}else if(mario->x+14 >= main_enemy.x[0]+4 && mario->x+14 <= main_enemy.x[2]+12 && mario->y+16 >= main_enemy.y[0]  && mario->y+16 <= main_enemy.y[3] + 16){
		mario_lives--;
		return true;
	}

	return false;


}

void monkey_move(monkey * main_enemy){
	int i;
	static int move = 0;

	if(main_enemy->dir == DIR_RIGHT && main_enemy->x[5] < 400){//the monkey goes right until his right leg reaches the right edge
		for(i=0; i<6; i++){
			++main_enemy->x[i];
		}
	}else if(main_enemy->dir == DIR_LEFT && main_enemy->x[0] > 224){//and goes left until his left leg reaches the left edge
		for(i=0; i<6; i++){
			--main_enemy->x[i];
		}
	}else if(main_enemy->dir == DIR_RIGHT && main_enemy->x[5] == 400){//when he reaches an edge, he changes direction
		main_enemy->dir = DIR_LEFT;
	}else{
		main_enemy->dir = DIR_RIGHT;
	}



	//the monkey changes sprites to simulate moving
	if(main_enemy->sprites[0] == MONKEY_RIGHT_LEG_SPRITES[0] && move == 14){
		for(i=0; i<6; i++){
			main_enemy->sprites[i] = MONKEY_LEFT_LEG_SPRITES[i];
			move = 0;
		}
	}else if(move == 14){
		for(i=0; i<6; i++){
			main_enemy->sprites[i] = MONKEY_RIGHT_LEG_SPRITES[i];
			move = 0;
		}
	}
	move++;
	monkey_spawn(*main_enemy, 0);



}

bool tramboline_check(int x, int y){
	if(x+14 >= tramboline[0] && x+14 <= tramboline[0]+8 && y+16 == tramboline[1]){
		return true;
	}
	if(x+2 >= tramboline[0] && x+2 <= tramboline[0]+8 && y+16 == tramboline[1]){
		return true;
	}
	return false;
}

void check_victory(){
	unsigned int buttons;
	long int addr;
	if(mario.y == 128 && mario.x <= 304){

		//draws a heart between Mario and the princess
		overworld[ 0 * FRAME_WIDTH + 8] = 2559;


		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (0+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + 8 + SIDE_PADDING);
		Xil_Out32( addr, overworld[0 * FRAME_WIDTH + 8]);

		while(1){
			buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );
			if ( BTN_SHOOT(buttons) ) {
				break;
			}

		}
		mario.x = INITIAL_MARIO_POSITION_X;
		mario.y = INITIAL_MARIO_POSITION_Y;
		mario.sprite = INITIAL_MARIO_SPRITE;
		mario.dir = DIR_RIGHT;
		pos = 1;
		on_ladder = false;
		ladder_count = 0;

		//deletes the heart between Mario and the princess
		overworld[ 0 * FRAME_WIDTH + 8] = SPRITES[6];


		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + (0+VERTICAL_PADDING)* ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + 8 + SIDE_PADDING);
		Xil_Out32( addr, overworld[0 * FRAME_WIDTH + 8]);
		mario_lives = 3;

	}
}
void check_remaining_lives(){
	unsigned int buttons;
	if(mario_lives == 0){
		while(1){
			buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );
			if ( BTN_SHOOT(buttons) ) {
				break;
			}
		}
		mario.x = INITIAL_MARIO_POSITION_X;
		mario.y = INITIAL_MARIO_POSITION_Y;
		mario.sprite = INITIAL_MARIO_SPRITE;
		mario.dir = DIR_LEFT;
		pos = 1;
		on_ladder = false;
		ladder_count = 0;
		mario_lives = 3;

	}
}

void check_collision(){
	if(collision(&mario, flame1, flame2, flame3, flame4)){
		mario.x = INITIAL_MARIO_POSITION_X;
		mario.y = INITIAL_MARIO_POSITION_Y;
		mario.sprite = INITIAL_MARIO_SPRITE;
		mario.dir = DIR_LEFT;
		pos = 1;
		on_ladder = false;
		ladder_count = 0;
	}
}
void battle_city() {
	unsigned int buttons;
    /*      initialization      */

	reset_memory();
	set_header();
    load_frame( DIR_STILL );

	mario.x = INITIAL_MARIO_POSITION_X;
	mario.y = INITIAL_MARIO_POSITION_Y;
	mario.sprite = INITIAL_MARIO_SPRITE;

	chhar_spawn(&mario, 0);
	chhar_spawn(&flame1, 0);



	while (1) {
		set_header();
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


		enemy_move(&flame1, 1);
		enemy_move(&flame2, 2);
		enemy_move(&flame3, 3);
		enemy_move(&flame4, 4);


		monkey_move(&main_enemy);
		link_move(&mario, d);

		check_collision();
		check_remaining_lives();
		check_victory();




	}
}
