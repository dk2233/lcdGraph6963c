/*

program do obslugi ekranu graficznego na t6963c


wg160128


*/


#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma config RETEN = OFF  //
#pragma config SOSCSEL = DIG  //  
#pragma config XINST=OFF
#pragma config FOSC = HS2     // XT do 4MHZ
#pragma config PLLCFG = OFF 
#pragma config FCMEN = OFF 
#pragma config IESO = OFF  //  
#pragma config PWRTEN = ON  //
#pragma config BOREN = OFF
// Watchdog timer OFF
#pragma config WDTEN = OFF
// Single-Supply ICSP OFF
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CANMX = PORTB
#pragma config STVREN = ON  //    
#pragma config MSSPMSK = MSK7  //  

#define __MPLAB_ICD2__  1


#define  KEY_RED_PORT   PORTEbits.RE2
#define  KEY_BLACK_PORT   PORTEbits.RE1
#define  KEY_BLUE_PORT   PORTEbits.RE0
#define   PRESSED	0
#define   UNPRESSED	1



#define led_out     LATCbits.LATC3
#define led_pin     PORTCbits.RC3




#define t6963_freq   5000000
#define latch_lcd_data  LATD
#define lcd_data  PORTD
#define tris_lcd_data  TRISD
#define lcd_WR   PORTbits.RB0
#define lcd_RD   PORTbits.RB1
#define lcd_CE   PORTbits.RB2
#define lcd_CD   PORTbits.RB3
#define lcd_halt   PORTbits.RB4
#define lcd_reset   PORTbits.RB5

#define latch_lcd_WR   LATBbits.LATB0
#define latch_lcd_RD   LATBbits.LATB1
#define latch_lcd_CE   LATBbits.LATB2
#define latch_lcd_CD   LATBbits.LATB3
#define latch_lcd_halt   LATBbits.LATB4
#define latch_lcd_reset   LATBbits.LATB5

#define t6963_columns  20
#define t6963_rows  16
#define t6963_font_size  8
#define ilosc_pixels_X   160
#define ilosc_pixels_Y   128
#define  graphic_home_adress   0x0c00
#define  text_home_adress   0x0000
#define t6963_cursor_blink_on			0x01
#define t6963_cursor_display_on			0x02
#define t6963_text_on			0x04
#define t6963_graphic_on		0x08	  
#define jak_dlugo_klawisz_do_zmiany	30



 

void t6963_addres_pointer_set(unsigned char lower,unsigned char upper);
void  t6963_bajt_and_command(unsigned char , unsigned char );
void  t6963_word_set_command(unsigned char, unsigned char , unsigned char );
void t6963_status_check(unsigned char);
void t6963_write_byte(unsigned char, unsigned char);
void t6963_mode_set(unsigned char);
void t6963_hardreset(void);
void  t6963_auto_write(unsigned char, unsigned char);
void t6963_auto_write_stop(void);
void clear_sreen(unsigned char znak, unsigned int ile, unsigned int adres);
void  napisz_tekst(unsigned char pol_x, unsigned char pol_y, unsigned char *tab); 
void  t6963_initialization(void);
void t6963_simple_line(unsigned int pos_x1, unsigned int pos_y1, unsigned int pos_x2,unsigned int  pos_y2);
unsigned char t6963_read_byte(unsigned char CD, unsigned char bajt);
void   t6963_diplay_mode(unsigned char set_display);
void   ustaw_atrybut_tekstu(unsigned char pol_x, unsigned char pol_y, unsigned char *tab, unsigned char);

void show_menu(unsigned char, unsigned char);
void sprawdz_wykonywanie_opcji(unsigned char nr);
void opusc_opcje(unsigned char nr);
void zmiana_opcji(unsigned char, unsigned int *);


unsigned char znaczniki;
#define czy_minal_tmr0		1
#define czy_jestem_w_menu	2
#define czy_jestem_w_opcji	4
#define odswiez_menu		8
unsigned char znaczniki_wcisniecia;
#define   znacznik_key_red    4
#define   znacznik_key_black    2
#define   znacznik_key_blue    1
unsigned char nr_opcji=0;
// unsigned char tab[22] = ;

//mode set

#define t6963_OR	0
#define t6963_EXOR	1
#define t6963_AND	3
#define t6963_atribute	4

unsigned char tab_mode[4] = {t6963_OR,t6963_EXOR,3,4};
unsigned int  jak_dlugo_wcisniete = 0;

unsigned char tab_disp[4] = {0, 4, 8, 0xc};

#define pos_option_menu	15
#define atrybut_wybranej_opcji	8
#define atrybut_zmiany_var	5

typedef struct   linia_menu{

	unsigned char *napis;
	unsigned int	var;


} Menu;

struct point_xy {
	unsigned int p_x;
	unsigned int p_y;
};

struct point_xy *p_point, point_1;


#define Wymiar_Menu   8
Menu  MyMenu[] = {
	{(void *)"mode set", 3 },
	{(void *)"display mode", 4},
	{(void *)"bajt", 0},
	{(void *)"odczyt bajtu", 55},
	{(void *)"linia",0},
	{(void *)"point",0},
	{(void *)"timer",1},
	{(void *)"fill screen",0},
	{(void *)"exit", 0},
	{(void *)'\0', 0}
	};

#define mode_set 0
#define disp_mode 1
#define point_menu 5
#define timer_set 6
#define filling 7

// MyMenu[point_menu].var = (unsigned int)p_point;
// union u_m{
	// unsigned char ilosc_menu;
	// unsigned char nr;
	// Menu  MyMenu[];
	
// } opis_menu;

// opis_menu.MyMenu[]={
	// {(void *)"mode set", t6963_text_on},
	// {(void *)"display mode", 0},
	// {(void *)"odczyt bajtu", 0},
	// {(void *)"exit", 0},
	// {(void *)'\0', 0}
	// }; 




void deklaracje(void) {	
	INTCON = 0;
	INTCON2 = 0;
	INTCON3 = 0;
	WREG =0;
	ANCON0 = WREG;
	ANCON1 = WREG;
	ADCON0 = 0 ;
	ADCON1 = 0 ;	    
	TRISE = 0b00000111;		
	
	TRISD = 0x00;
	// TRISDbits.TRISD4 = 0; 
	TRISC = 0b11110111;
	TRISB = 0b11000000 ;
	TRISA = 0xff; 
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE = 0;
	LATA = 0;
	LATB = 0;
	LATC = 0;
	LATD = 0;
	LATE = 0;
	
	
    CTMUCONH = 0 ;
    CTMUCONL = 0 ;
	CCP1CON = 0;
    CCP2CON = 0;
    CCP3CON = 0;
    CCP4CON = 0;
    CCP5CON = 0;
	OSCCON =  0b01111100;
    OSCCON2 = 0b00011000;
	OSCTUNE = 0;
	PSPCON = 0;
	SSPCON1 = 0;
	SSPCON2 = 0;
	ODCON =0;
	PMD0 = 0b11111111;
    PMD1 = 0b11110000;
    PMD2 = 0b00001111;	
	CM1CON = 0;
	CM2CON = 0;
    RCSTA1 = 0;
    RCSTA2 = 0;
	SPBRGH2 = 0;
	SPBRGH1 = 0;
	SPBRG2 = 0;
	TXSTA1 = 0;
	TXSTA2 = 0;
	PIE1 = 0 ;
	PIE2 = 0 ;
	PIE3 = 0 ;
	PIE4 = 0 ;
	PIE5 = 0 ;
	CVRCON = 0;
	HLVDCON = 0;
	CANCON =  0b11100000;
	CANSTAT = 0b10000000;
	//0 prescaler 52,4 ms
	T1CON = 0b00000011;	
	T2CON = 0b00100100;	
	
	T3CON = 0b00110000;
	
}



void InterruptsEnable(void)
{
	INTCON1= 0 ;
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;
	INTCONbits.TMR0IE = 1; //enable Interrupt 0 (RB0 as interrupt)
	RCONbits.IPEN=0; 
	
	
	//prescaler    16
	//to daje 1 s na przerwanie
	// T0CON = 0b10000101;
	//prescaler 4 = 0,25 s
	T0CON = 0b10000001;
	PIE1bits.TMR1IE = 0;	

} 

void main(void){
	unsigned char dana, i, akt_znak=0xff ;
	unsigned int _ii, position=0 ;
	unsigned char tab2[10];
	
	p_point = &point_1;
	p_point-> p_x = 10;
	p_point-> p_y = 10;
	deklaracje();
	
	__delay_ms(10);
	
	
	// po inicjalizacji
	t6963_initialization();
	
	
	clear_sreen(0x00, t6963_columns * t6963_rows, 0x00);
	// clear_sreen(0x00, 200);
	// t6963_write_byte(1,0x96);
	
		
	//text area
	// t6963_word_set_command(t6963_columns, 0 ,0x41);
	// t6963_word_set_command(t6963_columns, 0 ,0x41);
	//mode set  "AND"
	
	__delay_ms(1000);
	clear_sreen('A' - 0x20, t6963_columns * t6963_rows, 0x00);
	// clear_sreen(0x21, 200);
	// t6963_write_byte(1,0x96);
	__delay_ms(1000);
	clear_sreen(0x00, t6963_columns * t6963_rows, 0x00 );
	// clear_sreen(0x00, 200);
	// t6963_write_byte(1,0x96);
	//address pointer set
	// 
	t6963_addres_pointer_set(5,0x00);
	t6963_bajt_and_command('D' - 0x20, 0xC4);
	
	t6963_diplay_mode(t6963_graphic_on);
	// t6963_write_byte(1,0x98);
	clear_sreen(0xaa, t6963_columns * t6963_rows*8, 0x0c00 );
	
	// t6963_bajt_and_command(0x26, 0xC0);
	// t6963_bajt_and_command(0x32, 0xC0);
	__delay_ms(1000);
	clear_sreen(0x00, t6963_columns * t6963_rows*8, 0x0c00 );
	__delay_ms(1000);
	MyMenu[disp_mode].var = t6963_text_on;
	t6963_diplay_mode(t6963_text_on);
	napisz_tekst(0,0,(unsigned char *)" to jest ekran demo ");
	
	
	napisz_tekst(0,3,(unsigned char *)"program stworzony  ");
	napisz_tekst(0,5,(unsigned char *)"przez  ");
	
	napisz_tekst(0,6,(unsigned char *)"Daniela Kucharskiego");
	
	napisz_tekst(0,15,(unsigned char *)"0123456789abcdefghi*&!");
	
	t6963_simple_line(0,8,20,8);
	__delay_ms(5000);
	// t6963_write_byte(1,0x94);
	
	
	// t6963_addres_pointer_set(0x1,0);
	// t6963_bajt_and_command(0x24, 0xC0);
	
	// t6963_addres_pointer_set(0x5,5);
	// t6963_bajt_and_command(0x28, 0xC0);
	clear_sreen(0x00, t6963_columns * t6963_rows, 0x00 );
	
	
	InterruptsEnable();
	
	znaczniki |= odswiez_menu;
	
	while(1){
	
	
		if (znaczniki & odswiez_menu){
		
			znaczniki &= ~odswiez_menu;
			
			show_menu(nr_opcji, (znaczniki & czy_jestem_w_menu));
		
		}
		
		
		if (znaczniki & czy_minal_tmr0) {
			// led_out = !led_out; 
			//po kolei zapalam bity PORTB
			// dana  = PORTD;
			
			// if (PORTDbits.RD7 == 0 )				
				// dana <<= 1;
			// else
				// dana = 1;
			
			// LATD = dana;
			
			znaczniki &= ~czy_minal_tmr0;
			if (!(znaczniki & czy_jestem_w_menu) && (MyMenu[filling].var > 0)){
				if  (position >= (t6963_rows*8*t6963_columns)) {
					position = 0;
					akt_znak = ~akt_znak;
				}
				_ii = graphic_home_adress+ position ;
				
				t6963_addres_pointer_set(_ii & 0xff,_ii>>8);
				t6963_bajt_and_command(akt_znak,0xc4);	
				// t6963_diplay_mode(t6963_graphic_on);
				position++;
				
				sprintf(tab2,"%4X\0",_ii);
				napisz_tekst(0,15,tab2);
			}
			else
				position = 0;
			
		};	
		
		if (jak_dlugo_wcisniete >0){
			// led_out = 1;
			sprintf(tab2,"%d\0", jak_dlugo_wcisniete);
			napisz_tekst(13,15,tab2);}
		// else if {
			// led_out = 0;
			// napisz_tekst(0,8,"          ");
			// };
		
		
		
		if ( (KEY_RED_PORT == UNPRESSED) && (PIE1bits.TMR1IE == 0) && (jak_dlugo_wcisniete>jak_dlugo_klawisz_do_zmiany) ) {
			
			
			if  (MyMenu[disp_mode].var == (t6963_graphic_on | t6963_text_on) )
				MyMenu[disp_mode].var = t6963_text_on;
			else if (MyMenu[disp_mode].var == (t6963_graphic_on) )
				MyMenu[disp_mode].var = (t6963_graphic_on | t6963_text_on);
			else if (MyMenu[disp_mode].var == ( t6963_text_on) )	
				MyMenu[disp_mode].var = t6963_graphic_on;
			
			t6963_diplay_mode(MyMenu[disp_mode].var);
			jak_dlugo_wcisniete=0;
			
			
		}
		else if (KEY_RED_PORT == PRESSED && PIE1bits.TMR1IE == 0 ) {
		
			//klawisz czerwony
						
			WRITETIMER1(0);
			PIE1bits.TMR1IE = 1;
			jak_dlugo_wcisniete=0;
			
			
			
			t6963_addres_pointer_set(0x00,0x0c);
			t6963_bajt_and_command(0xff,0xc4);
			//xor z zawartoscia markers1  - do przelaczenia stanu
			//wszystkie pozostale bity nie ulegaja zmianie
			// xor robie z 1   czyli 00000001
			//jesli bit0 jest 0 wyjdzie 1
			//jesli bit0 jest 1 wyjdzie 0  
			znaczniki_wcisniecia = 0;
			znaczniki_wcisniecia |= znacznik_key_red;
			
			if (znaczniki & czy_jestem_w_opcji){
				// znaczniki |= odswiez_menu;
				zmiana_opcji(nr_opcji, &MyMenu[nr_opcji].var);
				sprawdz_wykonywanie_opcji(nr_opcji);
			}
			else if ( (znaczniki & czy_jestem_w_menu) && !(znaczniki & czy_jestem_w_opcji) ){
				znaczniki |= odswiez_menu;
				if ( nr_opcji < (Wymiar_Menu-1))
					nr_opcji++;
				else
					nr_opcji = 0;
			}
			
			
		};
		if (KEY_BLACK_PORT == PRESSED && PIE1bits.TMR1IE == 0 ) {
		
			
			WRITETIMER1(0);
			PIE1bits.TMR1IE = 1;
			znaczniki_wcisniecia = 0;
			znaczniki_wcisniecia |= znacznik_key_black;
			// t6963_addres_pointer_set(graphic_home_adress & 0xff, graphic_home_adress>>8);
			// dana = t6963_read_byte(0,0xc5);
			// utoa(tab2,dana,16);
			// sprintf(tab2,"%4.2X\0",dana);
			
			// napisz_tekst(0,9,tab2);
			
			if (znaczniki & czy_jestem_w_opcji){
				//odznaczam
				if (nr_opcji != point_menu){
					znaczniki &= ~czy_jestem_w_opcji;
					opusc_opcje(nr_opcji);
					znaczniki |= odswiez_menu;
				}
				else if (nr_opcji == point_menu){
					
					if (MyMenu[point_menu].var == 1){
						MyMenu[point_menu].var ^=1;
						znaczniki &= ~czy_jestem_w_opcji;
						opusc_opcje(nr_opcji);
						znaczniki |= odswiez_menu;}
					else{
						MyMenu[point_menu].var ^=1;
						sprawdz_wykonywanie_opcji(nr_opcji);
						}
						
					//w ten sposób stale zmieniam z 0 na 1
				
				}
			}
			else if  ((znaczniki & czy_jestem_w_menu) && !(znaczniki & czy_jestem_w_opcji) ) {
				//zaznaczam
				znaczniki |= czy_jestem_w_opcji;
				
				sprawdz_wykonywanie_opcji(nr_opcji);
				// znaczniki |= odswiez_menu;
			}	
			else{
				//jesli wchodze do menu
				znaczniki |= czy_jestem_w_menu;
				clear_sreen(0x00, t6963_columns * t6963_rows*8, graphic_home_adress );
				t6963_mode_set(t6963_atribute);
				MyMenu[disp_mode].var = (t6963_graphic_on | t6963_text_on);
				t6963_diplay_mode(MyMenu[disp_mode].var );
				znaczniki |= odswiez_menu;}
				
			
	
			
		};
		
		if (KEY_BLUE_PORT == PRESSED && PIE1bits.TMR1IE == 0 ) {
		
			
			WRITETIMER1(0);
			PIE1bits.TMR1IE = 1;
			znaczniki_wcisniecia = 0;
			znaczniki_wcisniecia |= znacznik_key_blue;
			
			
			if (znaczniki & czy_jestem_w_opcji){
				// znaczniki |= odswiez_menu;
				zmiana_opcji(nr_opcji, &MyMenu[nr_opcji].var);
				sprawdz_wykonywanie_opcji(nr_opcji);
			}
			else if ( (znaczniki & czy_jestem_w_menu) && !(znaczniki & czy_jestem_w_opcji) ){
				znaczniki |= odswiez_menu;
				if ( nr_opcji != 0)
					nr_opcji--;
				else
					nr_opcji = (Wymiar_Menu-1);
				}
				
				
			
		};
		
	};
	


}

void interrupt high(void){
	// unsigned char 
	if(INTCONbits.TMR0IF && INTCONbits.TMR0IE) {    // Timer0 overflow interrupt
		INTCONbits.TMR0IF = 0;    // ACK
		
		znaczniki ^= czy_minal_tmr0;
		
	};

	
	
	if (PIE1bits.TMR1IE && PIR1bits.TMR1IF ) {
		PIR1bits.TMR1IF = 0 ;
		if ( (KEY_RED_PORT == UNPRESSED ) &&  (znaczniki_wcisniecia & znacznik_key_red) )  {
			//wylaczam przerwanie jesli juz puscilem przycisk
			PIE1bits.TMR1IE = 0;
			znaczniki_wcisniecia = 0;
			// jak_dlugo_wcisniete=0;
		}
		else if ((KEY_BLACK_PORT == UNPRESSED) &&  (znaczniki_wcisniecia & znacznik_key_black) )  {
			//wylaczam przerwanie jesli juz puscilem przycisk
			PIE1bits.TMR1IE = 0;
			znaczniki_wcisniecia = 0;
			led_out = 0;
			// jak_dlugo_wcisniete=0;
		
			
		}
		else if ((KEY_BLUE_PORT == UNPRESSED) && (znaczniki_wcisniecia & znacznik_key_blue) ){
			PIE1bits.TMR1IE = 0;
			znaczniki_wcisniecia = 0;
			// jak_dlugo_wcisniete=0;
		};
		
		if ( (KEY_RED_PORT == PRESSED ) &&  (znaczniki_wcisniecia & znacznik_key_red)){
			jak_dlugo_wcisniete +=1;
		}
		else if ( (KEY_BLACK_PORT == PRESSED ) &&  (znaczniki_wcisniecia & znacznik_key_black)){
			led_out = 1;
		};;
		
	};
}


void t6963_hardreset(void){
	tris_lcd_data = 0xff;
	latch_lcd_data = 0;
	latch_lcd_halt = 1;
	latch_lcd_reset = 0;
	latch_lcd_CD = 1;
	latch_lcd_CE = 1;
	latch_lcd_WR = 1;
	latch_lcd_RD = 1;
	
	__delay_us(1);
	
	//zmieniam w high impedance
	TRISBbits.TRISB5 = 1;
	latch_lcd_reset = 1;
	// tris_lcd_data = 0;
	latch_lcd_data = 0;
	latch_lcd_halt = 1;
	latch_lcd_CD = 1;
	latch_lcd_CE = 1;
	latch_lcd_WR = 1;
	latch_lcd_RD = 1;
}





void t6963_status_check(unsigned char markers){
	unsigned char dana, i = 1;
	// led_out = 1;
	
while(i){
	// tris_lcd_data = 0x00;
	tris_lcd_data = 0xff;
	// latch_lcd_data = 0xff;
	latch_lcd_CD = 1;	
	latch_lcd_WR = 1;
	latch_lcd_RD = 0;
	
	latch_lcd_CE = 0;
	Nop();
	Nop();
	
	
	latch_lcd_CE = 1;
	// __delay_us(1);
	// Nop();
	
		
	dana =	lcd_data;
	 
	if ( (markers == 0) && ( (dana & 0x01) && (dana & 0x02)) ){
	
		i = 0;
	//dla STA = 3 	
	}
	else if ( (markers == 1) && (dana & 0x08)  ){
		i=0;}
	
};	
	// __delay_ms(200);
	// led_out = 0;
	latch_lcd_CD = 0;
	latch_lcd_CE = 1;
	latch_lcd_WR = 1;
	latch_lcd_RD = 1;

}





void  t6963_word_set_command(unsigned char bajtL, unsigned char bajtH, unsigned char Command){
	//mlodszy bajt
	t6963_status_check(0);
	t6963_write_byte(0,bajtL);
	//starszy bajt
	t6963_status_check(0);
	t6963_write_byte(0,bajtH);
	
	
	//comannd
	t6963_status_check(0);
	t6963_write_byte(1,Command);
}



void  t6963_bajt_and_command(unsigned char bajtL, unsigned char Command){
	//mlodszy bajt
	t6963_status_check(0);
	
	t6963_write_byte(0,bajtL);
	
	//comannd
	t6963_status_check(0);
	t6963_write_byte(1,Command);
	
}


void t6963_write_byte(unsigned char CD, unsigned char bajt){
	//jesli CD = 1 to komenda
	tris_lcd_data = 0;	
	latch_lcd_data = bajt;
	latch_lcd_CD = CD;
	latch_lcd_WR = 0;
	latch_lcd_RD = 1;
	
	
	latch_lcd_CE = 0;	
	// __delay_us(1);
	Nop();
	Nop();
	latch_lcd_CE = 1;
	// __delay_us(1);
	latch_lcd_CD = 0;
	latch_lcd_WR = 1;

}

unsigned char t6963_read_byte(unsigned char CD, unsigned char bajt){
	unsigned char dana;
	//jesli CD = 1 to komenda
	tris_lcd_data = 0xff;	
	latch_lcd_data = bajt;
	latch_lcd_CD = CD;
	latch_lcd_WR = 1;
	latch_lcd_RD = 0;
	
	
	latch_lcd_CE = 0;	
	// __delay_us(1);
	Nop();
	Nop();
	latch_lcd_CE = 1;
	dana = lcd_data;
	// __delay_us(1);
	latch_lcd_CD = 0;
	latch_lcd_RD = 0;
	return dana;

}

void t6963_addres_pointer_set(unsigned char lower,unsigned char upper){
	t6963_word_set_command(lower, upper ,0x24);


}

void  t6963_auto_write(unsigned char lower, unsigned char upper){
	t6963_addres_pointer_set(lower,upper);
	t6963_status_check(0);
	t6963_write_byte(1,0xb0);
	//sprawdzam sta0 i sta1
	// t6963_status_check(0);
	
}



void  t6963_auto_write_stop(void){
	//sta 3
	t6963_status_check(1);
	t6963_write_byte(1,0xb2);
}
// void   address_pointer_set(unsigned char bajtL, unsigned char bajtH, unsigned char adres){


void clear_sreen(unsigned char znak, unsigned int ile, unsigned int adres){
	unsigned int _ii;
	t6963_auto_write(adres & 0xff, adres>> 8);
	// for (i= 0; i<(t6963_columns*t6963_rows); i++){
	for (_ii= 0; _ii< ile; _ii++){
		//sprawdzam STA3
		t6963_status_check(1);
		t6963_write_byte(0,znak);
	
	};
	
	t6963_auto_write_stop();



}


void  napisz_tekst(unsigned char pol_x, unsigned char pol_y, unsigned char *tab){
	unsigned int adres,_ii;
	// unsigned char adresL, adresH;
	adres = text_home_adress + pol_x + pol_y*t6963_columns;
	
	t6963_auto_write(adres & 0xff,adres>>8);
		
	for(_ii = 0; tab[_ii] ; _ii++){
		//sprawdzam STA3
		t6963_status_check(1);
		t6963_write_byte(0,tab[_ii]-0x20);
		
	
	};

	t6963_auto_write_stop();

}

void  ustaw_atrybut_tekstu(unsigned char pol_x, unsigned char pol_y, unsigned char *tab, unsigned char atrybut){
	unsigned int adres,_ii;
	// unsigned char adresL, adresH;
	adres = graphic_home_adress + pol_x + pol_y*t6963_columns;
	
	t6963_auto_write(adres & 0xff,adres>>8);
		
	for(_ii = 0; tab[_ii] ; _ii++){
		//sprawdzam STA3
		t6963_status_check(1);
		t6963_write_byte(0,atrybut);
		
	
	};

	t6963_auto_write_stop();

}


void  t6963_initialization(void){
	t6963_hardreset();
	
	
	
	//control word set Graphic home address
	t6963_word_set_command(graphic_home_adress & 0xff, (graphic_home_adress >> 8) ,0x42);
	//graphic area set command
	t6963_word_set_command(t6963_columns, 0 ,0x43);
	//text home
	t6963_word_set_command(text_home_adress & 0xff, (text_home_adress >> 8) ,0x40);
	//Text area
	t6963_word_set_command(t6963_columns, 0 ,0x41);
	//set offset 0
	t6963_word_set_command(0x3,0,0x22);
	t6963_mode_set(MyMenu[mode_set].var);
	
	
	//set cursor pointer
	// t6963_word_set_command(5,5,0x21);
	//cursor pattern select
	t6963_status_check(0);
	t6963_write_byte(1,0xa1);
	
	//display off	
	t6963_status_check(0);
	t6963_write_byte(1,0x90);
	__delay_ms(100);
	//text on, graph off
	t6963_diplay_mode(t6963_text_on);



}

void t6963_simple_line(unsigned int pos_x1, unsigned int pos_y1, unsigned int pos_x2,unsigned int  pos_y2){
	unsigned int adres1,adres2, pos_x,pos_y, adres,_ii;
	adres1 = pos_x1 + pos_y1*t6963_rows;
	adres2 = pos_x2 + pos_y2*t6963_rows;
	
	
	// t6963_addres_pointer_set(adres1 & 0xff,adres1 >> 8);
	
	for(_ii = 0; _ii <10; _ii++){
		adres = graphic_home_adress +  pos_x+_ii + pos_y*t6963_columns;
		t6963_addres_pointer_set(adres & 0xff,adres >> 8);
		t6963_bajt_and_command(0xff,0xc4);
	
	};


}

void   t6963_graphics_set_point(unsigned int pos_x, unsigned int pos_y){
	unsigned int  address;
	unsigned char wartosc;
	wartosc = 1 >> (pos_x % t6963_font_size);
	address = graphic_home_adress + pos_y* (ilosc_pixels_X/8)+ pos_x*pos_x/t6963_font_size; 
	
	

	t6963_addres_pointer_set(address &0xff, address >> 8 );
	t6963_bajt_and_command(wartosc, address);
}


void   t6963_diplay_mode(unsigned char set_display){
	t6963_status_check(0);
	t6963_write_byte(1,(0x90 | set_display) );


}


void  t6963_mode_set(unsigned char set_mode){

	t6963_status_check(0);
	t6963_write_byte(1,(0x80 | set_mode) );

}

void  show_menu(unsigned char nr_zaznaczony, unsigned char stan){
	unsigned char i, pos_start ;
	
	if (stan != 0){
	clear_sreen(0x00, t6963_columns * t6963_rows*8, graphic_home_adress );
	};
		// t6963_mode_set(t6963_atribute);
		// MyMenu[disp_mode].var = (t6963_graphic_on | t6963_text_on);
		// t6963_diplay_mode(MyMenu[disp_mode].var );
		
	
	// }
	// else	
	// {
		
		
	// };
	

	for(pos_start=0; MyMenu[pos_start].napis ; pos_start++){
		
		napisz_tekst(0,pos_start, MyMenu[pos_start].napis);
		if ( (nr_zaznaczony == pos_start) && (stan !=0) ){
			ustaw_atrybut_tekstu(0,pos_start, MyMenu[pos_start].napis, atrybut_wybranej_opcji);
		
		}
	
	};


};



void sprawdz_wykonywanie_opcji(unsigned char nr){
	unsigned char string1[10];

	if (nr == (Wymiar_Menu-1)){
		//opusc menu
		znaczniki &= ~czy_jestem_w_menu;
		znaczniki &= ~czy_jestem_w_opcji;
		
		znaczniki |= odswiez_menu;
		t6963_mode_set(MyMenu[mode_set].var);
		t6963_diplay_mode(MyMenu[disp_mode].var);
		
	
	
	}
	else if (nr == mode_set){
		sprintf(string1, "%2d\0",MyMenu[nr].var);
		napisz_tekst(pos_option_menu, nr, string1);
		ustaw_atrybut_tekstu(pos_option_menu,nr, string1, atrybut_zmiany_var);
	
	
	}
	else if (nr == point_menu){
		
		sprintf(string1, "%3d\0",p_point->p_x);
		napisz_tekst(pos_option_menu-2, nr, string1);
		
		if (MyMenu[nr].var == 0 )
			ustaw_atrybut_tekstu(pos_option_menu-2,nr, string1, atrybut_zmiany_var);
		else	
			ustaw_atrybut_tekstu(pos_option_menu-2,nr, string1, 0);
			
			
		sprintf(string1, "%3d\0",p_point->p_y);	
		napisz_tekst(pos_option_menu+2, nr, string1);	
		if (MyMenu[nr].var == 1 )
			ustaw_atrybut_tekstu(pos_option_menu+2,nr, string1, atrybut_zmiany_var);
		else
			ustaw_atrybut_tekstu(pos_option_menu+2,nr, string1, 0);
	
	}
	else {
		sprintf(string1, "%2d\0",MyMenu[nr].var);
		napisz_tekst(pos_option_menu, nr, string1);
		ustaw_atrybut_tekstu(pos_option_menu,nr, string1, atrybut_zmiany_var);
	//koncze migac i wyswietlam napis zmiennej
	
	}



}



void opusc_opcje(unsigned char nr){
	unsigned char puste[5]="    ";
	
	napisz_tekst(pos_option_menu, nr,puste );
	ustaw_atrybut_tekstu(pos_option_menu,nr, puste, 0);

}


void   zmiana_opcji(unsigned char nr,unsigned int *p){
	unsigned char i ;
	unsigned char string1[15] , pp, len;
	if (nr == mode_set){
		//0 OR, 1 - EXOR, 11 - AND,
		// i = 	MyMenu[mode_set].var;
		// *pp = strchr(tab_mode, 3);
		len = sizeof(tab_mode);
		for( i=0; i<len; i++){
			if (tab_mode[i] == *p){ 
				pp = i;
				break;
			}	
		};
		
		if (znaczniki_wcisniecia & znacznik_key_blue){
			if (i== 3)
				i = 0;
			else
				i++;
		}
		else if (znaczniki_wcisniecia & znacznik_key_red){
			if (i== 0)
				i = 3;
			else
				i--;
		
		
		}
		
		MyMenu[nr].var = tab_mode[i];
		
		
		// sprintf(string1,"%d %d %X\0",len,i,znaczniki_wcisniecia);
		// napisz_tekst(0,15,string1);
		
		
		

	}
	else if (nr == disp_mode){
		//0 OR, 1 - EXOR, 11 - AND,
		// i = 	MyMenu[mode_set].var;
		// *pp = strchr(tab_mode, 3);
		len = sizeof(tab_disp);
		for( i=0; i<len; i++){
			if (tab_disp[i] == *p){ 
				pp = i;
				break;
			}	
		};
		
		if (znaczniki_wcisniecia & znacznik_key_blue){
			if (i== (len-1))
				i = 0;
			else
				i++;
		}
		else if (znaczniki_wcisniecia & znacznik_key_red){
			if (i== 0)
				i = (len-1);
			else
				i--;
		
		
		}
		
		MyMenu[nr].var = tab_disp[i];
		
		
		// sprintf(string1,"%d %d %X\0",len,i,znaczniki_wcisniecia);
		// napisz_tekst(0,15,string1);
		
		
		

	}
	else if (nr == timer_set){
		if (znaczniki_wcisniecia & znacznik_key_blue){
			if (*p== 7)
				*p = 0;
			else
				(*p)++;
		}
		else if (znaczniki_wcisniecia & znacznik_key_red){
			if (*p== 0)
				*p = 7;
			else
				(*p)--;
		};
		
		
		//ustaw timer0
		
		i = T0CON & 0b11111000;
		i |= (*p);
		
		T0CON = i;
	
	}
	else{
		// MyMenu[nr].var +=1;
		if (znaczniki_wcisniecia & znacznik_key_blue){
			if (*p== 99)
				*p = 0;
			else
				(*p)++;
		}
		else if (znaczniki_wcisniecia & znacznik_key_red){
			if (*p== 0)
				*p = 99;
			else
				(*p)--;
		};
		//
	// sprintf(string1,"%d %d %X\0",len,*p,znaczniki_wcisniecia);
	// napisz_tekst(0,15,string1);	
	}
	sprintf(string1,"%2d\0",MyMenu[nr].var);
	napisz_tekst(pos_option_menu,nr,string1);
	
}



