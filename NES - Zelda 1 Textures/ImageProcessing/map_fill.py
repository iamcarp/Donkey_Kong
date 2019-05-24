def map_fill():
    file = open("VHDL-map_fill.txt", "w")
    offset = 9983
    for i in range(1200):
        file_write = str(offset + i) + " => x\"00000016\", --padding\n"
        file.write(file_write)


    file.close()

map_fill()
