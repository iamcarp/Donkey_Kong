import numpy as np
import cv2
import pprint
import ImageProcessingFunctions as IP

sprite_height = 16
sprite_width = 16
Tilecolors = []
Mapcolors = []


def TestTiles():
    print("Printing Tile Image...")
    tiles = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")
    #tiles = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")
    print("Image Shape is:")
    pprint.pprint(tiles.shape)
    IP.draw('Tiles', IP.enlarge(tiles, 3))

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
    dict, Tilecolors = IP.defSprites(dict, "C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Overworld Tiles.png")

    print("TileColors length is: ", len(Tilecolors))
    print("TileColors are (BGR format): ")
    pprint.pprint( Tilecolors)
    IP.drawColors("Tilecolors", Tilecolors)

    print("Loading Map colors:")
    Mapcolors = IP.getColors(map)

    print("MapColors length is: ", len(Mapcolors))
    print("MapColors are (BGR format): ")
    pprint.pprint( Mapcolors)
    IP.drawColors("Mapcolors", Mapcolors)

    #print("Listing Tiles...")
    #print("Press ESC to skip")
    #IP.drawDict()


    #FOR CHECKING WHETHER IT CANT FIND A CIRTAIN COLOR
    #cantFind = []
    #for i in range(len(dict)):
    #    dict[i], cantFind = IP.fixColors(dict[i], cantFind, Tilecolors, Mapcolors)
    #print(cantFind)
    #drawDict()




    #TESTING ALL COLORS INSIDE TILES
    #print("Processing folowing Images:")
    #cantFind = []
    #oldTiles = IP.copyImg(IP.drawColors("OLD",Tilecolors))
    #tmp, cantFind = IP.fixColors(IP.drawColors("NEW", Tilecolors), cantFind, Tilecolors, Mapcolors)
    #newTiles = IP.copyImg(tmp)
    #newTilecolors = []
    #newTilecolors = IP.getColors(newTiles)

    #print("OLD:")
    #pprint.pprint(Tilecolors)

    #print("NEW:")
    #pprint.pprint(newTilecolors)

    #while(1):
    #    IP.draw("OLD TILES",enlarge(oldTiles, 5))
    #    IP.draw("NEW TILES",enlarge(newTiles, 5))

    #CHECKING FIXCOLORS ON THE TILECOLORS
    #cantFind = []
    #tmp, cantFind = IP.fixColors(tiles, cantFind, Tilecolors, Mapcolors)
    #IP.draw("Tiles after Fixed color", IP.enlarge(tmp, 3))

    #TileColorsAfterFix = []
    #TileColorsAfterFix = IP.getColors(tmp)
    #print("TileColors Before\n")
    #pprint.pprint(Tilecolors)
    #print("TileColors After\n")
    #pprint.pprint(TileColorsAfterFix)


    #FOR TESTING INDIVIDUAL SPRITES IN THE DICTIONARY
    i = 0
    cantFind = []
    while(i < 144):
        sprite = IP.copyImg(dict[i])
        sprite, cantFind = IP.fixColors(sprite, cantFind, Tilecolors, Mapcolors)
        dict[i] = IP.copyImg(sprite)
        i +=1

    #after this dictionary is FINAL

    Ftiles = IP.createFinalTiles(dict)
    print("MAP COLORS ARE:")
    pprint.pprint(Mapcolors)
    print("FINALTILES COLORS ARE:")
    FTcolors = IP.getColors(Ftiles)
    pprint.pprint(FTcolors)
    FTcolorsImg = IP.drawColors("Final Tiles Colors", FTcolors)

    #IP.FillMatrix(dict, newMap)
    print("PRINTING COLORS OF FINAL TILES IN FORM OF MATRIX:")


    Ftiles = cv2.imread("FinalTilesUpdated0.png")
    FTcolors = IP.getColors(Ftiles)
    matrix = IP.FillMatrixColor(FTcolors, Ftiles)
    IP.draw("FinalTiles",IP.enlarge(Ftiles,5))

    #Ading the tiles that appear only on the map to the Tiles image
    #Ftiles = IP.createFinalTiles(dict)
    #cv2.imwrite("FinalTilesUpdated.png",Ftiles)
    #IP.drawDict(dict)

    #REDEFINING THE MATRIX TO MATCH NEEDS:
    #each list is oen sprite
    Smatrix = []
    for i in range(162):
        Smatrix.append([])

    for i in range(len(matrix)):
        for j in range(len(matrix[0])):
            Smatrix[j//16 + 18*(i//16)].append(matrix[i][j])

    print("\nCORRECTED MATRIX\n")
    for i in range(len(Smatrix)):
        print(Smatrix[i])

    genSprite = np.zeros((16,16,3), np.uint8)
    for x in range(len(Smatrix)):
       for i in range(16):
            for j in range(16):
                genSprite[i][j] = FTcolors[Smatrix[x][i*16 + j]]
       IP.draw("genSprite",IP.enlarge(genSprite, 10))


def TestLinkSprites():
    link  = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\Export from Gimp\\NES - The Legend of Zelda - Link.png")
    IP.draw("Link", IP.enlarge(link, 1))
    ##REMOVING DESCRIPTIONS ON THE LEFT AND BOTTOM OF IMAGE, ALSO ONE EXTRA LINE OF PIXELS ON TOP
    x = 159
    y = 80
    clink = np.zeros((link.shape[0]-y,link.shape[1]-x,3), np.uint8)
    clink = link[1:link.shape[0] - y,x:link.shape[1]]

    IP.draw("croped Link", IP.enlarge(clink, 2))
    dict = {}
    Tilecolors = []
    dict, Tilecolors =  IP.defLinkSprites(dict,clink, Tilecolors)
    dict = IP.filterDict(dict)
    IP.drawDict(dict)
    pprint.pprint(Tilecolors)
    colorImg = IP.drawColors("Link colors", Tilecolors)
    #cv2.imwrite("OverworldCroped.png", newMap)

    #GetlinkColors -> Matrix and colorcodes extracted
    LinkTiles = IP.createFinalTiles(dict)
    IP.draw("LinkTiles", IP.enlarge(LinkTiles, 5))
    cv2.imwrite("LinkColors.png", IP.enlarge(colorImg, 5))
    cv2.imwrite("LinkTiles.png", IP.enlarge(LinkTiles, 5))

    matrix = IP.FillMatrixColor(Tilecolors, LinkTiles)

#fixing matrix to be as needed, each row is one sprite
    Smatrix = []
    for i in range(162):
        Smatrix.append([])

    for i in range(16*2):
        for j in range(len(matrix[0])):
            Smatrix[j//16 + 18*(i//16)].append(matrix[i][j])

    print("\nCORRECTED MATRIX\n")
    for i in range(len(Smatrix)):
        print(Smatrix[i])

    genSprite = np.zeros((16,16,3), np.uint8)
    for x in range(len(Smatrix)):
       for i in range(16):
            for j in range(16):
                genSprite[i][j] = Tilecolors[Smatrix[x][i*16 + j]]
       IP.draw("genSprite",IP.enlarge(genSprite, 10))

def TestItemSprites():
    items = cv2.imread("NES - The Legend of Zelda - TreasuresNEWER.png")
    items = items[0:items.shape[0]-208, 0:items.shape[1]]
    IP.draw("Items and Icons", IP.enlarge(items, 5))
    odict = {0:8, 1:8,  2:10, 3:16, 4:8}
    dict = {0:8, 1:8, 2:8, 3:16, 4:8, 5:10, 6:14, 7:8, 8:8, 9:8, 10:8, 11:8, 12:8, 13:9, 14:7, 15:8, 16:10, 17:6, 18:9, 19:7, 20:11, 21:6, 22:15, 23:16, 24:8,25:8, 26:8, 27:8, 28:16}

    spritesDict = {}

    p = countDict(dict, len(dict))
    sprite = items[0:16, 0+p:16+p]
    #IP.draw("sprite", IP.enlarge(sprite, 10))

    for i in range(len(dict)):
        spritesDict[i] = IP.fillImage(IP.copyImg(items[0:16, countDict(dict, i):dict[i]+countDict(dict, i)]), np.array([192, 192, 192]))


        #print(spritesDict[i].shape[0])
        #print(spritesDict[i].shape[1])

        #IP.draw("ime",IP.enlarge(spritesDict[i], 10))

    extraSprites = {}
    extraSprites[0] = spritesDict[7]
    extraSprites[1] = spritesDict[15]

    for i in range(len(extraSprites)):
        IP.draw("img", IP.enlarge(extraSprites[i], 10))



#removing sprites we don't have space for
    newdict = {}
    newdict[0] = IP.fillImage(spritesDict[0][:8,:], np.array([192, 192, 192]))
    newdict[1] = spritesDict[1]
    newdict[2] = spritesDict[2]


    Items = np.zeros((16, 16*3,3), np.uint8)


    for i in range(Items.shape[0]):
        for j in range(Items.shape[1]):
            Items[i,j]=newdict[(i//16)*3 + j//16][(i%16),j%16]
    IP.draw("Items", IP.enlarge(Items, 5))
    cv2.imwrite("Hearts.png", Items)



    ItemColors = []
    ItemColors = IP.getColors(Items)
    for i in range(len(ItemColors)):
        print(rgb2hex(ItemColors[i][0], ItemColors[i][1], ItemColors[i][2]))

    matrix = IP.FillMatrixColor(ItemColors, Items)


    Smatrix = []
    for i in range(3):
        Smatrix.append([])

    for i in range(16):
        for j in range(len(matrix[0])):
            Smatrix[j//16 + 16*(i//16)].append(matrix[i][j])

    print("\nCORRECTED MATRIX\n")
    for i in range(len(Smatrix)):
        print("{}, \n".format(Smatrix[i]))

    #genSprite = np.zeros((16,16,3), np.uint8)
    #for x in range(len(Smatrix)):
    #   for i in range(16):
    #        for j in range(16):
    #            genSprite[i][j] = ItemColors[Smatrix[x][i*16 + j]]
    #   IP.draw("genSprite",IP.enlarge(genSprite, 10))


    Pickups = np.zeros((16, 16*2, 3), np.uint8)

    for i in range(Pickups.shape[0]):
        for j in range(Pickups.shape[1]):
            Pickups[i,j]=extraSprites[(i//16)*3 + j//16][(i%16),j%16]
    IP.draw("Pickups", IP.enlarge(Pickups, 5))
    cv2.imwrite("Pickups.png", Pickups)

    print("\nPICKUPS\n")

    PickupColors = []
    PickupColors= IP.getColors(Pickups)
    for i in range(len(PickupColors)):
        print(rgb2hex(PickupColors[i][0], PickupColors[i][1], PickupColors[i][2]))

    matrix = IP.FillMatrixColor(PickupColors, Pickups)


    Smatrix = []
    for i in range(3):
        Smatrix.append([])

    for i in range(16):
        for j in range(len(matrix[0])):
            Smatrix[j//16 + 16*(i//16)].append(matrix[i][j])

    print("\nCORRECTED MATRIX\n")
    for i in range(len(Smatrix)):
        print("{}, \n".format(Smatrix[i]))



    return

def TestTextSprites():
    LetterImage = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\NES - The Legend of Zelda - Game Text.png");
    IP.draw("LetterImage", LetterImage);


    NotBold = 0
    if NotBold == 0:
        xx = 20
        x = 332
    else:
        x = 20
        xx = 332
    y = 20
    yy = 500-16
    CroppepedLetterImage = np.zeros((LetterImage.shape[0] - y- yy,LetterImage.shape[1]-x-xx,3), np.uint8)
    #print("Width is: ", CroppepedLetterImage.shape[1])
    #print("Hight is: ", CroppepedLetterImage.shape[0])


    CroppepedLetterImage= LetterImage[y:LetterImage.shape[0]-yy,x:LetterImage.shape[1]-xx]
    cv2.imwrite("CroppedLetterImage.png", IP.enlarge(CroppepedLetterImage, 5))

    IP.draw("CroppedLetterImage",CroppepedLetterImage)
    letters = {}
    LetterColors = []
    letters,LetterColors = IP.defSprites(letters, "CroppedLetterImage.png")
    IP.drawColors("LetterColors", LetterColors);
    pprint.pprint(LetterColors)
    #IP.drawDict(letters)

    sentence = [0,5,7]
    ExampleImage = np.zeros((16*16, len(sentence)//16+1,3), np.uint8)
    for x in range(len(sentence)):
        for i in range(16):
            for j in range(16):
                ExampleImage[x//16+j][x+i] = letters[sentence[x]][j*16 + i]
    IP.draw("ExampleImage", ExampleImage)

    matrix = IP.FillMatrixColor(LetterColors, CroppepedLetterImage)


#fixing matrix to be as needed, each row is one sprite
    Smatrix = []
    for i in range(162):
        Smatrix.append([])

    for i in range(16*3):
        for j in range(len(matrix[0])):
            Smatrix[j//16 + 16*(i//16)].append(matrix[i][j])

    print("\nCORRECTED MATRIX\n")
    for i in range(len(Smatrix)):
        print(Smatrix[i])

    genSprite = np.zeros((16,16,3), np.uint8)
    for x in range(len(Smatrix)):
       for i in range(16):
            for j in range(16):
                genSprite[i][j] = LetterColors[Smatrix[x][i*16 + j]]
       IP.draw("genSprite",IP.enlarge(genSprite, 10))

def countDict(dict, l):
    br = 0
    for i in range(l):
        br += dict[i]
    return br

def rgb2hex(r,g,b):
    hex = "0x{:02x}{:02x}{:02x}".format(r,g,b)
    return hex

def TestEnemieSprites():
    rawEnemies = cv2.imread("C:\\Users\\Milorad Markovic\\Downloads\\NES - Zelda 1 Textures\\zelda-sprites-enemies-noalpha.png")
    IP.draw("RawEnemies", IP.enlarge(rawEnemies, 3))
    dict = {}
    dict, EnemyColors = IP.defEnemieSprites(dict, rawEnemies)
    #IP.drawDict(dict)
    #IP.draw( "0",IP.enlarge(dict[0], 10))
    #IP.draw("10",IP.enlarge(dict[10], 10))
    #IP.draw("11",IP.enlarge(dict[11], 10))
    #IP.draw("14",IP.enlarge(dict[14], 10))
    #IP.draw("24",IP.enlarge(dict[24], 10))
    #IP.draw("66",IP.enlarge(dict[66], 10))
    #IP.draw("67",IP.enlarge(dict[67], 10))
    #IP.draw("80",IP.enlarge(dict[80], 10))
    #IP.draw("81",IP.enlarge(dict[81], 10))
    basic_sprites = [0, 10, 11, 14, 24, 66, 67, 80, 81]
    basic_dict = {}
    for i in range(len(basic_sprites)):
        basic_dict[i] = dict[basic_sprites[i]]
    IP.drawDict(basic_dict)

    EnemyColors = IP.getColors(rawEnemies)
    Tmatrix = []
    Matrix = [[],[],[],[],[],[],[],[],[]]
    pprint.pprint(EnemyColors)
    for i in range(len(basic_sprites)):
        Tmatrix = (IP.FillMatrixColor(EnemyColors, basic_dict[i]))
        for j in range(len(Tmatrix)):
            Matrix[i]+=(Tmatrix[j])
        #pprint.pprint(Tmatrix)

    print("\nCORRECTED MATRIX\n")
    print(Matrix)

    for i in range(len(EnemyColors)):
        print(rgb2hex(EnemyColors[i][2], EnemyColors[i][1], EnemyColors[i][0]))
    return

def TestNPCSprites():
    npcs = cv2.imread("mapa_sve.png")
    NPCSColors= []
    NPCSColors= IP.getColors(npcs)
    
    for k in range(9):
	    npcs = cv2.imread("mapa_sve.png")
	    npcs = npcs[ 0:16,k*16:(k+1)*16]
	    
	    
	    
	    for i in range(len(NPCSColors)):
		print(rgb2hex(NPCSColors[i][0], NPCSColors[i][1], NPCSColors[i][2]))

	    matrix = IP.FillMatrixColor(NPCSColors, npcs)


	    Smatrix = []
	    for i in range(3):
		Smatrix.append([])

	    for i in range(16):
		for j in range(len(matrix[0])):
		    Smatrix[j//16 + 16*(i//16)].append(matrix[i][j])

	    print("\nCORRECTED MATRIX\n")
	    
	    print("{}, \n".format(Smatrix[0]))




    return

#TestTiles()
#TestLinkSprites()
#TestItemSprites()
#TestEnemieSprites()
#TestTextSprites()
TestNPCSprites()

#TODO: MAKE A MATRIX FOR THE ORIGINAL MAP TILES, A MATRIX FOR THE COLORS, AND EXTRACT THE COLORS IN SOME WAY
#EXTRACT THE DUNGEON TILES
#EXTRACT COLORS AND MATRIX FOR LINK SPRITES


#import matplotlib.pyplot as mp
#import math

#l = []
#d = []
#for i in range(100):
#    d.append(i)
#    l.append(math.sin( 1/10*i));

#mp.bar(d, l, 0.1, 1, align = 'edge')
#mp.show()
