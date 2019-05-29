from PIL import Image

def convertToPNG():
    img = Image.open('./mario_trci_desno1.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./mario_trci_desno1.png", "PNG")#converted Image name
    print('Done')

convertToPNG()
