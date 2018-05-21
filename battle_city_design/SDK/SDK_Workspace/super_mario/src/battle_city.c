#include "battle_city.h"
#include "map.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>

typedef int bool;
#define true 1
#define false 0

/*          COLOR PALETTE - base addresses in ram.vhd         */
#define FRAME_COLORS_OFFSET         0
#define LINK_COLORS_OFFSET          8

/*		SCREEN PARAMETERS		 - in this case, "screen" stands for one full-screen picture	 */
#define SCREEN_BASE_ADDRESS			6900
#define SCR_HEIGHT					30
#define SCR_WIDTH					40

/*		FRAME HEADER		*/
#define HEADER_BASE_ADDRESS			7192

/*      FRAME       */
#define FRAME_BASE_ADDRESS			7392 // FRAME_OFFSET in battle_city.vhd
#define SIDE_PADDING				12
#define VERTICAL_PADDING			7
#define INITIAL_FRAME_X				7
#define INITIAL_FRAME_Y				7
#define INITIAL_LINK_POSITION_X		200 + 64
#define INITIAL_LINK_POSITION_Y		270

/*      LINK SPRITES START ADDRESS - to move to next add 64    */
#define LINK_SPRITES_OFFSET             3519
#define SWORD_SPRITE                    4415 
#define LINK_STEP						10

/*		find out what the below lines stand for		 */
#define MAP_X							0
#define MAP_X2							640
#define MAP_Y							4
#define MAP_W							64
#define MAP_H							56
/*		find out what the above lines stand for		 */

#define REGS_BASE_ADDRESS               ( SCREEN_BASE_ADDRESS + SCR_WIDTH * SCR_HEIGHT )

#define BTN_DOWN( b )                   ( !( b & 0x01 ) )
#define BTN_UP( b )                     ( !( b & 0x10 ) )
#define BTN_LEFT( b )                   ( !( b & 0x02 ) )
#define BTN_RIGHT( b )                  ( !( b & 0x08 ) )
#define BTN_SHOOT( b )                  ( !( b & 0x04 ) )


/*			these are the high and low registers that store moving sprites - two registers for each sprite		 */
#define TANK1_REG_L                     8
#define TANK1_REG_H                     9
#define TANK_AI_REG_L                   4
#define TANK_AI_REG_H                   5
#define TANK_AI_REG_L2                  6
#define TANK_AI_REG_H2                  7
#define TANK_AI_REG_L3                  2
#define TANK_AI_REG_H3                  3
#define TANK_AI_REG_L4                  10
#define TANK_AI_REG_H4                  11
#define TANK_AI_REG_L5                  12
#define TANK_AI_REG_H5                  13
#define TANK_AI_REG_L6                  14
#define TANK_AI_REG_H6                  15
#define TANK_AI_REG_L7                  16
#define TANK_AI_REG_H7                  17
#define BASE_REG_L						0
#define BASE_REG_H	                    1

int lives = 0;
int score = 0;
int mapPart = 1;
int udario_glavom_skok = 0;
int map_move = 0;
int udario_u_blok = 0;
int counter = 0;
int last = 0; //last state mario was in before current iteration (if he is walking it keeps walking)
/*For testing purposes -- all is +1
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
		15 - arrow
		16 - boomerang 1
		17 - boomerang 2
		18 - boomerang 3
		19 - magic 1
		20 - magic 2
		21 - up flipped
		22 - left walk
		23 - left stand
		24 - left walk shield
		25 - left stand shield
		26 - left attack
*/
	
/*			16x16 IMAGES		 */
unsigned short SPRITES[53] = {0x00FF, 0x013F, 0x017F, 0x01BF, 0x01FF, 0x023F, 0x027F, 0x02BF, 0x02FF, 0x033F, 0x037F, 0x03BF, 0x03FF, 0x043F, 0x047F, 0x04BF, 0x04FF, 0x053F, 0x057F, 0x05BF, 0x05FF, 0x063F, 0x067F, 0x06BF, 0x06FF, 0x073F, 0x077F, 0x07BF, 0x07FF, 0x083F, 0x087F, 0x08BF, 0x08FF, 0x093F, 0x097F, 0x09BF, 0x09FF, 0x0A3F, 0x0A7F, 0x0ABF, 0x0AFF, 0x0B3F, 0x0B7F, 0x0BBF, 0x0BFF, 0x0C3F, 0x0C7F, 0x0CBF, 0x0CFF, 0x0D3F, 0x0D7F, 0x0DBF, 0x0DFF, 0x0E3F };

/*		 ACTIVE FRAME		*/
unsigned short* frame;

typedef enum {
	b_false, b_true
} bool_t;

typedef enum {
	DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL, DIR_ATTACK
} direction_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	direction_t dir;
	unsigned int type;

	bool_t destroyed;

	unsigned int reg_l;
	unsigned int reg_h;
} characters;

characters mario = { 
		INITIAL_LINK_POSITION_X,		// x
		INITIAL_LINK_POSITION_Y,		// y
		DIR_DOWN, 	             		// dir
		0x0DFF,							// type - sprite address in ram.vhdl
		b_false,                		// destroyed
		TANK1_REG_L,            		// reg_l
		TANK1_REG_H             		// reg_h
		};

characters sword = {
		INITIAL_LINK_POSITION_X,		// x
		INITIAL_LINK_POSITION_Y,		// y
		DIR_LEFT,              			// dir
		SWORD_SPRITE,  					// type

		b_false,                		// destroyed

		TANK_AI_REG_L,            		// reg_l
		TANK_AI_REG_H             		// reg_h
		};
/*
characters enemie2 = { 450,						// x
		431,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemi1,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L2,            		// reg_l
		TANK_AI_REG_H2             		// reg_h
		};

characters enemie3 = { 330,						// x
		272,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemi1,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L3,            		// reg_l
		TANK_AI_REG_H3             		// reg_h
		};

characters enemie4 = { 635,						// x
		431,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemi1,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L4,            		// reg_l
		TANK_AI_REG_H4             		// reg_h
		};
*/
int overw_x = INITIAL_FRAME_X;
int overw_y = INITIAL_FRAME_Y;
void load_frame(direction_t dir) {
    switch(dir) {
        case DIR_LEFT:
            overw_x = (--overw_x<0)? 0 : overw_x;
            break;
        case DIR_RIGHT:
            overw_x = (++overw_x>15)? 15 : overw_x;
            break;
        case DIR_UP:
            overw_y = (--overw_y<0)? 0 : overw_y;
            break;
        case DIR_DOWN:
            overw_y = (++overw_y>7)? 7 : overw_y;
            break;
    }
    frame = overworld[overw_y*16 + overw_x];

    /*      loading next frame into memory      */
    int x,y;
    long int addr;
    for (y = 0; y < FRAME_HEIGHT; y++) {
		for (x = 0; x < FRAME_WIDTH; x++) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (FRAME_BASE_ADDRESS + y * (SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING) + x);
			Xil_Out32(addr, frame[y*FRAME_WIDTH + x]);
		}
	}

    /*      setting the correct palette for the current frame     */
    long int addr_fill, addr_floor;
    addr_fill = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (FRAME_COLORS_OFFSET);
    addr_floor = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (FRAME_COLORS_OFFSET+1);
    addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (LINK_COLORS_OFFSET);

    if ((overw_y==2 & (overw_x<3 || overw_x==15)) || (overw_y == 3 & (overw_x<2 || overw_x==3)) || (overw_y==4 & overw_x<2) || (overw_y==5 & overw_x ==0) ) {
		/*    red/green -> white    */
		Xil_Out32(addr_fill, 0x00FCFCFC);
		/*    sand -> gray    */
		Xil_Out32(addr_floor, 0x747474);
		//Xil_Out32(addr, 0x747474);
    } else if ((overw_y==3 & (overw_x==12 || overw_x==13)) || (overw_y==4 & (overw_x>5 & overw_x<15)) || ((overw_y==4 || overw_y==5) & (overw_x>3 & overw_x<15)) || (overw_y==7 & (overw_x>3 & overw_x<9)) || (overw_y==6 &  overw_x > 3 & overw_x <15) ) {
		/*    red/white -> green    */
		Xil_Out32(addr_fill, 0x00A800);
		/*    gray -> sand    */
		Xil_Out32(addr_floor, 0xA8D8FC); 
		//Xil_Out32(addr, 0xA8D8FC);
    } else {
		/*    green/white -> red    */
		Xil_Out32(addr_fill, 0x0C4CC8);
		/*    gray -> sand    */
		Xil_Out32(addr_floor, 0xA8D8FC); 
		//Xil_Out32(addr, 0xA8D8FC);
    }
    /*      TODO: add logic for updating the overworld position in header   */
    /*  idea: 1x2 gray sprites, position is 2x2 pixels     */
}

unsigned int rand_lfsr113(void) {
	static unsigned int z1 = 12345, z2 = 12345;
	unsigned int b;

	b = ((z1 << 6) ^ z1) >> 13;
	z1 = ((z1 & 4294967294U) << 18) ^ b;
	b = ((z2 << 2) ^ z2) >> 27;
	z2 = ((z2 & 4294967288U) << 2) ^ b;

	return (z1 ^ z2);
}

static void chhar_spawn(characters * chhar, int rotation) {
	if (rotation == 1){																			 //rotate 90degrees clockwise
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int )0x8F100000 | (unsigned int )chhar->type);
	} else if (rotation == 2){ 																	//rotate 90degrees aniclockwise
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int )0x8F010000 | (unsigned int )chhar->type);
	} else if (rotation == 3){ 																	//rotate 180degrees
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int )0x8F020000 | (unsigned int )chhar->type);
	}else {																						//no rotation
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
				(unsigned int )0x8F000000 | (unsigned int )chhar->type);
	}
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x);
}

static void delete_sword(characters* chhar){
	int i  = 0;
	for (; i < 100000; i++){

	}
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
			(unsigned int )0x80000000 | (unsigned int )chhar->type);
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x);
}

static void map_update(characters * mario) {
	int x, y, i;
	long int addr;

	if (mario->x >= 330) {
		if (udario_u_blok <= 0) {
			map_move++;
		}
		if (mario->x == 2560) {
			map_move = 2520;
		}
	}

	for (y = 0; y < FRAME_HEIGHT; y++) {
		for (x = 0; x < FRAME_WIDTH; x++) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (FRAME_BASE_ADDRESS + y * (SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING) + x);
			Xil_Out32(addr, frame[y*FRAME_WIDTH + x]);
		}
	}
}
// currently, this function is cleaning the registers used for movind characters sprites; two registers are used for each sprite
static void map_reset() {
	unsigned int i;
	long int addr;

	for(i = 0; i<SCR_WIDTH*SCR_HEIGHT; i++) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * (SCREEN_BASE_ADDRESS + i);
		Xil_Out32(addr, SPRITES[10]); // SPRITES[10] is a black square
	}

	for (i = 0; i <= 20; i += 2) {
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ),
				(unsigned int )0x0F000000);
	}
}


static bool_t mario_move(characters * mario, characters* sword, direction_t dir) {
	unsigned int x;
	unsigned int y;
	int obstackle = 0;
	int sword_rotation = 0;
	int i;


	//frame and map edges
    if (mario->x > ((SIDE_PADDING + FRAME_WIDTH) * 16 - 16)){
    	mario->x = overw_x==15? mario->x-1 :SIDE_PADDING * 16;
    	load_frame(DIR_RIGHT);
    	return b_false;
	}
    if (mario->y > (SIDE_PADDING + FRAME_HEIGHT) * 16 - 16) {
    	mario->y = overw_y==7 ? mario->y-1 : 12 * 16;
    	load_frame(DIR_DOWN);
    	return b_false;
    }
    if (mario->y < SIDE_PADDING * 16) {
    	mario->y = overw_y==0 ? mario->y+1 : (SIDE_PADDING + FRAME_HEIGHT) * 16 - 16;
    	load_frame(DIR_UP);
		return b_false;
    }
    if (mario->x < SIDE_PADDING * 16) {
    	mario->x = overw_x==0 ? mario->x+1 :((SIDE_PADDING + FRAME_WIDTH) * 16 - 16);
    	load_frame(DIR_LEFT);

		return b_false;
	}

	x = mario->x;
	y = mario->y;

	/*For testing purposes -- all is +1
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
		15 - arrow
		16 - boomerang 1
		17 - boomerang 2
		18 - boomerang 3
		19 - magic 1
		20 - magic 2
		21 - up flipped
		22 - left walk
		23 - left stand
		24 - left walk shield
		25 - left stand shield
		26 - left attack
	*/

	//animacija kretanja
	if (dir == DIR_LEFT) {
		x--;
		if (counter%LINK_STEP == 0) {
			last = (last == 22)? 23 : 22;
		}
		mario->type =  LINK_SPRITES_OFFSET + 64*last;
		counter++;
	} else if (dir == DIR_RIGHT) {
		x++;
		if (counter%LINK_STEP == 0) {
			last = (last == 3)? 4 : 3;
		}
		//TODO:	set sprite
		mario->type =  LINK_SPRITES_OFFSET + 64*last;
		counter++;
	} else if (dir == DIR_UP) {
		y--;
		if (counter%LINK_STEP == 0) {
			last = (last == 2)? 21 : 2;
		}
			mario->type =  LINK_SPRITES_OFFSET+64*last;
			counter++;

	} else if (dir == DIR_DOWN) {
		y++;
		if (counter%LINK_STEP == 0) {
			last = (last == 0) ? 1 : 0;
		}
		mario->type = LINK_SPRITES_OFFSET + 64*last;
		counter++;
	} else if (dir == DIR_ATTACK){
		switch(last){
			case 0: //down
				sword->x = mario->x;
				sword->y = mario->y+16;
				mario->type =  LINK_SPRITES_OFFSET + 64*9;
				sword_rotation = 1;
				break;
			case 1: //down
				sword->x = mario->x;
				sword->y = mario->y+16;
				mario->type =  LINK_SPRITES_OFFSET + 64*9;
				sword_rotation = 1;
				break;
			case 2: //up
				sword->x = mario->x;
				sword->y = mario->y-16;
				mario->type =  LINK_SPRITES_OFFSET + 64*10;
				sword_rotation = 2;
				break;
			case 21: //up
				sword->x = mario->x;
				sword->y = mario->y-16;
				mario->type =  LINK_SPRITES_OFFSET + 64*10;
				sword_rotation = 2;
				break;
			case 3: //right
				sword->x = mario->x + 16;
				mario->type =  LINK_SPRITES_OFFSET + 64*11;
				sword_rotation = 0;
				sword->y = mario->y;
				break;
			case 4: //right
				sword->x = mario->x + 16;
				mario->type =  LINK_SPRITES_OFFSET + 64*11;
				sword_rotation = 0;
				sword->y = mario->y;
				break;
			case 22: //left
				sword->x = mario->x - 16;
				mario->type =  LINK_SPRITES_OFFSET + 64*26;
				sword_rotation = 3;
				sword->y = mario->y;
				break;
			case 23: //left
				sword->x = mario->x - 16;
				mario->type =  LINK_SPRITES_OFFSET + 64*26;
				sword_rotation = 3;
				sword->y = mario->y;
				break;
		}
		Xil_Out32(
					XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + mario->reg_l ),
					(unsigned int )0x8F000000 | (unsigned int )mario->type);
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + mario->reg_h ),
				(mario->y << 16) | mario->x);
		chhar_spawn(sword,sword_rotation);
		for(i = 0; i <3000000; i++) {}
		//After a short break (representing the attack animation) go back to standing sprite facing the same direciton
		if (last ==22 || last == 23) { 						//left
			mario->type =  LINK_SPRITES_OFFSET + 64*23;
		} else if (last == 3 || last == 4) { 				//right
			mario->type =  LINK_SPRITES_OFFSET + 64*4;
		} else if (last == 2 || last == 21) {				//up
			mario->type =  LINK_SPRITES_OFFSET + 64*21;
		} else if (last == 0 || last == 1) {				//down
			mario->type =  LINK_SPRITES_OFFSET + 64*1;
		}
	}

	if (!obstackles_detection(x,y, frame, dir)) {

		mario->x = x;
		mario->y = y;
	}

//Used this to determine if i'm in the right frame, -> I am+
	//mario->type = frame[21];

	print("Nesto");

	Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + mario->reg_l ),
				(unsigned int )0x8F000000 | (unsigned int )mario->type);
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + mario->reg_h ),
			(mario->y << 16) | mario->x);

	delete_sword(sword);
	for (i = 0; i < 1000; i++) { //1000 - good speed
	}

	return b_false;
}

bool tile_walkable(int index, unsigned short* map_frame) {
	int walkables[20] = {0, 2, 6, 10, 22, 27, 28, 29, 33, 34, 35, 39, 40, 41, 42, 43, 44, 45, 46, 47}; // the last row in Finaltiles is not included
	int i;

	for(i = 0; i < 20; i++) {
		if(map_frame[index] == SPRITES[walkables[i]]){ //ovo ne prolazi, znaci da ne nalazi adresu medju sprajtovima
			return true;
		}
	}

	return false;
}

bool obstackles_detection(int x, int y, unsigned short* f, /*unsigned char * map,*/
		int dir) {
			int x_left = x - SIDE_PADDING*16;
			int x_right = x + 13 - SIDE_PADDING*16;
			int y_top = y + 8 - (VERTICAL_PADDING + 5)*16;
			int y_bot = y + 15 - (VERTICAL_PADDING + 5)*16;

			x_left/=16;
			x_right/=16;
			y_top/=16;
			y_bot/=16;
			
			if (dir == DIR_UP) {
				return !(tile_walkable(x_left + y_top*FRAME_WIDTH, f) && tile_walkable(x_right + y_top*FRAME_WIDTH, f));
			} else if (dir == DIR_DOWN) {
				return !(tile_walkable(x_left + y_bot*FRAME_WIDTH, f) && tile_walkable(x_right + y_bot*FRAME_WIDTH, f));
			} else if (dir == DIR_LEFT) {
				return !(tile_walkable(x_left + y_top*FRAME_WIDTH, f) && tile_walkable(x_left + y_bot*FRAME_WIDTH, f));
			} else if (dir == DIR_RIGHT) {
				return !(tile_walkable(x_right + y_top*FRAME_WIDTH, f) && tile_walkable(x_right + y_bot*FRAME_WIDTH, f));
			}
			return false;
}

void battle_city() {

	print("Nesto0");
	unsigned int buttons, tmpBtn = 0, tmpUp = 0;
	int i, change = 0, jumpFlag = 0;
	int block;
	frame = overworld[0];
	mario.x = INITIAL_LINK_POSITION_X;
	mario.y = INITIAL_LINK_POSITION_Y;
	mario.type = LINK_SPRITES_OFFSET;
	sword.x = INITIAL_LINK_POSITION_X+13;
	sword.y = INITIAL_LINK_POSITION_Y;
	overw_x = INITIAL_FRAME_X;
	overw_y = INITIAL_FRAME_Y;
	map_reset(/*map1*/);
	map_update(&mario);

	//chhar_spawn(&enemie1);
	//chhar_spawn(&enemie2);
	//chhar_spawn(&enemie3);
	//chhar_spawn(&enemie4);
	chhar_spawn(&mario,0);
    load_frame(DIR_STILL);

	while (1) {

		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

		direction_t d = DIR_STILL;
		if (BTN_LEFT(buttons)) {
			d = DIR_LEFT;
		} else if (BTN_RIGHT(buttons)) {
			d = DIR_RIGHT;
		} else if (BTN_UP(buttons)) {
			d = DIR_UP;
		} else if (BTN_DOWN(buttons)) {
			d = DIR_DOWN;
		} else if (BTN_SHOOT(buttons)) {
			d = DIR_ATTACK;
		}

		mario_move(/*map1,*/ &mario, &sword, d);

		map_update(&mario);

		for (i = 0; i < 1; i++) {
		}

	}
}
