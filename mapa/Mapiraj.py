def mapiraj_sprajtove():
	x=0
	y=0
	d = {
		'1' : '0x00FF,',
		'2' : '0x013F,',
		'3' : '0x017F,',
		'4' : '0x01BF,',
		'5' : '0x01FF,',
		'6' : '0x023F,',
		'7' : '0x027F,',
		'8' : '0x02BF,',
		'9' : '0x02FF,'
	}
	mapa = open("indeksi_mape.txt", "r")
	ispis = open("sprajtovi_mape.txt", "w")
	for i in range(300):
		broj = mapa.read(1)
		if(broj == "\n"):
			continue
		ispis.write(d[broj])
		temp = " // x = " + str(x) + ", y = " + str(y)
		x = x+1
		if(x == 20):
			y= y+1
			x = 0
		ispis.write(temp)
		ispis.write("\n")
		
	
	mapa.close()
	ispis.close()

mapiraj_sprajtove()
