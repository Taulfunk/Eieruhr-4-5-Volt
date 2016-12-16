//
//  segmentanzeige.h
//  Eieruhr
//
//  Created by Paul Funk on 29.10.16.
//  Copyright © 2016 Paul Funk. All rights resörved.
//

#ifndef segment_h
#define segment_h

extern uint8_t Sekunden;
extern void playTone(uint8_t, uint16_t);

/*Welche Bits legen welche Zahlen fest?*/
#define ZERO    0b01111111
#define ONE     0b00000111
#define TWO     0b10111101
#define THREE   0b10011111
#define FOUR    0b11000111
#define FIVE    0b11011011
#define SIX     0b11111011
#define SEVEN   0b00001111
#define EIGHT   0b11111111
#define NINE    0b11011111

/*Verschiedene defines für die Anzeige*/
#define RECHTS  PC4
#define LINKS   PC5
#define ZEROL   0b01111110
#define SEGMENTGEWARTE  10000           // 10000us = 10 ms

uint8_t counter[10] =
{
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE
};

uint8_t u8Minuten[7]=
{
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
};

/*Zeigt den zweistelligen Dezimalwert des übergebenen Wertes an ohne down zu counten.
 Dabei wird schnell zwischen den beiden Segmentanzeigen getoggled um Ports zu sparen*/
void Anzeige(uint8_t timer)
{
    PORTB = counter[timer / 10];
    (Sekunden == 1) ? (PORTB |= (1 << 0)) : (PORTB &= ~(1 << 0));
    PORTC = _BV(LINKS);
    _delay_us(SEGMENTGEWARTE);
    PORTB = counter[timer % 10];
    (Sekunden == 1) ? (PORTB &= ~(1 << 0)) : (PORTB |= (1 << 0));
    PORTC = _BV(RECHTS);
    _delay_us(SEGMENTGEWARTE);
}

/*Gibt ebenfalls den Wert an. Zaehlt abhaengig vom Modus in Sekunden oder Minuten herunter*/
void Anzeige2(uint16_t timer, uint16_t Sekunden)
{
    switch(Sekunden)
    {
        case (1):
            for (uint16_t a = timer; a > 0; a--)
            {
                for(uint8_t i = 0; i < 51; i++)     //Läuft eine Sekunde
                {
                    PORTB = counter[a / 10];
                    PORTB &= ~(1 << 0);
                    PORTC = _BV(LINKS);
                    _delay_us(SEGMENTGEWARTE);
                    PORTB = counter[a % 10];
                    PORTC = _BV(RECHTS);
                    _delay_us(SEGMENTGEWARTE);
                }
            }
            break;
            
        case (0):
            for (uint16_t a = timer; a > 0; a--)
            {
                /*diese Anzahl an Durchlaeufen hat sich ergeben
                 bei der Suche nach einem moeglichst genauen Countdown ohne Quarz*/
                
                /*erneute Suche bei 4,5 Volt Spannung:
                 90 Minuten:
                 51*60+18---------------------------------(+48)
                 ---------------------------------------------
                 30 Minuten:
                 51*60+16-------------175900 - 185900eta||185931(+31)
                 51*60+16------------190259+ - 200259eta||200331(+32)
                 51*60+5--------------210830 - 213830eta||213845(+15)
                 50*60+50------------214100  - 221105eta||221101(-4)
                 60 Minuten:
                 51*60+12-------------200520 - 210520eta||210549(+29)
                 50*60+50-------------073300 - 083300eta||083300
                 50*60+50................061800 - 071800eta||071802(+2)
                 */
                
                for(uint16_t i = 0; i <= (50*60+50); i++)
                {
                    PORTB = counter[a / 10];
                    PORTB &= ~(1 << 0);
                    PORTC = _BV(LINKS);
                    _delay_us(SEGMENTGEWARTE);
                    PORTB = counter[a % 10];
                    PORTC = _BV(RECHTS);
                    _delay_us(SEGMENTGEWARTE);
                }
            }
            break;
        default:
            break;
    }
}

/*
 0b00000001 - Punkt
 0b00000010 - Unten rechts
 0b00000100 - Oben rechts
 0b00001000 - Oben
 0b00010000 - Unten
 0b00100000 - Unten links
 0b01000000 - Oben links
 0b10000000 - Mitte
 => E = 0b11111000 = 0xF8
 => I = 0b01100001 = 0x61
 */

void Hi(void)
{
    playTone(1, 50);
    _delay_ms(50);
    playTone(1, 50);
    _delay_ms(50);
    playTone(1, 50);
    _delay_ms(50);
    for(uint8_t a = 0; a < 35; a++)
    {
        PORTB = 0xF8;
        PORTC = _BV(LINKS);
        _delay_us(SEGMENTGEWARTE);
        PORTB = 0x61;
        PORTC = _BV(RECHTS);
        _delay_us(SEGMENTGEWARTE);
    }
}

#endif /* segment_h */
