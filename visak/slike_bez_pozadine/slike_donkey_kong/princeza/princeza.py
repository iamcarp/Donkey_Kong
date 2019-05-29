from PIL import Image


def convertToPNG():
    img = Image.open('./princeza_help1.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_help1.png", "PNG")#converted Image name
    print('Done')

convertToPNG()



def convertToPNG():
    img = Image.open('./princeza_help2.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_help2.png", "PNG")#converted Image name
    print('Done')

convertToPNG()



def convertToPNG():
    img = Image.open('./princeza_help3.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_help3.png", "PNG")#converted Image name
    print('Done')

convertToPNG()



def convertToPNG():
    img = Image.open('./princeza_help4.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_help4.png", "PNG")#converted Image name
    print('Done')

convertToPNG()



def convertToPNG():
    img = Image.open('./princeza_slomljeno_srce1.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_slomljeno_srce1.png", "PNG")#converted Image name
    print('Done')

convertToPNG()



def convertToPNG():
    img = Image.open('./princeza_slomljeno_srce2.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_slomljeno_srce2.png", "PNG")#converted Image name
    print('Done')

convertToPNG()



def convertToPNG():
    img = Image.open('./princeza_slomljeno_srce3.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_slomljeno_srce3.png", "PNG")#converted Image name
    print('Done')

convertToPNG()



def convertToPNG():
    img = Image.open('./princeza_slomljeno_srce4.png')#image path and name
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] == 0 and item[1] == 0 and item[2] == 0:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    img.save("./princeza_slomljeno_srce4.png", "PNG")#converted Image name
    print('Done')

convertToPNG()
