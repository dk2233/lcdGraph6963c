/*

program do obslugi ekranu graficznego na t6963c


wg160128


*/


#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

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
#define jak_dlugo_klawisz_do_zmiany	15


struct	arguments_for_functions{
	
	unsigned char no_of_menu_option;
	unsigned char no_of_marked_value_of_option;
	unsigned char var3;
	unsigned char var4;
	
	
} str_arg_func, *p_arg;

 
 
 
void  t6963_initialization(void);
void t6963_addres_pointer_set(unsigned char lower,unsigned char upper);
void  t6963_bajt_and_command(unsigned char , unsigned char );
void  t6963_word_set_command(unsigned char, unsigned char , unsigned char );
void t6963_status_check(unsigned char);
void t6963_write_byte(unsigned char, unsigned char);
void t6963_mode_set(unsigned char);
void t6963_hardreset(void);
void  t6963_auto_write(unsigned char, unsigned char);
void t6963_auto_write_stop(void);
void t6963_simple_line(unsigned int pos_x1, unsigned int pos_y1, unsigned int pos_x2,unsigned int  pos_y2);
unsigned char t6963_read_byte( unsigned char bajt);
void   t6963_diplay_mode(unsigned char set_display);
void   t6963_ustaw_atrybut_tekstu(unsigned char pol_x, unsigned char pol_y, unsigned char *tab, unsigned char);
void   t6963_graphics_set_point(unsigned int pos_x, unsigned int pos_y, unsigned char color);


void clear_screen(unsigned char znak, unsigned int ile, unsigned int adres);
void  napisz_tekst(unsigned char pol_x, unsigned char pol_y, unsigned char *tab); 




// void   ustaw_atrybut_tekstu(unsigned char );

void show_menu(unsigned char, unsigned char);
void opusc_opcje(unsigned char nr);
void zmiana_opcji(unsigned char, unsigned int *);





void   menu_write_option_value(struct arguments_for_functions *arg);
// void   menu_write_option_value(unsigned char nr_line, unsigned char which_marked);
void   Menu_Exit(struct arguments_for_functions *arg);
void  Menu_Show_GFX(struct arguments_for_functions *arg);
void  Menu_sin(struct arguments_for_functions *arg);
void  Clear_All_GFX(struct arguments_for_functions *arg);

unsigned char znaczniki;
#define czy_minal_tmr0		1
#define czy_jestem_w_menu	2
#define czy_jestem_w_opcji	4
#define odswiez_menu		8
unsigned char znaczniki_wcisniecia;
#define   znacznik_key_red    4
#define   znacznik_key_black    2
#define   znacznik_key_blue    1
// unsigned char nr_opcji=0;
// unsigned char tab[22] = ;

//mode set

#define t6963_OR	0
#define t6963_EXOR	1
#define t6963_AND	3
#define t6963_atribute	4

unsigned char tab_mode[4] = {t6963_OR,t6963_EXOR,3,4};
unsigned int  jak_dlugo_wcisniete = 0;

unsigned char tab_disp[4] = {0, 4, 8, 0xc};

#define pos_option_menu	14
#define atrybut_wybranej_opcji	8
#define atrybut_zmiany_var	5

typedef struct   linia_menu{

	unsigned char *napis;
	unsigned int	how_many_options;
	unsigned int  variables_table[4];
	//pointer to a void function - co ma byc wykonywane
	void	 (*function)(struct arguments_for_functions *arg);   


} Menu_t;

struct point_xy {
	unsigned int p_x;
	unsigned int p_y;
};

struct point_xy point_1, *p_point = &point_1;


// void *func_ptr(unsigned char) =  

#define Wymiar_Menu   12
//inna postac struktury
Menu_t  MyMenu[] = {
	{(unsigned char *)"mode set", 1 , {3,0},  &menu_write_option_value },
	{(void *)"display mode", 1, {0,0},  &menu_write_option_value},
	{(void *)"bajt", 1, {4,5 },   &menu_write_option_value },
	{(void *)"odczyt bajtu", 2, {0,1}  , &menu_write_option_value},
	{(unsigned char *)"linia",4,{0} , &menu_write_option_value},
	{(void *)"point",3, {2,0,1}, &menu_write_option_value },
	{(void *)"timer",1, {3}, &menu_write_option_value },
	// {(void *)"timer",1, {3}},
	{(void *)"fill screen",1,{3}, &menu_write_option_value },
	{(void *)"sin",0,{60}, &Menu_sin },
	{(void *)"grafika",0,{0}, &Menu_Show_GFX  },
	{(unsigned char *)"clear gfx",0,{0}, &Clear_All_GFX },
	{(unsigned char *)"exit", 1,{0},&Menu_Exit },
	{(void *)'\0', 0, {  } }
	};

#define mode_set 0
#define disp_mode 1
#define  line_menu 4
#define point_menu 5
#define timer_set 6
#define filling 7
#define show_graph 9
#define  show_sin 8

// MyMenu[point_menu].how_many_options = (unsigned int)p_point;
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

// time_t	czas;


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
	// unsigned char nr_marked_value = 0;
	
	// p_point = &point_1;
	p_point-> p_x = 10;
	p_point-> p_y = 10;
	
	
	p_arg = &str_arg_func;
	
	deklaracje();
	
	__delay_ms(10);
	
	
	// po inicjalizacji
	t6963_initialization();
	
	
	clear_screen(0x00, t6963_columns * t6963_rows, 0x00);
	
	
		
	//text area
	// t6963_word_set_command(t6963_columns, 0 ,0x41);
	// t6963_word_set_command(t6963_columns, 0 ,0x41);
	//mode set  "AND"
	
	__delay_ms(1000);
	clear_screen('A' - 0x20, t6963_columns * t6963_rows, 0x00);
	// clear_sreen(0x21, 200);
	// t6963_write_byte(1,0x96);
	__delay_ms(1000);
	clear_screen(0x00, t6963_columns * t6963_rows, 0x00 );
	// clear_sreen(0x00, 200);
	// t6963_write_byte(1,0x96);
	//address pointer set
	// 
	t6963_addres_pointer_set(5,0x00);
	t6963_bajt_and_command('D' - 0x20, 0xC4);
	
	t6963_diplay_mode(t6963_graphic_on);
	// t6963_write_byte(1,0x98);
	clear_screen(0xaa, t6963_columns * t6963_rows*8, 0x0c00 );
	
	// t6963_bajt_and_command(0x26, 0xC0);
	// t6963_bajt_and_command(0x32, 0xC0);
	__delay_ms(1000);
	clear_screen(0x00, t6963_columns * t6963_rows*8, graphic_home_adress );
	__delay_ms(1000);
	MyMenu[disp_mode].variables_table[0] = t6963_text_on;
	t6963_diplay_mode(MyMenu[disp_mode].variables_table[0]);
	napisz_tekst(0,0,(unsigned char *)" to jest ekran demo ");
	
	
	napisz_tekst(0,3,(unsigned char *)"program stworzony  ");
	napisz_tekst(0,5,(unsigned char *)"przez  ");
	
	napisz_tekst(0,6,(unsigned char *)"Daniela Kucharskiego");
	
	napisz_tekst(0,15,(unsigned char *)"0123456789abcdefghi*&!");
	
	// t6963_simple_line(0,8,20,8);
	__delay_ms(2000);
	// t6963_write_byte(1,0x94);
	
	
	// t6963_addres_pointer_set(0x1,0);
	// t6963_bajt_and_command(0x24, 0xC0);
	
	// t6963_addres_pointer_set(0x5,5);
	// t6963_bajt_and_command(0x28, 0xC0);
	clear_screen(0x00, t6963_columns * t6963_rows, text_home_adress );
	
	
	InterruptsEnable();
	
	znaczniki |= odswiez_menu;
	
	while(1){
		
	
		// srand();
	
		if (znaczniki & odswiez_menu){
		
			znaczniki &= ~odswiez_menu;
			
			show_menu(p_arg->no_of_menu_option, (znaczniki & czy_jestem_w_menu));
		
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
			if (!(znaczniki & czy_jestem_w_menu) && (MyMenu[filling].variables_table[0] > 0)){
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
			napisz_tekst(17,15,tab2);
			
			}
			
			
		if ((jak_dlugo_wcisniete> jak_dlugo_klawisz_do_zmiany) && (znaczniki & czy_jestem_w_opcji) && ((KEY_RED_PORT == PRESSED) || (KEY_BLUE_PORT == PRESSED)) ){
			
				jak_dlugo_wcisniete =0;
				zmiana_opcji(p_arg->no_of_menu_option, &(MyMenu[p_arg->no_of_menu_option].variables_table[p_arg->no_of_marked_value_of_option]));
				// sprawdz_wyswietlanie_opcji_po_enter(nr_opcji, nr_marked_value);
				// menu_write_option_value( (struct arguments_for_functions *){nr_opcji,  nr_marked_value});
				// p_arg->var1 =  nr_opcji;
				// p_arg->var2 = nr_marked_value;
				menu_write_option_value(p_arg);
			
		};
		
		
		
		// if ( (KEY_RED_PORT == UNPRESSED) && (PIE1bits.TMR1IE == 0) && (jak_dlugo_wcisniete>jak_dlugo_klawisz_do_zmiany) && (znaczniki_wcisniecia & znacznik_key_red ) ) {
		if ( (KEY_RED_PORT == UNPRESSED) && (PIE1bits.TMR1IE == 0) && (jak_dlugo_wcisniete>jak_dlugo_klawisz_do_zmiany) && !(znaczniki & czy_jestem_w_menu)  ) {
			
			
			if  (MyMenu[disp_mode].variables_table[0] == (t6963_graphic_on | t6963_text_on) )
				MyMenu[disp_mode].variables_table[0] = t6963_text_on;
			else if (MyMenu[disp_mode].variables_table[0] == (t6963_graphic_on) )
				MyMenu[disp_mode].variables_table[0] = (t6963_graphic_on | t6963_text_on);
			else if (MyMenu[disp_mode].variables_table[0] == ( t6963_text_on) )	
				MyMenu[disp_mode].variables_table[0] = t6963_graphic_on;
			
			t6963_diplay_mode(MyMenu[disp_mode].variables_table[0]);
			jak_dlugo_wcisniete=0;
			
			
		}
		else if (KEY_RED_PORT == PRESSED && PIE1bits.TMR1IE == 0 ) {
		
			//klawisz czerwony wcisniety
						
			WRITETIMER1(0);
			PIE1bits.TMR1IE = 1;
			jak_dlugo_wcisniete=0;
			
			znaczniki_wcisniecia = 0;
			znaczniki_wcisniecia |= znacznik_key_red;
			
			if (znaczniki & czy_jestem_w_opcji){
				// znaczniki |= odswiez_menu;
				zmiana_opcji(p_arg->no_of_menu_option, &(MyMenu[p_arg->no_of_menu_option].variables_table[p_arg->no_of_marked_value_of_option]));
				// sprawdz_wyswietlanie_opcji_po_enter(nr_opcji, nr_marked_value);
				
				
				
				// p_arg->var1 =  nr_opcji;
				// p_arg->var2 = nr_marked_value;
				menu_write_option_value( p_arg);
			}
			else if ( (znaczniki & czy_jestem_w_menu) && !(znaczniki & czy_jestem_w_opcji) ){
				znaczniki |= odswiez_menu;
				if (  (p_arg->no_of_menu_option) < (Wymiar_Menu-1))
					(p_arg->no_of_menu_option)++;
				else
					(p_arg->no_of_menu_option) = 0;
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
			// sprintf(tab2,"%2d\0",nr_marked_value);
			// napisz_tekst(6,15,tab2);
			
			if ( (znaczniki & czy_jestem_w_opcji) && ( (MyMenu[p_arg->no_of_menu_option].how_many_options) == 0) ) {
			
				// options without values showing 
				// it is merely do sthg and leave after Black key pressed again
				if (   (p_arg->no_of_menu_option) == show_graph)
					{
					
					opusc_opcje(p_arg->no_of_menu_option);
					
					
					}
				
				znaczniki &= ~czy_jestem_w_opcji;
				znaczniki |= odswiez_menu;
			}
			else if ( (znaczniki & czy_jestem_w_opcji) && ( (p_arg->no_of_marked_value_of_option) == (MyMenu[p_arg->no_of_menu_option].how_many_options-1) ) ) {
				//odznaczam
				
				znaczniki &= ~czy_jestem_w_opcji;
				p_arg->no_of_marked_value_of_option = 0;
				opusc_opcje(p_arg->no_of_menu_option);
				znaczniki |= odswiez_menu;
			}	
			else if (znaczniki & czy_jestem_w_opcji) {
					
				(p_arg->no_of_marked_value_of_option)++;	
				// nr_marked_value = sprawdz_wyswietlanie_opcji_po_enter(nr_opcji, nr_marked_value);
				 // sprawdz_wyswietlanie_opcji_po_enter(nr_opcji, nr_marked_value);
				 // p_arg->var1 =  nr_opcji;
				// p_arg->var2 = nr_marked_value;
				menu_write_option_value( p_arg);
						
					
				
				
			}
			else if  ((znaczniki & czy_jestem_w_menu) && !(znaczniki & czy_jestem_w_opcji) ) {
				//zaznaczam
				//jesli jestem w menu i wchodze naciskam enter znow
				
				
				//jesli zdefiniowano wskaznik do funkcji to ja wykonaj
				if (MyMenu[p_arg->no_of_menu_option].function)
					MyMenu[p_arg->no_of_menu_option].function(p_arg);
				
			}	
			else{
				//jesli wchodze do menu
				znaczniki |= czy_jestem_w_menu;
				clear_screen(0x00, t6963_columns * t6963_rows*8, graphic_home_adress );
				t6963_mode_set(t6963_atribute);
				MyMenu[disp_mode].variables_table[0] = (t6963_graphic_on | t6963_text_on);
				t6963_diplay_mode(MyMenu[disp_mode].variables_table[0] );
				znaczniki |= odswiez_menu;}
				
			
	
			
		};
		
		if (KEY_BLUE_PORT == PRESSED && PIE1bits.TMR1IE == 0 ) {
		
			
			WRITETIMER1(0);
			PIE1bits.TMR1IE = 1;
			znaczniki_wcisniecia = 0;
			znaczniki_wcisniecia |= znacznik_key_blue;
			
			
			if (znaczniki & czy_jestem_w_opcji){
				// znaczniki |= odswiez_menu;
				zmiana_opcji(p_arg->no_of_menu_option, &(MyMenu[p_arg->no_of_menu_option].variables_table[(p_arg->no_of_marked_value_of_option)]));
				// sprawdz_wyswietlanie_opcji_po_enter(nr_opcji, nr_marked_value);
				// p_arg->var1 =  nr_opcji;
				// p_arg->var2 = nr_marked_value;
				menu_write_option_value( p_arg);
			}
			else if ( (znaczniki & czy_jestem_w_menu) && !(znaczniki & czy_jestem_w_opcji) ){
				znaczniki |= odswiez_menu;
				if ( p_arg->no_of_menu_option != 0)
					(p_arg->no_of_menu_option)--;
				else
					(p_arg->no_of_menu_option) = (Wymiar_Menu-1);
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
			// led_out = 0;
			// jak_dlugo_wcisniete=0;
		
			
		}
		else if ((KEY_BLUE_PORT == UNPRESSED) && (znaczniki_wcisniecia & znacznik_key_blue) ){
			PIE1bits.TMR1IE = 0;
			znaczniki_wcisniecia = 0;
			// jak_dlugo_wcisniete=0;
		};
		
		if ( ( (KEY_RED_PORT == PRESSED ) &&  (znaczniki_wcisniecia & znacznik_key_red)) || ( (KEY_BLUE_PORT == PRESSED ) &&  (znaczniki_wcisniecia & znacznik_key_blue)) ) {
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

unsigned char t6963_read_byte(unsigned char bajt){
	unsigned char dana;
	
	
	t6963_write_byte(1,0xc5);
	t6963_status_check(0);
	
	// tris_lcd_data = 0;	
	tris_lcd_data = 0xff;	
	//komenda odczytu
	// latch_lcd_data = 0xc5;
	latch_lcd_CD = 0;
	latch_lcd_WR = 1;
	latch_lcd_RD = 0;
	
	
	latch_lcd_CE = 0;	
	// __delay_us(1);
	Nop();
	Nop();
	latch_lcd_CE = 1;
	// __delay_us(1);
	
	latch_lcd_CD = 0;
	latch_lcd_RD = 1;
	
	Nop();
	
	dana = lcd_data;
	
	
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


void clear_screen(unsigned char znak, unsigned int ile, unsigned int adres){
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

void  t6963_ustaw_atrybut_tekstu(unsigned char pol_x, unsigned char pol_y, unsigned char *tab, unsigned char atrybut){
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
	t6963_mode_set(MyMenu[mode_set].variables_table[0]);
	
	
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
	unsigned int pos_x,pos_y;	
	float  a_func, b_func;
	unsigned char string1[40];
	signed int delta_y, delta_x;
	float pos_x_new, pos_y_new, sign_delta_x,dy   ;
	unsigned char count_X, count_Y;
	
	// a_func = ((signed int)pos_y1 - (signed int)pos_y2)  ;
	// sprintf( string1, "%3.3f \0",a_func);
	// napisz_tekst(10,14,string1);
			// __delay_ms(3000);
	
	// a_func =(float) ((signed int)pos_y1 - (signed int)pos_y2) / pos_x2;
	// sprintf( string1, "%3.3f \0",a_func);
	// napisz_tekst(10,14,string1);
			// __delay_ms(2000);
	delta_y = ( (signed int)pos_y2 - (signed int)pos_y1);
	delta_x = ((signed int)pos_x2 - (signed int) pos_x1);
	
	
	
	if (delta_x != 0)
		a_func = (float)delta_y/delta_x;
	else
		a_func = 1e7;

	sign_delta_x = delta_x/abs(delta_x);
	// sign_delta_y = delta_y/abs(delta_y);
	
	
	b_func = (float)(signed int)pos_y1 - a_func * (signed int)pos_x1;
	
	
	// __delay_ms(2000);
	
	// sprintf( string1, "%d \0",(pos_x1 != pos_x2));
	// napisz_tekst(10,14,string1);
			// __delay_ms(1000);
	pos_x = pos_x1;
	pos_y = pos_y1;
	//how many i write on the same x, y
	
	// pos_y_new = ((float)pos_x + (float)sign_delta_x)*a_func;			
	// pos_x_new = ((float)pos_y+(float)sign_delta_y)/a_func;
	//i am finding a new x i y and the increament is by 1, -1 or 0
	pos_y_new = ((float)pos_x + sign_delta_x)*a_func + b_func;			
	// pos_x_new = ((float)pos_y+(float)sign_delta_y)/a_func;
	
	// o ile przyrasta y
	dy = fabs(pos_y_new - (float)pos_y);
	// dx = abs(pos_x_new - pos_x);
	if (dy>1){
		
		sign_delta_x = sign_delta_x / dy;

	};
	
	pos_x_new = (float) pos_x1;
	sprintf( string1, "a = %f  %f\0",a_func, sign_delta_x);
	napisz_tekst(0,12,string1);
	while( (pos_x_new <= pos_x2) ){
	// && (pos_y <= pos_y2) ){
		
		pos_x_new = (float)pos_x_new + sign_delta_x;
		pos_y_new = (pos_x_new)*a_func + b_func ;
		
		
		// if ((pos_y_new > 1) && (count_Y < pos_y_new) ){ 
			// count_Y +=1;
				
		// }
		// else if ((pos_y_new > 1) && (count_Y >= pos_y_new)){
			// count_Y = 0;
			// pos_y = (unsigned int)pos_y_new;
		// }
		// else if ( (pos_y_new <= 1)  ){
		
		
			pos_y = (unsigned int)pos_y_new;
		// }
		
		// if ( (pos_x_new > 1) && (count_X < pos_x_new) )
			// count_X +=1;	
		// else if ((pos_x_new > 1) && (count_X >= pos_x_new)){
			// count_X = 0;
			// pos_x = (unsigned int)pos_x_new;
		// }
		// else if ( (pos_x_new <= 1)  ){
		
		
			pos_x = (unsigned int)pos_x_new;
		// }
		t6963_graphics_set_point(pos_x, pos_y,1);
	sprintf( string1, "x=%d y=%d %f %f  \0",pos_x,pos_y, pos_x_new, pos_y_new);
	napisz_tekst(0,13,string1);
	pos_x = 100*MyMenu[timer_set].variables_table[0];
	__delay_ms(50);
	
	}
	
	
}

void   t6963_graphics_set_point(unsigned int pos_x, unsigned int pos_y, unsigned char color){
	unsigned int  address;
	unsigned char wartosc, wartosc2;
	unsigned char a;
	unsigned char string1[5];
	a = (pos_x % t6963_font_size);
	color <<= 7;
	wartosc = color >> a;
	address = graphic_home_adress + pos_y* (ilosc_pixels_X/8)+ pos_x/t6963_font_size; 
	
	

	t6963_addres_pointer_set(address &0xff, address >> 8 );
	wartosc2 = t6963_read_byte(0);
	if (color)
		wartosc |= wartosc2;
	else
		wartosc &= wartosc2;
	
	t6963_bajt_and_command( wartosc , 0xc4);
	
	// sprintf(string1,"%X %X %d %X\0",wartosc,wartosc2,a,address);
	// napisz_tekst(0,13, string1);
	
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
		// clear_sreen(0x00, t6963_columns * t6963_rows*8, graphic_home_adress );
	};
		// t6963_mode_set(t6963_atribute);
		// MyMenu[disp_mode].how_many_options = (t6963_graphic_on | t6963_text_on);
		// t6963_diplay_mode(MyMenu[disp_mode].how_many_options );
		
	
	// }
	// else	
	// {
		
		
	// };
	

	for(pos_start=0; MyMenu[pos_start].napis ; pos_start++){
		
		napisz_tekst(0,pos_start, MyMenu[pos_start].napis);
		if ( (nr_zaznaczony == pos_start) && (stan !=0) ){
			t6963_ustaw_atrybut_tekstu(0,pos_start, MyMenu[pos_start].napis, atrybut_wybranej_opcji);
		
		}
		else
			t6963_ustaw_atrybut_tekstu(0,pos_start, MyMenu[pos_start].napis, 0);	
	
	};


};





void opusc_opcje(unsigned char nr){
	unsigned char puste[7]="      ";
	if  (nr == show_graph){
		//ustawiam znowu atribute mode
		t6963_mode_set(t6963_atribute);
		
		t6963_diplay_mode( t6963_graphic_on | t6963_text_on);
		znaczniki |= odswiez_menu;
				
			
		
	}
	else{
	napisz_tekst(pos_option_menu, nr,puste );
	t6963_ustaw_atrybut_tekstu(pos_option_menu,nr, puste, 0);
	}
	
	sprintf( puste ,"%d   \0", nr);
	napisz_tekst(0,15,puste);
	//wykonanie dodatkowych polecen
	
	if (nr == point_menu ){
		
		t6963_graphics_set_point(MyMenu[nr].variables_table[0],MyMenu[nr].variables_table[1],MyMenu[nr].variables_table[2]);
		
		
	}
	else if (nr == line_menu){
		
		t6963_simple_line(MyMenu[nr].variables_table[0],    MyMenu[nr].variables_table[1], MyMenu[nr].variables_table[2], MyMenu[nr].variables_table[3]  );
		
		// napisz_tekst(0,14,(unsigned char *)"linia\0");
		
	}
	
}


void   zmiana_opcji(unsigned char nr,unsigned int *p){
	unsigned char i ;
	unsigned char string1[8] ,  len;
	
	sprintf(string1,"%d %X\0",*p,p);
	napisz_tekst(5,15,string1);
	
	if (nr == mode_set){
		//0 OR, 1 - EXOR, 11 - AND,
		// i = 	MyMenu[mode_set].how_many_options;
		// *pp = strchr(tab_mode, 3);
		len = sizeof(tab_mode);
		for( i=0; i<len; i++){
			if (tab_mode[i] == *p){ 
				// pp= i;
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
		
		*p = tab_mode[i];
		
		
		
		
		
		

	}
	else if (nr == disp_mode){
		//0 OR, 1 - EXOR, 11 - AND,
		// i = 	MyMenu[mode_set].how_many_options;
		// *pp = strchr(tab_mode, 3);
		len = sizeof(tab_disp);
		for( i=0; i<len; i++){
			if (tab_disp[i] == *p){ 
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
		
		*p = tab_disp[i];
		
		
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
	
	
	
	else if (nr == point_menu){
		// MyMenu[nr].how_many_options +=1;
		if (znaczniki_wcisniecia & znacznik_key_blue){
			if (*p== ilosc_pixels_X)
				*p = 0;
			else
				(*p)++;
		}
		else if (znaczniki_wcisniecia & znacznik_key_red){
			if (*p== 0)
				*p = ilosc_pixels_X;
			else
				(*p)--;
		};
		//
	// sprintf(string1,"%d %d %X\0",len,*p,znaczniki_wcisniecia);
	// napisz_tekst(0,15,string1);	
	}
else{
		// MyMenu[nr].how_many_options +=1;
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
	// sprintf(string1,"*p=%d %X %d\0",*p,p,MyMenu[nr].how_many_options);
	// napisz_tekst(0,14,string1);
	
}

// void   menu_write_option_value(unsigned char nr_line, unsigned char which_marked){
void   menu_write_option_value(  struct arguments_for_functions *arg ){
	unsigned char  i , string1[5], nr_line = arg->no_of_menu_option,  which_marked = arg->no_of_marked_value_of_option;

		znaczniki |= czy_jestem_w_opcji;
	
		for( i=0; i<MyMenu[nr_line].how_many_options ; i++){
			
			sprintf(string1, "%3.2d\0",MyMenu[nr_line].variables_table[i]);
			napisz_tekst(pos_option_menu+i*3, nr_line, string1);
			if (i == (which_marked) )
				t6963_ustaw_atrybut_tekstu(pos_option_menu+i*3,nr_line, string1, atrybut_zmiany_var);
			else	
				t6963_ustaw_atrybut_tekstu(pos_option_menu+i*3,nr_line, string1, 0);
			
			
		}
		
		

	
	
}

//funkcja wyjscia Z MENU	
void  Menu_Exit(struct arguments_for_functions *arg){
	
	//opusc menu
		znaczniki &= ~czy_jestem_w_menu;
		znaczniki &= ~czy_jestem_w_opcji;
		
		znaczniki |= odswiez_menu;
		t6963_mode_set(MyMenu[mode_set].variables_table[0]);
		t6963_diplay_mode(MyMenu[disp_mode].variables_table[0]);
		// return 0;
	
}


void  Menu_Show_GFX(struct arguments_for_functions *arg){
	znaczniki |= czy_jestem_w_opcji;
	t6963_mode_set(t6963_EXOR);
	t6963_diplay_mode(t6963_graphic_on);
	
	
}



void  Menu_sin(struct arguments_for_functions *arg){
	unsigned int i,move,amplitude, rr, czas;
	unsigned char spring1[10];
	float f_i;
	
	// znaczniki |= czy_jestem_w_opcji;
	t6963_mode_set(t6963_EXOR);
	t6963_diplay_mode(t6963_graphic_on | t6963_text_on);
	
	
	while(KEY_RED_PORT == UNPRESSED ){
		for(i = 0;i<ilosc_pixels_X;i++)
		{
			czas = READTIMER0();
			// time(&toc);
			srand(czas);
			rr = rand();
			amplitude = 2+ rr % 40;
			
			
			if (KEY_RED_PORT == PRESSED)
				break;
			f_i= amplitude*sin((float)(i+move*5)/10)+MyMenu[show_sin].variables_table[0];
			sprintf(spring1, "%d \0",amplitude);
			napisz_tekst(0,14,spring1);
			t6963_graphics_set_point(i,(unsigned int)f_i, (move % 2));
			
			
		}
		move++;
	}
	t6963_mode_set(t6963_atribute);
		
		t6963_diplay_mode( t6963_graphic_on | t6963_text_on);
		znaczniki |= odswiez_menu;
}


void		Clear_All_GFX(struct arguments_for_functions *arg){
	
	
	clear_screen(0x00, t6963_columns * t6963_rows * 8, graphic_home_adress);
	
	
}

