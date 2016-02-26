/*
 * File:   tester.c
 * Author: simon
 *
 * Created on 26 febbraio 2016, 16.12
 */


#include <xc.h>
#include "pic_config.h"
#define _XTAL_FREQ 16000000
#define LCD_DEFAULT
#include "LCD_44780.h" 
#include "LCD_44780.c"
#include "delay.h"
#include "delay.c"
void inizializzazione(void);

void main(void) {
    inizializzazione();
    while (1) {
        LCD_write_message("OK OK OK OK OK !");
        delay_ms(1000);
    }
}

void inizializzazione(void) {
    LATA = 0x00;
    TRISA = 0xFF; //PORTA all input

    LATB = 0x00;
    TRISB = 0b00; //PORTB ALL OUTPUTS

    LATC = 0x00;
    TRISC = 0b11111111;

    LCD_initialize(16);
    LCD_write_message("inizializzazione");
    delay_ms(1000);
    LCD_clear();
}