#include "sprites.h"

void load_sprites(bool textMode) {
    int i;
    unsigned int * load;
    long int addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4*(1599);  //  1599 = base address of sprites[21]
    load = textMode ? VHDL_CHAR_SPRITES : VHDL_reload_map;
    
    for (i = 0; i < 30*64; i++) {
	    Xil_Out32(addr, load[i]); 
    }
}

int char_to_addr(char c) {
    if(c > '0' - 1 && c < '9' + 1) {
        return CHAR_SPRITES[c-48];
    } else if (c > 'a' - 1 && c < 'b' + 1) {
        return CHAR_SPRITES[c-65+10];
    } else if (c > 'A' - 1 && c < 'B' + 1) {
        return CHAR_SPRITES[c-97+10];
    } else {
        return CHAR_SPRITES[0];     // fix case for signs
    }
}

void write_line(char* text, int len, long int addr) {
    int i, j;
    for (i = 0; i < len; i++) {
		Xil_Out32(addr+4*i, char_to_addr(text[i])); 
        for (j = 0; j<1000; j++);               //      delay
    }
}

void write_introduction() {
    char text[] = "its dangerous";
    short len = 13;
    long int addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_BASE_ADDRESS + SCR_WIDTH + 1 );
    write_line(text , len, addr);

    text[] = "to go alone.";
    len = 12;
    addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_BASE_ADDRESS + SCR_WIDTH*2 + 1 );
    write_line(text , len, addr);

    text[] = "take this.";
    len = 10;
    addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( FRAME_BASE_ADDRESS + SCR_WIDTH*3 + 1 );
    write_line(text , len, addr);
}
