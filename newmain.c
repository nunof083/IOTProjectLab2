// CONFIG
#pragma config FOSC = EXTRC     // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>
#include <stdio.h>
#define _XTAL_FREQ 20000000

#define RS PORTBbits.RB0
#define RW PORTBbits.RB1
#define EN PORTBbits.RB2

void lcd_cmd(unsigned char cmd) {
    PORTB = (cmd & 0xF0);
    EN = 1;
    RW = 0;
    RS = 0;
    __delay_ms(2);
    EN = 0;
    PORTB = ((cmd<<4) & 0xF0);
    EN = 1;
    RW = 0;
    RS = 0;
    __delay_ms(2);
    EN = 0;
}

void lcd_data(unsigned char data) {
    PORTB = (data & 0xF0);
    EN = 1;
    RW = 0;
    RS = 1;
    __delay_ms(2);
    EN = 0;
    PORTB = ((data<<4) & 0xF0);
    EN = 1;
    RW = 0;
    RS = 1;
    __delay_ms(2);
    EN = 0;
}

void lcd_init() {
    lcd_cmd(0x02);
    lcd_cmd(0x28); // 4-bit mode, 2 lines
    lcd_cmd(0x0C); // Display on, cursor off
    lcd_cmd(0x06); // Auto-increment cursor
    lcd_cmd(0x01); // Clear display
}

void lcd_string(const unsigned char *str, unsigned char num) {
    unsigned char i;
    for (i = 0; i < num; i++) {
        lcd_data(str[i]);
    }
}

unsigned int timer_count = 0;
unsigned int timer_count1 = 0;

char countString[5];
char countString2[5];

void __interrupt() timer_0() {
    if (INTCONbits.TMR0IF == 1) {
        timer_count++;
        timer_count1++;
        INTCONbits.TMR0IF = 0;   // Clear interrupt flag
        TMR0 = 59;               // Reload the timer
    }
}

void main(void) {
    TRISB = 0x00;               // PORTB = 0
    TRISA = 0x00;               // PORTA = 0
    
    //PINS
    TRISCbits.TRISC0 = 1;       // joseBouton = 1
    TRISCbits.TRISC1 = 1;       // marieBouton = 1
    
    TRISAbits.TRISA0 = 0;       // joseLED = 0
    TRISAbits.TRISA1 = 0;       // marieLED = 0
    
    TRISAbits.TRISA4 = 0;       // Buzzer = 0

    // Setup pour timer et interruptions
    INTCONbits.GIE = 1;         // Enable global interrupts
    INTCONbits.PEIE = 1;        // Enable peripheral interrupts
    INTCONbits.TMR0IE = 1;      // Enable Timer 0 interrupt
    OPTION_REG = 0x07;          // Timer 0 prescaler 1:256
    TMR0 = 59;                  // Valeur de départ du tmr0

    lcd_init();
    unsigned char jose = 0;
    unsigned char marie = 0;

    while(1) {
        // jose
        if (RC0 == 1) {
            __delay_ms(50); // Debounce delay
            if (RC0 == 1) {
                timer_count = 0;        // Reset le timer de jose
                jose = 0;               // Reset jose
                lcd_cmd(0x80);          // 1 ligne LCD
                lcd_string("Jose OK", 8);
                RA0 = 0;                // Eteindre LED
                RA4 = 0;                // Eteindre Buzzer
            }
        }

        if (timer_count < 500 && !jose) {
            lcd_cmd(0x80);               // 1 ligne LCD
            lcd_string("Jose OK", 8);
            jose = 1;                    // Jose OK
            RA0 = 0;                     // Eteindre LED
            if (timer_count1 < 500) {
                RA4 = 0;                  // Eteindre Buzzer si OK pour marie
            }
            else {
                RA4 = 1;
            }
        } else if (timer_count >= 500 && jose) {
            lcd_cmd(0x80);
            lcd_string("Jose NON", 8);
            jose = 0;                    // Reset jose
            RA0 = 1;                     // Allumer LED
            RA4 = 1;                     // Allumer Buzzer
        }

        // marie
        if (RC1 == 1) {
            __delay_ms(50); // Debounce delay
            if (RC1 == 1) {
                timer_count1 = 0;        // Reset timer de marie
                marie = 0;               // Reset marie
                lcd_cmd(0xC0);          
                lcd_string("Marie OK", 9);
                RA1 = 0;                 // Eteindre LED
                RA4 = 0;                 // Eteindre Buzzer
            }
        }

        if (timer_count1 < 500 && !marie) {
            lcd_cmd(0xC0);               // 2 ligne LCD
            lcd_string("Marie OK", 9);
            marie = 1;                    // Marie OK
            RA1 = 0;                      // Eteindre LED
            if (timer_count < 500) {
                RA4 = 0;                  // Eteindre Buzzer si OK pour jose
            }
            else {
                RA4 = 1;
            }
        } else if (timer_count1 >= 500 && marie) {
            lcd_cmd(0xC0);
            lcd_string("Marie NON", 9);
            marie = 0;                    // Reset marie
            RA1 = 1;                      // Allumer LED
            RA4 = 1;                      // Allumer Buzzer
        }
    }
}
