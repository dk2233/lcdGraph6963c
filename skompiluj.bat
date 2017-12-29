REM kompilacja ze zrodel w c
xc8  --chip=18F46K80 --opt=all -I..\libs -g -DTris_ds1820_bits=TRISDbits.TRISD3 -DLatch_ds1820=LATDbits.LATD3   -D_XTAL_FREQ=20000000 lcd_graphic3.c  
