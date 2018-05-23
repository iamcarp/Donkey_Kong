#include "battle_city.h"
#include "map.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>
#include "sprites.h"

typedef int bool;
#define true 1
#define false 0


/*          COLOR PALETTE - base addresses in ram.vhd         */
#define FRAME_COLORS_OFFSET         0
#define LINK_COLORS_OFFSET          8
#define ENEMY_COLORS_OFFSET         35

/*		SCREEN PARAMETERS		 - in this case, "screen" stands for one full-screen picture	 */
#define SCREEN_BASE_ADDRESS			6900
#define SCR_HEIGHT					30
#define SCR_WIDTH					40
#define SPRITE_SIZE					16

/*		FRAME HEADER		*/
#define HEADER_BASE_ADDRESS			7192
#define HEADER_HEIGHT				5

/*      FRAME       */
#define FRAME_BASE_ADDRESS			7392 // FRAME_OFFSET in battle_city.vhd
#define SIDE_PADDING				12
#define VERTICAL_PADDING			7
#define INITIAL_FRAME_X				7
#define INITIAL_FRAME_Y				7
#define INITIAL_LINK_POSITION_X		200 + 64
#define INITIAL_LINK_POSITION_Y		270

/*      LINK SPRITES START ADDRESS - to move to next add 64    */
#define LINK_SPRITES_OFFSET             5172
#define SWORD_SPRITE                    6068
#define LINK_STEP						10

/*      ENEMIE SPRITES START ADDRESS - to move to next add 64    */
#define ENEMIE_SPRITES_OFFSET          4596
#define ENEMY_STEP						10

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
#define ENEMY_7_REG_L                  16
#define ENEMY_7_REG_H                  17
#define BASE_REG_L						0
#define BASE_REG_H	                    1


#define ENEMY_FRAMES_NUM 			34
/*			contains the indexes of frames in overworld which have enemies  	*/
bool ENEMY_FRAMES[] = {32, 33, 45, 48, 49, 55, 56, 62, 64, 65, 68, 73, 76, 79,
					    84, 85, 86, 87, 88, 90, 95, 99, 100, 101, 102, 103, 104,
						     105, 106, 110, 111, 120, 125, 126};

int lives = 0;
int counter = 0;
int last = 0; //last state link was in before current iteration (if he is walking it keeps walking)

/*		 ACTIVE FRAME		*/
unsigned short* frame;

typedef enum {
	DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL, DIR_ATTACK
} direction_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	direction_t dir;
	unsigned short sprite;
	bool destroyed;
	unsigned int reg_l;
	unsigned int reg_h;
} characters;

characters link = { 
		INITIAL_LINK_POSITION_X,		// x
		INITIAL_LINK_POSITION_Y,		// y
		DIR_DOWN, 	             		// dir
		0x0DFF,							// type - sprite address in ram.vhdl
		false,                		// destroyed
		LINK_REG_L,            		// reg_l
		LINK_REG_H             		// reg_h
		};

characters sword = {
		INITIAL_LINK_POSITION_X,		// x
		INITIAL_LINK_POSITION_Y,		// y
		DIR_LEFT,              			// dir
		SWORD_SPRITE,  					// type
		false,                		// destroyed
		WEAPON_REG_L,            		// reg_l
		WEAPON_REG_H             		// reg_h
		};

characters octorok = {
		0,								// x
		0,								// y
		DIR_LEFT,              			// dir
		SWORD_SPRITE,  					// type
		false,                		// destroyed
		ENEMY_2_REG_L,            		// reg_l
		ENEMY_2_REG_H             		// reg_h
		};

/*      indexes of the active frame in overworld        */
int overw_x;
int overw_y;

bool inCave = false;
/*      the position of the door so link could have the correct position when coming out of the cave    */
int door_x, door_y;

void load_frame( direction_t dir ) {
	if( !inCave ) {
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
		}

		frame = overworld[ overw_y * OVERWORLD_HORIZONTAL + overw_x ];
	} else {
		if ( dir == DIR_DOWN ) {
			frame = overworld[ overw_y * OVERWORLD_HORIZONTAL + overw_x ];
			inCave = false;
		} else {
			frame = CAVE;
		}
        //TODO: load_sprites(inCave);
	}

    /*      checking if there should be enemies on the current frame     */
    int i;
    int frame_index = overw_y * OVERWORLD_HORIZONTAL + overw_x;
    for ( i = 0; i < ENEMY_FRAMES_NUM; i++ ){
    	if( frame_index == ENEMY_FRAMES[i] ){
    		initialize_enemy( frame_index );
    	}
    }

    /*      loading next frame into memory      */
	set_frame_palette();
	int x,y;
	long int addr;
	for ( y = 0; y < FRAME_HEIGHT; y++ ) {
		for ( x = 0; x < FRAME_WIDTH; x++ ) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_BASE_ADDRESS + y * ( SIDE_PADDING + FRAME_WIDTH + SIDE_PADDING ) + x );
			Xil_Out32( addr, frame[ y * FRAME_WIDTH + x ] );
		}
	}

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

void set_header() {
    /*      TODO: add logic for updating the overworld position in header   */
    /*  idea: 1x2 gray sprites, position is 2x2 pixels     */

}

void initialize_enemy( int frame_index ) {
	//TODO:	define function
	/* set enemy on a random position 
	 * check if there is an obstacle on that position
	 * this function is only for initializing the enemy!
	 * enemy movement logic will be defined in an other function
	 * the enemy's position should depend on the frame
	 * in other words, it will use overw_x and overw_y
	 */
    
}

static void chhar_spawn( characters * chhar, int rotation ) {
	if ( rotation == 1 ) {																			 //rotate 90degrees clockwise
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
	}
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x );      //  the higher 2 bytes represent the row (y)
}

static void delete_sword( characters* chhar ){
	int i;
	for ( i = 0; i < 100000; i++ );         //  delay

	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
			(unsigned int) 0x80000000 | (unsigned int) chhar->sprite );
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x );      //  the higher 2 bytes represent the row (y)
}

/*  cleaning the registers used for moving characters sprites; two registers are used for each sprite   */
static void reset_memory() {
	unsigned int i;
	long int addr;

	for( i = 0; i < SCR_WIDTH*SCR_HEIGHT; i++ ) {
		addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( SCREEN_BASE_ADDRESS + i );
		Xil_Out32( addr, SPRITES[10] );             // SPRITES[10] is a black square
	}

	for ( i = 0; i <= 20; i += 2 ) {
		Xil_Out32( XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ), (unsigned int) 0x0F000000);
	}
}

static bool link_move(characters * link, characters* sword, direction_t dir) {
	unsigned int x;
	unsigned int y;
	int obstackle = 0;
	int sword_rotation = 0;
	int lasting_attack = 0;
	int i;

	/*      change frame if on the edge     */
    if (link->x > ( ( SIDE_PADDING + FRAME_WIDTH ) * SPRITE_SIZE  - SPRITE_SIZE)){
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
	}

    /*      get the current position of link    */
	x = link->x;
	y = link->y;

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

	/*      movement animation      */
	if ( dir == DIR_LEFT ) {
		x--;
		if ( counter%LINK_STEP == 0 ) {
			last = ( last == 22 ) ? 23 : 22;
		}
		lasting_attack = 0;
		link->sprite = LINK_SPRITES_OFFSET + 64 * last;
		counter++;
	} else if ( dir == DIR_RIGHT ) {
		x++;
		if ( counter % LINK_STEP == 0 ) {
			last = (last == 3) ? 4 : 3;
		}
		lasting_attack = 0;
		//TODO:	set sprite
		link->sprite =  LINK_SPRITES_OFFSET + 64 * last;
		counter++;
	} else if ( dir == DIR_UP ) {
		y--;
		if ( counter % LINK_STEP == 0 ) {
			last = (last == 2) ? 21 : 2;
		}
		lasting_attack = 0;
		link->sprite = LINK_SPRITES_OFFSET + 64 * last;
		counter++;
	} else if ( dir == DIR_DOWN ) {
		y++;
		if ( counter % LINK_STEP == 0) {
			last = (last == 0) ? 1 : 0;
		}
		lasting_attack = 0;
		link->sprite = LINK_SPRITES_OFFSET + 64 * last;
		counter++;
	} else if ( dir == DIR_ATTACK ){
		switch( last ){
			case 0:                     //down
				sword->x = link->x;
				sword->y = link->y + SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 9;
				sword_rotation = 1;
				break;
			case 1:                     //down
				sword->x = link->x;
				sword->y = link->y + SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 9;
				sword_rotation = 1;
				break;
			case 2:                     //up
				sword->x = link->x;
				sword->y = link->y - SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 10;
				sword_rotation = 2;
				break;
			case 21:                    //up
				sword->x = link->x;
				sword->y = link->y - SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 10;
				sword_rotation = 2;
				break;
			case 3:                     //right
				sword->x = link->x + SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 11;
				sword_rotation = 0;
				sword->y = link->y;
				break;
			case 4:                     //right
				sword->x = link->x + SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 11;
				sword_rotation = 0;
				sword->y = link->y;
				break;
			case 22:                    //left
				sword->x = link->x - SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 26;
				sword_rotation = 3;
				sword->y = link->y;
				break;
			case 23:                    //left
				sword->x = link->x - SPRITE_SIZE;
				link->sprite =  LINK_SPRITES_OFFSET + 64 * 26;
				sword_rotation = 3;
				sword->y = link->y;
				break;
		}
		if ( lasting_attack != 1 ){
			Xil_Out32(
					XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_l ),
					(unsigned int) 0x8F000000 | (unsigned int) link->sprite);
			Xil_Out32(
					XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_h ),
					 ( link->y << 16) | link->x);      //  the higher 2 bytes represent the row (y)
		}

		for ( i =0; i <900000; i++ );             //      delay

		if ( lasting_attack != 1 ){
			chhar_spawn( sword, sword_rotation );
		}
		if ( dir == DIR_ATTACK) {
			lasting_attack = 1;
		}

		for ( i =0; i <3000000; i++);             //      delay

		/*   After a short break (representing the attack animation), go back to standing sprite facing the same direciton    */
		if ( last ==22 || last == 23 ) { 						//left
			link->sprite = LINK_SPRITES_OFFSET + 64 * 23;
		} else if ( last == 3 || last == 4 ) { 				//right
			link->sprite = LINK_SPRITES_OFFSET + 64 * 4;
		} else if ( last == 2 || last == 21 ) {				//up
			link->sprite = LINK_SPRITES_OFFSET + 64 * 21;
		} else if ( last == 0 || last == 1 ) {				//down
			link->sprite = LINK_SPRITES_OFFSET + 64 * 1;
		}
	}

    if( !inCave && isDoor(x, y) ) {
        link->x = ( SIDE_PADDING + (int) FRAME_WIDTH/2 ) * SPRITE_SIZE;                         // set to the middle of the frame
        link->y = ( VERTICAL_PADDING + HEADER_HEIGHT + FRAME_HEIGHT - 1 ) * SPRITE_SIZE;        //set to the bottom of the cave
        inCave = true;
        load_frame( DIR_UP );
        //TODO: write_introduction();
	    /*		skip collision detection if on the bottom of the frame 			*/
    } else if( dir == DIR_DOWN && y == (( VERTICAL_PADDING + FRAME_HEIGHT + HEADER_HEIGHT - 1 ) * SPRITE_SIZE + 1)) {
		link->x = x;
		link->y = y;
	} else {
		if ( !obstackles_detection(x, y, frame, dir ) ) {
			link->x = x;
			link->y = y;
		}
	}


	if ( lasting_attack != 1 ){
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_l ),
				(unsigned int) 0x8F000000 | (unsigned int) link->sprite );
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + link->reg_h ),
				(link->y << 16) | link->x );      //  the higher 2 bytes represent the row (y)
		delete_sword( sword );
	}

	for (i = 0; i < 1000; i++);          //     delay = 1000 <- good speed

	return false;
}

bool isDoor(x,y) {
    /*      calculate the index of the position in the frame     */
	x = x + 13 - SIDE_PADDING*SPRITE_SIZE;
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
	int walkables[20] = {0, 2, 6, 10, 22, 27, 28, 29, 33, 34, 35, 39, 40, 41, 42, 43, 44, 45, 46, 47, 49}; 
	int i;

	for ( i = 0; i < 20; i++) {
        /*      check if the current sprite is walkable     */
		if (  map_frame[index] == SPRITES[walkables[i]] ){ 
			return true;
		}
	}

	return false;
}

bool obstackles_detection(int x, int y, unsigned short* f, int dir) {
			int x_left = x + 3 - SIDE_PADDING * SPRITE_SIZE;
			int x_right = x + 12 - SIDE_PADDING * SPRITE_SIZE;
			int y_top = y + 11 - (VERTICAL_PADDING + HEADER_HEIGHT) * SPRITE_SIZE;
			int y_bot = y + 15 - (VERTICAL_PADDING + HEADER_HEIGHT) * SPRITE_SIZE;

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

			return false;
}

void battle_city() {
	unsigned int buttons, tmpBtn = 0, tmpUp = 0;
	int i;
    
    /*      initialization      */
	reset_memory();
	overw_x = INITIAL_FRAME_X;
	overw_y = INITIAL_FRAME_Y;
    load_frame( DIR_STILL );

	link.x = INITIAL_LINK_POSITION_X;
	link.y = INITIAL_LINK_POSITION_Y;
	link.sprite = LINK_SPRITES_OFFSET;
	sword.x = INITIAL_LINK_POSITION_X + 13;
	sword.y = INITIAL_LINK_POSITION_Y;

	chhar_spawn(&link, 0);

	while (1) {
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

		link_move(&link, &sword, d);
		//if (enemies_exist) {
		//TODO: enemy_move(); }

	}
}
