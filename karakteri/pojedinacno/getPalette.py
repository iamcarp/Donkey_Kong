palette = ['c0c0c0','f83800','0000bc','6888ff','ffffff','ffa044','b8f818','503000','ac7c00','e45c10','f8b800','b8b8f8','000000','008888','ffe0a8','58f898','005800','00a844','4c5864','7c7c7c','004058','00e8d8']

if __name__ == "__main__":
    pf = open("VHDL_palette.txt", "w")
    offset = 35
    for c in palette:
        pf.write("\t\t" + str(offset) + " => x\"00" + c[4::] + c[2:4] + c[0:2] + "\",\n")
        offset += 1

    pf.close()
