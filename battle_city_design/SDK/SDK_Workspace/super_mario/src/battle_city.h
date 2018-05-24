

#ifndef BATTLE_CITY_H_
#define BATTLE_CITY_H_

#define FRAME_WIDTH           16
#define FRAME_HEIGHT          11

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480


unsigned short char_to_addr(char c);
void write_line(char* text, int len, long int addr):
void write_introduction();



void battle_city();

#endif /* BATTLE_CITY_H_ */
