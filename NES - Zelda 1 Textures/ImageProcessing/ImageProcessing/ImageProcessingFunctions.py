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

def defEnemieSprites(dict, tiles):
    #dict - dictionary to which to write
    Tilecolors = []
    x = 0
    y = 0
    br = 0
    step = 14
    while(y <8): 
        while(x < 14): 
            dict[br] = np.zeros((sprite_height,sprite_width,3), np.uint8)
            for i in range(16):
                for j in range(16):
                    dict[br][i,j] = tiles[i+y*step+y*16,j+step*x+x*16]
                    rememberColor(dict[br][i,j], Tilecolors);
            br = br + 1
            x = x+1
        x = 0
        y = y+1
    return dict, Tilecolors


#Saves sprites from the input image into seperate images inside the dictionary - designed to work with the image given, may be changed for different images (LinkSprites)
def defLinkSprites(dict, img, Tilecolors):
#def defSprites(dict, path):
    #dict - dictionary to which to write
    tiles = img
    Tilecolors = []
    height = sprite_height 
    width = sprite_width 
    x = 0
    y = 0
    s = 0
    br = 0
    while(y <16): # 8 za map tilove
        while(x < 20): # 18 za map tilove
            dict[br] = np.zeros((height,width,3), np.uint8)
            if x <16:
                for i in range(height):
                    for j in range(width):
                        dict[br][i,j] = tiles[i+y*2+y*height,j+x*width+x*2]
                        rememberColor(dict[br][i,j], Tilecolors);
            else:
                if x < 18:
                    for i in range(height):
                        for j in range(8):
                            dict[br][i,j] = tiles[i+y*2+y*height,j+18*width + s*10]
                            rememberColor(dict[br][i,j], Tilecolors);
                    s+=1

                else:
                    for i in range(height):
                        for j in range(width):
                            dict[br][i,j] = tiles[i+y*2+y*height,j+x*width+x*2-6]
                            rememberColor(dict[br][i,j], Tilecolors);
            br = br + 1
            x = x+1
        x = 0
        y = y+1
        s = 0

    print(len(dict))
    return dict, Tilecolors

#Saves sprites from the input image into seperate images inside the dictionary - designed to work with the image given, may be changed for different images (ImageTiles)
def defSprites(dict, path):
    #dict - dictionary to which to write
    tiles = cv2.imread(path)
    draw("Image",tiles)
    Tilecolors = []
    x = 0
    y = 0
    br = 0
    while(y <3): # 8 za map tilove
        while(x < 16): # 18 za map tilove
            #print(br)
            dict[br] = np.zeros((sprite_height,sprite_width,3), np.uint8)
            for i in range(16):
                for j in range(16):
                    dict[br][i,j] = tiles[i+y*16,j+x*16]
                    rememberColor(dict[br][i,j], Tilecolors);
            br = br + 1
            x = x+1
        x = 0
        y = y+1
    return dict, Tilecolors

#filters unwanted values for the dictionary - designed to work with the dictionary given, may be changed for different dictionaries (LinkSprites Dict)
def filterDict(dict):
    ndict = {}
    br = 0
    for i in range(len(dict)):
        #takes just the first row, because all others can be remade from the colors, and uses sprite 136 because it's the impact one (on arrow inpact)
        if (i <20 or i==136):
            ndict[br] = dict[i]
            br +=1

    #flipping sprites for left and up walking
    ndict[br]= np.zeros((16, 16, 3), np.uint8)
    br+=1
    ndict[br]= np.zeros((16, 16, 3), np.uint8)
    br+=1
    ndict[br]= np.zeros((16, 16, 3), np.uint8)
    ndict[br+1]= np.zeros((16, 16, 3), np.uint8)
    ndict[br+2]= np.zeros((16, 16, 3), np.uint8)
    ndict[br+3]= np.zeros((16, 16, 3), np.uint8)
    
    #print(len(ndict))
    for i in range(16):
        for j in range(16):
            ndict[br-2][i,j] = ndict[2][i,15-j]
            ndict[br-1][i,j] = ndict[3][i,15-j]
            ndict[br][i,j] = ndict[4][i,15-j]
            ndict[br+1][i,j] = ndict[7][i,15-j]
            ndict[br+2][i,j] = ndict[8][i,15-j]
            ndict[br+3][i,j] = ndict[11][i,15-j]            
    #print("Drawing filtered dict")
    #drawDict(ndict)
    #print("Drawing done for filtered dictionary")
    return ndict

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
    if len(colors)%10 == 0:
        b = 0
    else:
        b = 1

    colorImage = np.zeros((16*(len(colors)//10 + b),16*10,3), np.uint8)
    x = 16*(len(colors)//10 + 1)
    dif = 0
    for i in range(x):
        for j in range(16*10):
            if (j//16 + 10*(i//16)) < len(colors):
                colorImage[i,j]=colors[j//16 + 10*(i//16)]
    draw(name, enlarge(colorImage, 5))
    return colorImage

#Lists out all the images inside the dictionary one after the other - is interruped by pressing ESC
def drawDict(dict):
    for i in range(len(dict)):
        print(i)
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

#fills in the ireguar image to have dimensions 16x16
def fillImage(img, fillcolor):
    tmp = np.zeros((16, 16,3), np.uint8)
    for i in range(tmp.shape[0]):
        for j in range(tmp.shape[1]):
            tmp[i,j] = fillcolor
    
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            tmp[i,j] = img[i,j]
    return tmp

#fixes colors from the original TilePallete to the one used in the map image
def fixColors(sprite, cantFind, Tilecolors, Mapcolors):
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
    height = 8+1#2#8 + 1 #for map tiles
    width = 18
    FTiles = np.zeros((height * 16,width * 16,3), np.uint8)
    for i in range(height*16):
        for j in range(width*16):
            if (i//16*18 + j//16) < len(dict):
                FTiles[i,j] = dict[i//16*18 + j//16][i%16, j%16]
    return  FTiles

#prints out the values of the matrix in a format which can easily be used to read in C++
def printMatrix(matrix, height):
    print("Printing Matrix")
    for i in range(height):
        print(matrix[i])
    return

#Checks each extracted sprite if it is in the dictionary, if not, adds it
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

#Checks each extracted pixel if it's value is in the dictionary, doesn't add them
def FillMatrixColor(colors, img):
    matrix = []
    for i in range(img.shape[0]):
        row = []
        for j in range(img.shape[1]):
            row.append([])
        matrix.append(row)

    #pprint.pprint(matrix)
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            for k in range(len(colors)):
               if  img[i,j][0] == colors[k][0] and img[i,j][1] == colors[k][1] and img[i,j][2] == colors[k][2]:
                    matrix[i][j] = k
        #print row
        print("{}, ".format(matrix[i]))
    #pprint.pprint(matrix)
    printMatrix(matrix, img.shape[0])
    return matrix

#Extracts sprite from image given in first parameter, placement of the sprite are in second and third
def extractSprite(map, startHight, startWidth):
        #EXTRACTING SPRITE FROM THE MAP AND CHECKING HIS VALUES
    tsprite = np.zeros((16,16,3), np.uint8)

    for i in range(16):
        for j in range(16):
            tsprite[i,j] = map[startHight*16 + i,startWidth*16 + j]

    #draw(str(startHight) + ", " + str(startWidth), enlarge(tsprite,10))

    return tsprite

