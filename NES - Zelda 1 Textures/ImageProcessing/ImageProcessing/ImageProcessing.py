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

#draws the image in a seperate window
def draw(name, img):
    #img    - the image you want to draw
    cv2.imshow(name,img)
    k  = cv2.waitKey(0)
    cv2.destroyAllWindows()
    return k

#returns the input image in greater size for better view
def enlarge(img, x):
    #img    - the image you want to enlarge
    #x      - how many times you want to enlarge it
    height = img.shape[0]
    width = img.shape[1]
    Enlarged = np.zeros((height*x,width*x,3), np.uint8)

    for i in range(height):
        for j in range(width):
            for ii in range(x):
                for jj in range(x):
                    b = img[i, j]
                    Enlarged[x*i+ii,x*j+jj] = b

    return Enlarged

#Saves sprites from the input image into seperate images inside the dictionary - designed to work with the image given, may be changed for different images
def defSprites(dict, path):
    #dict - dictionary to which to write
    tiles = cv2.imread(path)
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
                    rememberColor(dict[br][i,j], Tilecolors);
            br = br + 1
            x = x+1
        x = 0
        y = y+1
    return dict

#checks if the color is present, if not adds it to the list
def rememberColor(c, colors):
    for i in range(len(colors)):
        if colors[i][0] == c[0] and colors[i][1] == c[1] and colors[i][2] == c[2]:
             return colors
    colors.append(c)
    return colors

#returns an array of all the colors in the image
def getColors(img):
    colors = []
    height = img.shape[0] 
    width = img.shape[1]
    for i in range(height):
        for j in range(width):
            rememberColor(img[i,j], colors)
    return colors

#draws the list of colors in image format and returns the image
def drawColors(name, colors):
    colorImage = np.zeros((16,16*len(colors),3), np.uint8)
    for i in range(16):
        for j in range(16*len(colors)):
            colorImage[i,j]=colors[j//16]

    draw(name, enlarge(colorImage, 5))
    return colorImage

#Lists out all the images inside the dictionary one after the other - is interruped by pressing ESC
def drawDict():
    for i in range(len(dict)):
        if draw('Dictionary', enlarge(dict[i], 10)) == 27:
            return None

#compares the two sprites, compares them pixel color by pixel color, if difference found, prints out the difference found and returns false else returns true, expects sprites to be the same size
def compare(sprite1, sprite2):
    for i in range(sprite1.shape[0]):
        for j in range(sprite1.shape[1]):
            if sprite1[i,j][0] != sprite2[i,j][0] and sprite1[i,j][1] != sprite2[i,j][1] and sprite1[i,j][2] != sprite2[i,j][2]:
                #print("Values are sprite1: [{},{},{}] and sprite2: [{},{},{}] on position[{},{}] ".format(sprite1[i,j][0],sprite1[i,j][1],sprite1[i,j][2],sprite2[i,j][0],sprite2[i,j][1],sprite2[i,j][2], i, j))
                return False
    return True

#copies a image to another, used as to not assign references to the same adress
def copyImg(img):
    copy = np.zeros((img.shape[0],img.shape[1],3), np.uint8)
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            copy[i,j]=img[i,j]
    return copy

#fixes colors from the original TilePallete to the one used in the map image
def fixColors(sprite, cantFind):
    global Tilecolors
    global Mapcolors
    #drawColors("Map colors", Mapcolors)

    for i in range(sprite.shape[0]):
        for j in range(sprite.shape[1]):
            if (sprite[i,j][0] == Tilecolors[0][0] and sprite[i,j][1] == Tilecolors[0][1] and sprite[i,j][2] == Tilecolors[0][2]) or (sprite[i,j][0] == Tilecolors[8][0] and sprite[i,j][1] == Tilecolors[8][1] and sprite[i,j][2] == Tilecolors[8][2]):
                for k in range(3):
                    sprite[i,j][k] = Mapcolors[1][k]

            elif sprite[i,j][0] == Tilecolors[1][0] and sprite[i,j][1] == Tilecolors[1][1] and sprite[i,j][2] == Tilecolors[1][2]:
                for k in range(3):
                    sprite[i,j][k] = Mapcolors[0][k]

            elif sprite[i,j][0] == Tilecolors[2][0] and sprite[i,j][1] == Tilecolors[2][1] and sprite[i,j][2] == Tilecolors[2][2]:
                for k in range(3):
                    sprite[i,j][k] = Mapcolors[2][k]

            elif (sprite[i,j][0] == Tilecolors[3][0] and sprite[i,j][1] == Tilecolors[3][1] and sprite[i,j][2] == Tilecolors[3][2]) or (sprite[i,j][0] == Tilecolors[7][0] and sprite[i,j][1] == Tilecolors[7][1] and sprite[i,j][2] == Tilecolors[7][2]):
                for k in range(3):
                    sprite[i,j][k] = Mapcolors[3][k]

            elif sprite[i,j][0] == Tilecolors[4][0] and sprite[i,j][1] == Tilecolors[4][1] and sprite[i,j][2] == Tilecolors[4][2]:
                for k in range(3):
                    sprite[i,j][k] = Mapcolors[4][k]

            elif sprite[i,j][0] == Tilecolors[5][0] and sprite[i,j][1] == Tilecolors[5][1] and sprite[i,j][2] == Tilecolors[5][2]:
                for k in range(3):
                    sprite[i,j][k] = Mapcolors[5][k]

            elif sprite[i,j][0] == Tilecolors[6][0] and sprite[i,j][1] == Tilecolors[6][1] and sprite[i,j][2] == Tilecolors[6][2]:
                for k in range(3):
                    sprite[i,j][k] = Mapcolors[6][k]
            elif sprite[i,j][0] == 192 and sprite[i,j][1] == 192 and sprite[i,j][2] == 192:
                #this is the gray outline of the tiles in the image
                x = 5 #this was added just so there is no empty if 
            else:
                print("Can't find color:", sprite[i,j])
                if (str(sprite[i,j][0]) + " " + str(sprite[i,j][1]) + " " + str(sprite[i,j][2])) not in cantFind:
                    cantFind.append(str(sprite[i,j][0]) + " " + str(sprite[i,j][1]) + " " + str(sprite[i,j][2]))
    return  sprite, cantFind

#creates final Tiles which will be exported for further use
def createFinalTiles(dict):
    height = 8 + 1
    width = 18
    FTiles = np.zeros((height * 16,width * 16,3), np.uint8)
    for i in range(height*16):
        for j in range(width*16):
            if (i//16*18 + j//16) < len(dict):
                FTiles[i,j] = dict[i//16*18 + j//16][i%16, j%16]
    return  FTiles

def printMatrix(matrix, height):
    print("Printing Matrix")
    for i in range(height):
        print(matrix[i])
    return

def FillMatrix(dict, mapImage):
    matrix = []
    for i in range(mapImage.shape[0]//16):
        row = []
        for j in range(mapImage.shape[1]//16):
            row.append([])
        matrix.append(row)

    #pprint.pprint(matrix)
    newOnes = 0
    for i in range(mapImage.shape[0]//16):
        for j in range(mapImage.shape[1]//16):
            found = 0
            for k in range(len(dict)):
                if compare(extractSprite(mapImage, i, j), dict[k]):
                    matrix[i][j]=k
                    #print(k)
                    #draw("comparing", enlarge(dict[k], 10))
                    #draw("comparing", enlarge(extractSprite(mapImage, i, j), 10))
                    found = 1
                elif k == len(dict)-1 and found == 0:
                    #draw("Couldn't find", enlarge(extractSprite(mapImage, i, j),10))  
                    #drawDict()     
                    newOnes +=1   
                    dict[143 + newOnes] = copyImg(extractSprite(mapImage, i, j))
                    matrix[i][j]=143+newOnes
        #print row
        print(matrix[i])
    #pprint.pprint(matrix)
    printMatrix(matrix, mapImage.shape[0]//16)
    return


def extractSprite(map, startHight, startWidth):
        #EXTRACTING SPRITE FROM THE MAP AND CHECKING HIS VALUES
    tsprite = np.zeros((16,16,3), np.uint8)

    for i in range(16):
        for j in range(16):
            tsprite[i,j] = map[startHight*16 + i,startWidth*16 + j]

    #draw(str(startHight) + ", " + str(startWidth), enlarge(tsprite,10))

    return tsprite



print("Printing Tile Image...")
tiles = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")
#tiles = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")
print("Image Shape is:")
pprint.pprint(tiles.shape)
draw('Tiles', enlarge(tiles, 3))

#print("Printing Map...")
map = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\ZeldaOverworldMapQ1BGcroped.png")
#draw("Map", map)
#print(map.shape)


#REMOVING BLACK LINE ON THE LEFT OF IMAGE
i = 0
while(map[0,i][0] == 0 and map[0,i][2] == 0 and map[0,i][2] == 0):
    i+=1

x = i
newMap = np.zeros((map.shape[0],map.shape[1]-x,3), np.uint8)

newMap = map[0:map.shape[0],x:map.shape[1]]
#draw("NEW Map", newMap)
print("new map shape:")
print(newMap.shape)

cv2.imwrite("OverworldCroped.png", newMap)



dict = {}
defSprites(dict, "C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")

print("TileColors length is: ", len(Tilecolors))
print("TileColors are (BGR format): ")
pprint.pprint( Tilecolors)
drawColors("Tilecolors", Tilecolors)

print("Loading Map colors:")
Mapcolors = getColors(map)

print("MapColors length is: ", len(Mapcolors))
print("MapColors are (BGR format): ")
pprint.pprint( Mapcolors)
drawColors("Mapcolors", Mapcolors)

#print("Listing Tiles...")
#print("Press ESC to skip")
#drawDict()


#FOR CHECKING WHETHER IT CANT FIND A CIRTAIN COLOR
#cantFind = []
#for i in range(len(dict)):
#    dict[i], cantFind = fixColors(dict[i], cantFind)
#print(cantFind)
#drawDict()




#TESTING ALL COLORS INSIDE TILES
#print("Processing folowing Images:")
#cantFind = []
#oldTiles = copyImg(drawColors("OLD",Tilecolors))
#tmp, cantFind = fixColors(drawColors("NEW", Tilecolors), cantFind)
#newTiles = copyImg(tmp)
#newTilecolors = []
#newTilecolors = getColors(newTiles)

#print("OLD:")
#pprint.pprint(Tilecolors)

#print("NEW:")
#pprint.pprint(newTilecolors)

#while(1):
#    draw("OLD TILES",enlarge(oldTiles, 5))
#    draw("NEW TILES",enlarge(newTiles, 5))

#CHECKING FIXCOLORS ON THE TILECOLORS
#cantFind = []
#tmp, cantFind = fixColors(tiles, cantFind)
#draw("Tiles after Fixed color", enlarge(tmp, 3))

#TileColorsAfterFix = []
#TileColorsAfterFix = getColors(tmp)
#print("TileColors Before\n")
#pprint.pprint(Tilecolors)
#print("TileColors After\n")
#pprint.pprint(TileColorsAfterFix)


#FOR TESTING INDIVIDUAL SPRITES IN THE DICTIONARY
i = 0
cantFind = []
while(i < 144):
    sprite = copyImg(dict[i])   
    sprite, cantFind = fixColors(sprite, cantFind)
    dict[i] = copyImg(sprite)
    i +=1

#after this dictionary is FINAL

Ftiles = createFinalTiles(dict)
print("MAP COLORS ARE:")
pprint.pprint(Mapcolors)
print("FINALTILES COLORS ARE:")
pprint.pprint(getColors(Ftiles))

FillMatrix(dict, newMap)
draw("FinalTiles",enlarge(Ftiles,1))

#Ading the tiles that appear only on the map to the Tiles image
Ftiles = createFinalTiles(dict)
cv2.imwrite("FinalTilesUpdated.png",Ftiles)
drawDict()