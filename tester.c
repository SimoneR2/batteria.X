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
#define USE_AND_MASKS
#include <xc.h>
#include "pic_config.h"
#define _XTAL_FREQ 16000000
#define LCD_DEFAULT
#include "LCD_44780.h" 
#include "LCD_44780.c"
#include "delay.h"
#include "delay.c"
#include <stdio.h>
#include <math.h>
//#include "adc.h"

#define R1 17825 //INSERIRE VALORE Ohm PARTITORE
#define R2 8111 //INSERIRE VALORE Ohm PARTITORE
void inizializzazione(void);
void read_adc(void);
void ricarica(void);
void scarica(void);

unsigned char combinazioni[] = {
    0b00000001, //AN0
    0b00000101, //AN1
    0b00001001  //AN2
};
signed long lettura[3] = 0;
float current, voltage, rapporto = 0;
unsigned char str [8] = 0;
unsigned long tempo, tempo_old, tempo_delay = 0;
unsigned int ore, minuti, secondi = 0;
float sommatoriaCorrente = 0;

__interrupt(high_priority) void isr_alta(void) { //incremento ogni secondo
    INTCONbits.TMR0IF = 0;
    TMR0H = 0x0B;
    TMR0L = 0xDC;
    tempo++;
    secondi++;
    if (secondi == 60) {
        secondi = 0;
        minuti++;
        if (minuti == 60) {
            minuti = 0;
            ore++;
        }
    }
}

void main(void) {
    delay_set_quartz(16);
    rapporto = (R1 + R2);
    rapporto = R2 / rapporto;
    inizializzazione();
    read_adc();
    while (1) {
        ricarica();
        LCD_clear();
        scarica();
        LCD_clear();
        while(voltage>5){
            LCD_home();
            LCD_write_message("Scollegare batt");
            LCD_goto_line(2);
            LCD_write_message("per nuovo test.");
            delay_s(1);
            read_adc();
        }
    }
}

void scarica(void) {
    tempo = 0;
    T0CON = 0x85;
    TMR0H = 0x0B;
    TMR0L = 0xDC;
    INTCONbits.GIE = 1;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    secondi = 0;
    minuti = 0;
    ore = 0;
    
    while (voltage > 10) {
        //LCD_clear();
        LATAbits.LATA3 = 1;
        LCD_home();
        LCD_write_message("tempo:");
        LCD_write_integer(ore, 2, ZERO_CLEANING_OFF);
        LCD_write_message(":");
        LCD_write_integer(minuti, 2, ZERO_CLEANING_OFF);
        LCD_write_message(":");
        LCD_write_integer(secondi, 2, ZERO_CLEANING_OFF);
        read_adc();
        LCD_goto_line(2);
        sprintf(str, "V:%.3f", voltage); //convert float to char
        str[7] = '\0'; //add null character
        LCD_write_string(str); //write Voltage in LCD
        sprintf(str, " I:%.3f", current); //convert float to char
        str[7] = '\0'; //add null character
        LCD_write_string(str); //write Current in LCD
        delay_ms(100);
        if (tempo - tempo_old >= 59) {
            tempo_old = tempo;
            sommatoriaCorrente = current + sommatoriaCorrente;
        }
    }
    LCD_clear();
    LATAbits.LATA3 = 0;
    LCD_write_message("batteria scarica");
    LCD_goto_line(2);
    LCD_write_message("C:");
        float time = 0;
        time = secondi/(60*60);
        time = time+minuti/60;
        time = time+ore;
        if (time>0){
        sommatoriaCorrente = sommatoriaCorrente/time;
        sommatoriaCorrente = -(sommatoriaCorrente);
        sprintf(str, "%.2f", sommatoriaCorrente); //convert float to char
        str[7] = '\0'; //add null character
        LCD_write_string(str); //write Voltage in LCD
        LCD_write_message("Ah");
        }
//    } else {
//        if ((tempo!=0)&&(sommatoriaCorrente!=0)){
//        sommatoriaCorrente = sommatoriaCorrente / (minuti*60);
//        sommatoriaCorrente = 0-sommatoriaCorrente;
//        sprintf(str, "%.3f", sommatoriaCorrente); //convert float to char
//        str[7] = '\0'; //add null character
//        LCD_write_string(str); //write Voltage in LCD
//        LCD_write_message("Ah");
//    }
        else{
            LCD_write_message("ERRORE!!");
        }
    }
    delay_s(5);
}

void ricarica(void) {
    read_adc();
    while ((current < -0.5) || (voltage < 13.5)) {
        PORTCbits.RC6 = 1; //attivo ciclo ricarica
        LCD_goto_line(1);
        LCD_write_message("Ciclo ricarica..");
        LCD_goto_line(2);
        sprintf(str, "V:%.3f", voltage); //convert float to char
        str[7] = '\0'; //add null character
        LCD_write_string(str); //write Voltage in LCD
        sprintf(str, " I:%.3f", current); //convert float to char
        str[7] = '\0'; //add null character
        LCD_write_string(str); //write Current in LCD
        read_adc();
        delay_ms(500); //attendi un po' prima di rileggere il tutto
       //
        delay_ms(1);
    }
    if ((current > -1)&&(voltage > 14.5)) {
        LCD_write_message("Carica terminata");
        delay_ms(5000);
    }
}

void read_adc(void) {
    for (char i = 0; i < 3; i++) {
        ADCON0 = combinazioni[i]; //disattivo conversione, imposto il canale interessato
        ADCON0bits.GO = 1; //inzio conversione
        while (ADCON0bits.GODONE == 1); //attendo fine conversione
        lettura [i] = ADRESH; //salvo il dato
        lettura [i] = ((lettura[i] << 8) | ADRESL); //salvo il dato
        delay_ms(5); //attesa random
    }
    current = (lettura[0] - lettura[1]);
    current = (current * 5);
    current = current / 1024;
    current = current / 0.200;
    voltage = (lettura[2]);
    voltage = (voltage * 5) / 1024;
    voltage = voltage / rapporto; //Conversione in tensione reale
}

void inizializzazione(void) {
    LATA = 0x00;
    TRISA = 0b11110111; //PORTA all input

    LATB = 0x00;
    TRISB = 0x00; //PORTB ALL OUTPUTS

    LATC = 0x00;
    TRISC = 0b11111111;

    LCD_initialize(16);
    LCD_write_message("TESTER BATTERIE");
    delay_ms(500);
    LCD_clear();
    //OpenADC(ADC_FOSC_16 & ADC_RIGHT_JUST & ADC_16_TAD, ADC_CH0 & ADC_REF_VDD_VSS & ADC_INT_OFF, ADC_4ANA);
    ADCON0 = 0b00000000; //DISABILITO TUTTO
    ADCON1 = 0b00001100;
    ADCON2 = 0b10111110;
    ADCON0bits.CHS3 = 0; //IMPOSTAZIONE DI SICUREZZA
    ADCON0bits.CHS2 = 0; //IMPOSTAZIONE DI SICUREZZA
    ADCON0bits.CHS1 = 0; //IMPOSTAZIONE DI SICUREZZA
    T0CON = 0x85;
    TMR0H = 0x0B;
    TMR0L = 0xDC;
    INTCONbits.GIE = 1;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    ADCON0bits.ADON = 1; //attivo ADC
}