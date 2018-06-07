from overworld import *
from sprites import *

def cave_frame_c():
    cave = open("cave.txt", "w")
    cave.write("unsigned short cave[11*16] = {")

    for y in range(11):
        for x in range(16):
            value = "0x017F, "      #2
            if x in [0,15] or y == 0:
                value = "0x05BF, "      #19
            elif y == 1:
                if x == 1:
                    value = "0x05FF, "  #20
                elif x == 14: 
                    value = "0x057F, "  #18
            elif y == 9:
                if x == 1:  
                    value = "0x047F, "  #14
                elif x == 14:
                    value = "0x03FF, "  #12
            elif y == 10:
                if x in [7,8]:
                    value = "0x017F, "  #2
                else:
                    value = "0x043F, "  #13
            cave.write(value)
        cave.write("\n\t\t\t\t\t\t")

    cave.write("}\n")
    cave.close()

def character_sprites_to_c(name, sprites, file_name, offset, values):
    output = open(file_name, "w")
    tiles_width = 16
    # spriteHDLoffset represents the offset of the specific sprite in ram.vhd
    i_range = len(sprites)    

    for i in range(i_range):
        if values[i] in ['I','T','S','D','A','N','G','E','R','O','U','L','K','H','F', '\'', ',','.']: 
            spriteHDLoffset = offset + i*64  
            output.write("unsigned short CHAR_" + values[i] + " = 0x%0.4X; \n" % spriteHDLoffset)

    output.close()
    
def letters_to_VHDL(sprites, file_name, offset, black, white, values):
    # offset = sprites offset in ram.vhdl
    # the goal is to get values in this format:   ---  255 => x"01020202" --- where   01 02 02 02    <- each block is an index for one color - we want to store them in sets of 4

    VHDL = open(file_name, "w")
    for i in range(len(sprites)): 
        if values[i] in ['I','T','S','D','A','N','G','E','R','O','U','L','K','H','F', '\'', ',', '.']: 
            VHDL.write("\n                --  sprite -" + values[i] + "-\n")
            temp = "        " + str(offset) + " => x\""
            s = sprites[i] 
            for j in range(len(s)):
                if s[j]:
                    temp += "%0.2X" % white
                else:
                    temp += "%0.2X" % black     
                if not (j+1)%4:
                    temp += "\",\n"
                    VHDL.write(temp)
                    offset += 1
                    temp = "        " + str(offset) + " => x\""
     
    VHDL.close()



def character_sprites_to_VHDL(sprites, file_name, offset, palette_offset, max_len = None):
    # indexed-colored sprites - each sub-list is one sprite // from FinalTilesColors Matrix.txt
    # offset = sprites offset in ram.vhdl
    # the goal is to get values in this format:   ---  255 => x"01020202" --- where   01 02 02 02    <- each block is an index for one color - we want to store them in sets of 4
    if(max_len):
        i_range = max_len
    else:
        i_range = len(sprites) 
    
    VHDL = open(file_name, "w")
    for i in range(i_range):  
        #   only for loading Link sprites - skip boomerang
        if i in range(15,21) :
            continue
        VHDL.write("\n                --  sprite " + str(i) + "\n")
        temp = "        " + str(offset) + " => x\""
        s = sprites[i] 
        for j in range(len(s)):
            if s[j]:
                temp += "%0.2X" % (palette_offset + s[j])
            else:
                temp += "%0.2X" % s[j]      #   in case the index is 0, it should be transparent, so we leave it at 0
            if not (j+1)%4:
                temp += "\",\t\t-- colors: "+str(palette_offset+s[j-3])+", "+str(palette_offset+s[j-2])+", "+str(palette_offset+s[j-1])+", "+str(palette_offset+s[j]) + "\n"
                VHDL.write(temp)
                offset += 1
                temp = "        " + str(offset) + " => x\""
     
    VHDL.close()


def overworld_sprites_to_VHDL(sprites):
    # overworld sprites are loaded with a seperate function because not all of them are used - the doubles are remapped to the originals and the grave is moved to the end 
    # indexed-colored sprites - each sub-list is one sprite // from FinalTilesColors Matrix.txt
    # offset = sprites offset in ram.vhdl
    # the goal is to get values in this format:   ---  255 => x"01020202" --- where   01 02 02 02    <- each block is an index for one color - we want to store them in sets of 4
    
    offset = 255
    VHDL = open("VHDL_overworld_sprites.txt", "w")
    output = open("c_reload_overworld.txt", "w")
    m = 0
    for i in range(51):  #   len(sprites)//3-3 -> the last row isn't full
        VHDL.write("\n                --  sprite " + str(i) + "\n")
        temp = "        " + str(offset) + " => x\""
        if i < 50:     #   len(sprites)//3-2 -> all before last
            s = sprites[m] 
        else:
            s = sprites[31]     #   add the grave to the end of the list
        for j in range(len(s)):
            temp += "0" + str(s[j])
            if i > 20:
                output.write("0" + str(s[j]))
            if not (j+1)%4:
                if i > 20:
                    output.write(", 0x")
                temp += "\",\n"
                VHDL.write(temp)
                # move to next line
                offset += 1
                temp = "        " + str(offset) + " => x\""
        if (m%18) == 5:
            m += 12
        m += 1        
     
    output.close()
    VHDL.close()


def simplifyMap(overworld):
    # adjusting map values to the new version of overworld sprites tiles, defined in overworld_sprites_to_VHDL()
    for i in range(len(overworld)):
        for j in range(len(overworld[i])):
            if overworld[i][j]%18 > 11:
                if overworld[i][j]//18 == 2 and overworld[i][j]%18 == 16:   #   white eye
                    overworld[i][j] = 144
                elif overworld[i][j]//18 == 1 and overworld[i][j]%18 == 13: #   grave
                    overworld[i][j] = 146
                else:
                    overworld[i][j] -= 12 
            elif overworld[i][j]%18 > 5:
                if overworld[i][j]//18 == 8:
                    if overworld[i][j]%18 == 6:    #   white eyes - from the bottom row
                        overworld[i][j] = 40
                    elif  overworld[i][j]%18 == 9:    #   green eye - from the bottom row
                        overworld[i][j] = 144
                    elif overworld[i][j]%18 == 7:    #   white doors left - from the bottom row
                        overworld[i][j] = 57
                    elif overworld[i][j]%18 == 8:    #   white doors right - from the bottom row
                        overworld[i][j] = 59
                    else:
                        overworld[i][j] -= 6
                else:
                    overworld[i][j] -= 6
            elif overworld[i][j]//18 == 8:
                if overworld[i][j]%18 in [2,3,5]:    #   trees - from the bottom row
                    overworld[i][j] = 19
                elif overworld[i][j]%18 == 4:    #   white left sphere - from the bottom row
                    overworld[i][j] = 39
                

def overworld_to_c(overworld):
    # transforms map into a in array of frames 
    # each frame is made up from an array of sprites

    file2 = open("c_overworld.txt", "w")

    sprites_base = 255  # sprites base address in ram.vhd
	
    horizontal_pad = 12
    vertical_pad = 7
    header_height = 5

    map_height = 11     # frame height
    map_width = 16      # frame width
    overw_height = 88
    overw_width = 256

    # spriteHDLoffset represents the offset of the specific sprite in ram.vhd

    for overw_y in range(0, overw_height, map_height):
        i_start = overw_y
        i_end = overw_y + map_height
        for overw_x in range(0, overw_width, map_width):
            j_start = overw_x
            j_end = overw_x + map_width
            file2.write("\n//     map(" + str(overw_x//map_width) + ", " + str(overw_y//map_height) + ")\n")    # write the position of the curent frame in overworld
            file2.write("{")
            for i in range(i_start, i_end):
                    for j in range(j_start, j_end):
                        x = overworld[i][j]
                        spriteHDLoffset = sprites_base+((x//18)*6+x%18)*64   # 18 is the width of the FinalTiles matrix, 6 is the number of colums that we used from that matrix
                        file2.write("0x%0.4X" % spriteHDLoffset)
                        # add a coma after every sprite, except after the last one - the end of the array
                        if (i != overw_y + map_height - 1 or j != overw_x + map_width - 1):
                            file2.write(",")      
                        file2.write(" // x = "  + str((j)%map_width) + ", y = " + str(i%map_height) + "\n")

            file2.write("},\n")
            
    file2.close()


def screen_in_VHDL(overworld, offset):
    # this function generates the view of one screen
    # the frame loaded doesn't matter, because it's overwritten in battle_city.c
    # the point is to get the starting (base) addresses of the header and the frame itself in ram,vhdl

    file1 = open("VHDL_screen.txt", "w")
    file1.write("\n                --  MAP\n")
	
    horizontal_pad = 12
    vertical_pad = 7
    header_height = 5

    map_height = 11
    map_width = 16

    # spriteHDLoffset represents the offset of the specific sprite in ram.vhd

    # frame position in overworld
    overw_x = 16
    overw_y = 16

    i_start = overw_y - vertical_pad
    i_end = overw_y + map_height + header_height + vertical_pad
    j_start = overw_x - horizontal_pad
    j_end = overw_x + map_width + horizontal_pad
    for i in range(i_start, i_end, 1):
        for j in range(j_start, j_end, 1):
            if(i>overw_y-1 and i<overw_y+map_height and j>overw_x-1 and j<overw_x+map_width):
                x = overworld[i][j]
                spriteHDLoffset = 255+((x//18)*6+x%18)*64
                temp = "        " + str(offset) + " => x\"%0.8X\"," % x
                temp += " -- z: 0 rot: 0 ptr: " 
                temp += str(spriteHDLoffset) + "\n"
            elif (i>overw_y-header_height and i<overw_y and j>overw_x-1 and j<overw_x+map_width):
                temp = "        " + str(offset) + " => x\"00000016\", -- header \n"     # points to a black sprite
            else:
                temp = "        " + str(offset) + " => x\"00000016\", -- pedding \n"    # points to a black sprite
            file1.write(temp)
            offset += 1
            
    file1.close()

def generate_minimap_VHDL(file_name, offset, color):
    VHDL = open(file_name, "w")
    for i in range(2):
        VHDL.write("\n          -- minimap " + str(i))
        for j in range(64):
            VHDL.write("\n\t\t\t")
            VHDL.write(str(offset))
            VHDL.write(" => x\"" + "%0.2X" % color +  "%0.2X" % color +  "%0.2X" % color + "%0.2X" % color + "\",")
            offset+=1

    VHDL.close()

#simplifyMap(overworld)
#screen_in_VHDL(overworld, 6992)
#overworld_to_c(overworld)

#   overworld sprites
#overworld_sprites_to_VHDL(overworld_sprites)

#   Link sprites
#character_sprites_to_VHDL(link_sprites, "VHDL_Link_sprites.txt", 5648, 8, 27) 

#   enemies sprites
#character_sprites_to_VHDL(enemies_sprites, "VHDL_enemies_sprites.txt", 5072, 35) 

#  heart sprites
#character_sprites_to_VHDL(hearts, "VHDL_heart_sprites.txt", 4671, 57) 

#   Numbers and letters     -   the functions read only certain letters becaouse there is not enough memory in ram.vhd 
#values = ['0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',',','!','\'','&','.','\"','?','-']
#character_sprites_to_c("CHAR_SPRITES", numbers_and_letters, "c_numbers_and_letters.txt", 3519, values) 
#letters_to_VHDL(numbers_and_letters, "VHDL_letters_sprites.txt", 3519, 2, 15, values) 

#   generate Cave frame
#cave_frame_c()

#   generate mini map sprites for header
generate_minimap_VHDL("VHDL_minimap.txt", 4863, 7)
