from PIL import Image

def convertToPNG():
    img = Image.open('./enemy_levo.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./enemy_levo.png", "PNG")#converted Image name
    print('Done')

convertToPNG()
