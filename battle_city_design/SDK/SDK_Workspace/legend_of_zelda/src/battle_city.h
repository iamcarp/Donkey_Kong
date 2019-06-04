

#ifndef BATTLE_CITY_H_
#define BATTLE_CITY_H_

#define FRAME_WIDTH           20
#define FRAME_HEIGHT          15

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480

typedef int bool;
#define true 1
#define false 0

typedef enum {
	DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL, DIR_ATTACK
} direction_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	direction_t dir;
	unsigned short sprite;
	bool active;
	unsigned int reg_l;
	unsigned int reg_h;
} characters;

typedef struct {
	unsigned int x[6];
	unsigned int y[6];
	direction_t dir;
	unsigned short sprites[6];
	bool active;
	unsigned int reg_l[6];
	unsigned int reg_h[6];
} monkey;

unsigned int random_number();
unsigned short char_to_addr(char c);
void write_line(char* text, int len, long int addr);
static void write_introduction();
void load_frame( direction_t dir );
void set_frame_palette();
void set_grandpa();
void set_sword();
void pick_up_sword();
void set_minimap();
void set_pickups();
void set_health(int health);
void set_header();
bool initialize_enemy( int frame_index);
direction_t random_direction(direction_t dir, int );
void enemy_move(characters* chhar, int);
void chhar_delete();
void chhar_spawn( characters * chhar, int rotation );
void delete_sword( characters* chhar );
void reset_memory();
bool link_move(characters * link, characters* sword, direction_t dir);
bool isDoor(int x, int y);
bool tile_walkable(int index, unsigned short* map_frame);
bool obstackles_detection(int x, int y, unsigned short* f, int dir, bool isLink);
void set_fire();


void battle_city();

#endif /* BATTLE_CITY_H_ */
