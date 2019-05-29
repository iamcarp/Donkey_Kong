from PIL import Image

def convertToPNG():
    img = Image.open('./bure.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure.png", "PNG")#converted Image name
    print('Done')

convertToPNG()


def convertToPNG():
    img = Image.open('./bure1.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure1.png", "PNG")#converted Image name
    print('Done')

convertToPNG()


def convertToPNG():
    img = Image.open('./bure2.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure2.png", "PNG")#converted Image name
    print('Done')

convertToPNG()


def convertToPNG():
    img = Image.open('./bure3.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure3.png", "PNG")#converted Image name
    print('Done')

convertToPNG()

def convertToPNG():
    img = Image.open('./bure4.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure4.png", "PNG")#converted Image name
    print('Done')

convertToPNG()

def convertToPNG():
    img = Image.open('./bure_start1.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure_start1.png", "PNG")#converted Image name
    print('Done')

convertToPNG()

def convertToPNG():
    img = Image.open('./bure_start2.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure_start2.png", "PNG")#converted Image name
    print('Done')

convertToPNG()

def convertToPNG():
    img = Image.open('./bure_start3.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure_start3.png", "PNG")#converted Image name
    print('Done')

convertToPNG()

def convertToPNG():
    img = Image.open('./bure_start4.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./bure_start4.png", "PNG")#converted Image name
    print('Done')

convertToPNG()
