import numpy as np
import cv2
import pprint

#siva boja izmedju spritova je [192, 192, 192]

sprite_height = 16
sprite_width = 16
Tilecolors = []
Mapcolors = []

#def draw(img):
#	while(1):
#		cv2.imshow('img',img)
#		k = cv2.waitKey(33)
#		if k == 27:
#			cv2.destroyAllWindows()
#			break
#		elif k == -1:
#			continue
#		else:
#			print(k)


def draw(name, img):
    #img    - the image you want to draw
    cv2.imshow(name,img)
    k  = cv2.waitKey(0)
    cv2.destroyAllWindows()
    return k

def enlarge(img, x, height, width):
    #img    - the image you want to enlarge
    #x      - how many times you want to enlarge it

    Enlarged = np.zeros((height*x,width*x,3), np.uint8)

    for i in range(height):
        for j in range(width):
            for ii in range(x):
                for jj in range(x):
                    b = img[i, j]
                    Enlarged[x*i+ii,x*j+jj] = b

    return Enlarged

def defSprites(dict):
    #dict - dictionary to which to write
    tiles = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")
    global Tilecolors
    x = 0
    y = 0
    br = 0
    while(y <8):
        while(x < 18):
            dict[br] = np.zeros((sprite_height,sprite_width,3), np.uint8)
            for i in range(16):
                for j in range(16):
                    dict[br][i,j] = tiles[i+1+y+y*16,j+1+x*16+x]
                    rembemberColor(dict[br][i,j], Tilecolors);
            br = br + 1
            x = x+1
        x = 0
        y = y+1
    return dict

def rembemberColor(c, colors):
    for i in range(len(colors)):
        if colors[i][0] == c[0] and colors[i][1] == c[1] and colors[i][2] == c[2]:
             return colors
    colors.append(c)
    return colors

#def getColors(img, shape, colors):
#    height = shape[0]
#    width = shape[1]
#    for i in range(height):
#        for j in range(width):
#            rememberColor(img[i,j], colors)
#    return colors

def drawColors(name, colors):
    colorImage = np.zeros((16,16*len(colors),3), np.uint8)
    for i in range(16):
        for j in range(16*len(colors)):
            colorImage[i,j]=colors[j//16]

    draw(name, enlarge(colorImage, 5, 16, 16*len(colors)))
    return


def drawDict():
    for i in range(len(dict)):
        if draw('Dictionary', enlarge(dict[i], 10, 16, 16)) == 27:
            return None

def compare(sprite1, sprite2):
    for i in range(sprite_height):
        for j in range(sprite_width):
            #if sprite1[i,j][0] != sprite2[i,j][0] and sprite1[i,j][1] != sprite2[i,j][1] and sprite1[i,j][2] != sprite2[i,j][2]:
            if sprite1[i,j][0] != sprite2[i,j][0] and sprite1[i,j][1] != sprite2[i,j][1] and sprite1[i,j][2] != sprite2[i,j][2]:
                print("Values are sprite1: [{},{},{}] and sprite2: [{},{},{}]".format(sprite1[i,j][0],sprite1[i,j][1],sprite1[i,j][2],sprite2[i,j][0],sprite2[i,j][1],sprite2[i,j][2]))
                return False
    return True

print("Printing Tile Image...")
tiles = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")
print("Image Shape is:")
pprint.pprint(tiles.shape)
draw('Tiles', tiles)

dict = {}
defSprites(dict)

print("TileColors length is: ", len(Tilecolors))
drawColors("Tilecolors", Tilecolors)

#Mapcolors = getColors(tiles, tiles.shape , Mapcolors)

#print("MapColors length is: ", len(Mapcolors))
#drawColors("Mapcolors", Mapcolors)


print("Listing Tiles...")
print("Press ESC to skip")
drawDict()

#print("Printing Map...")
test = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\ZeldaOverworldMapQ1BG.png")
#draw("Map", test)

tsprite = np.zeros((sprite_height,sprite_width,3), np.uint8)
y = 0
x = 16
for i in range(16):
    for j in range(16):
        tsprite[i,j] = test[i+y+y*16,j+x*16+x]
        test[i+y+y*16,j+x*16+x] = [255,255,255]        

while(1):
    print("Drawing extracted sprite from Map...")
    draw("Extracted sprite", enlarge(tsprite, 10, 16, 16))
    print("Drawing Tile...")
    draw("Tile", enlarge(dict[55], 10, 16, 16))           

print("Drawing Map to detect position of extracted sprite...")
draw("Map", test)

print("Comparing the two sprites, result: ")
print(compare(tsprite, dict[55]))
