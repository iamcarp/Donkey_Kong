from PIL import Image

def convertToPNG():
    img = Image.open('./image.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./new_image.png", "PNG")#converted Image name
    print('Done')

convertToPNG()
