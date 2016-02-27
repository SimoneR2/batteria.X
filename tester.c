/*==============================================================================
 *PROGRAM: Tester
 *WRITTEN BY: Simone Righetti
 *DATA: 27/02/2016
 *VERSION: 1.0
 *FILE SAVED AS: tester.c
 *FOR PIC: 18F2550
 *CLOCK FREQUENCY: 16 MHz
 *PROGRAM FUNCTION: 

======================================          
=         INPUT AND OUTPUTS          =            
=   RA0 => Tensione batteria (ADC)   =            
=   RA1 => Warning LED               =            
=   RD4-5-6-7 => ECCP (PWM Motore)   =   
=   RB2/RB3 => CANBus                =            
======================================
*/
#include <xc.h>
#include "pic_config.h"
#define _XTAL_FREQ 16000000
#define LCD_DEFAULT
#include "LCD_44780.h" 
#include "LCD_44780.c"
#include "delay.h"
#include "delay.c"
#define R1 15000 //INSERIRE VALORE KOhm PARTITORE
#define R2 7500 //INSERIRE VALORE KOhm PARTITORE
void inizializzazione(void);
void read_adc(void);
void ricarica (void);

unsigned char combinazioni[] = {
    0x00,
    0x01,
    0x02
};
int lettura[] = 0;
float current, voltage, rapporto = 0;

void main(void) {
    delay_ms(1);
    rapporto = (R1+R2);
    rapporto = R2/rapporto;
    inizializzazione();
    read_adc();
    while (1) {
        read_adc();
    }
}

void read_adc(void) {
    for (char i = 0; i < 3; i++) {
        ADCON0 = combinazioni[i]; //disattivo conversione, imposto il canale interessato
        ADCON0bits.ADON = 1; //attivo ADC
        ADCON0bits.GO = 1; //inzio conversione
        while (ADCON0bits.GODONE == 1); //attendo fine conversione
        lettura [i] = ADRESH; //salvo il dato
        lettura [i] = ((lettura[i] << 8) | ADRESL); //salvo il dato
        delay_ms(5); //attesa random
    }
    current = (lettura[1]-lettura[0]);
    voltage = (lettura[2]*5)/1024;
    voltage = voltage * rapporto;
}

void inizializzazione(void) {
    LATA = 0x00;
    TRISA = 0xFF; //PORTA all input

    LATB = 0x00;
    TRISB = 0b00; //PORTB ALL OUTPUTS

    LATC = 0x00;
    TRISC = 0b11011111;

    LCD_initialize(16);
    LCD_write_message("inizializzazione");
    delay_ms(1000);
    LCD_clear();

    ADCON0 = 0b00000000; //DISABILITO TUTTO
    ADCON1 = 0b00001101;
    ADCON2 = 0b10110010;
    ADCON0bits.CHS3 = 0; //IMPOSTAZIONE DI SICUREZZA
    ADCON0bits.CHS2 = 0; //IMPOSTAZIONE DI SICUREZZA
    ADCON0bits.CHS1 = 0; //IMPOSTAZIONE DI SICUREZZA
}