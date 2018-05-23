@ECHO OFF
@REM ###########################################
@REM # Script file to run the flow 
@REM # 
@REM ###########################################
@REM #
@REM # Command line for ngdbuild
@REM #
ngdbuild -p xc6slx45fgg676-2 -nt timestamp -bm battle_city.bmm "C:/Temp/ra41-2015/LPRS2-Zelda/battle_city_design/implementation/battle_city.ngc" -uc battle_city.ucf battle_city.ngd 

@REM #
@REM # Command line for map
@REM #
map -o battle_city_map.ncd -w -pr b -ol high -timing -detail battle_city.ngd battle_city.pcf 

@REM #
@REM # Command line for par
@REM #
par -w -ol high battle_city_map.ncd battle_city.ncd battle_city.pcf 

@REM #
@REM # Command line for post_par_trce
@REM #
trce -e 3 -xml battle_city.twx battle_city.ncd battle_city.pcf 

