#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Segment.h"
#include <avr/eeprom.h>

#define DEBOUNCE 1500
#define WAIT 500
#define SET             PD0
#define Speaker         PD6
#define SPEAKER_DDR     DDRD
#define PRESET          PD4
#define ENCODERBUTTON   PD5
#define RESETBUTTON     PD7
#define PRESET2         PD1

volatile uint8_t duration = 0;
uint8_t interruptknopf = 0;
uint8_t Sekunden = 0;
uint8_t Preset = 20;

struct
{
    uint8_t minuto;
    uint8_t sekundo;
}
Preset1, Preset2;

void playTone(uint8_t, uint16_t);
ISR(INT0_vect)
{
//    _delay_us(DEBOUNCE);
    if (bit_is_set(PIND, PD2))
    {
        _delay_us(0);
        
        if (bit_is_set(PIND, PD3))      //rechtsdreh
        {
            if(duration < 98)
            {
                duration++;
                playTone(1, 1);
                
            }
            else if(duration < 99)
            {
                duration++;
                playTone(1, 1);
            }
            else
            {
                duration = 0;
                playTone(1, 10);
            }
        }
        
        else if(bit_is_clear(PIND, PD3))    //linksdreh
        {
            
            if(duration > 1)
            {
                duration--;
                playTone(1, 1);
            }
            
            else if(duration > 0)
            {
                duration = 0;
                playTone(1, 10);
            }
        
            else
            {
                duration = 99;
                playTone(1, 10);
                _delay_us(WAIT);
            }
        }
    }
}

/*ResetButtonInterrupt*/
/*EncoderButtonInterrupt*/
ISR(PCINT2_vect)
{
    _delay_us(DEBOUNCE);
    bit_is_set(PIND, SET) ? (Sekunden = 1) : (Sekunden = 0);
    _delay_us(DEBOUNCE);
    
    /*Erster der beiden Presetbuttons*/
    if (bit_is_set(PIND, PRESET))
    {
        _delay_ms(200);
        if (bit_is_set(PIND, PRESET))
        {
            switch(Sekunden)
            {
                case 1:
                    if(Preset1.sekundo != duration)
                    {
                        eeprom_update_byte((uint8_t*)47, duration);
                        Preset1.sekundo = duration;
                        
                        for (uint8_t i = 0; i < 3; i++)
                        {
                            playTone(1, 10);
                            _delay_ms(100);
                        }
                    }
                    else
                    {
                        duration = Preset1.sekundo;
                        playTone(10, 10);
                    }
                    break;
                case 0:
                    if(Preset1.minuto != duration)
                    {
                        eeprom_update_byte((uint8_t*)46, duration);
                        Preset1.minuto = duration;
                        for (uint8_t i = 0; i < 3; i++)
                        {
                            playTone(1, 10);
                            _delay_ms(100);
                        }
                    }
                    else
                    {
                        duration = Preset1.minuto;
                        playTone(10, 10);
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch (Sekunden)
            {
                case 1:
                    duration = Preset1.sekundo;
                    break;
                case 0:
                    duration = Preset1.minuto;
                default:
                    break;
            }
        }
    }

    
    /*Countdown starten*/
    else if (bit_is_set(PIND, ENCODERBUTTON))
    {
        /*Verhindert eine Störung durch Bedienung der Knoepfe waehrend des Kochvorganges*/
        cli();
        
        switch (Sekunden)
        {
            case 1:
                Anzeige2(duration, 1);
                break;
            case 0:
                Anzeige2(duration, 0);
                break;
            default:
                break;
        }
        
        Anzeige(0);
        duration = 0;
        
        /*Interrupts reenabled um dem Piepen ein Ende setzen zu koennen*/
        sei();
        
                        /*Piept*/
                for (uint16_t a = 0; a <= 9; a++)
                {
                    playTone(1, 100);
                    for(uint8_t i = 0; i < 51; i++)     //Laeuft eine Sekunde
                    {
                        PORTB = 0b10000000;
                        PORTB &= ~(1 << 0);
                        PORTC = _BV(LINKS);
                        _delay_us(SEGMENTGEWARTE);
                        PORTB = counter[a];
                        PORTC = _BV(RECHTS);
                        _delay_us(SEGMENTGEWARTE);
                    }
                    
                    if (duration != 0)
                        return;
                }
        
                   /*Nervt einfach nur rum*/
                for (uint8_t i = 0; i <= 1000; i++)
                {
                    PORTB = 0b10000000;
                    PORTC = _BV(LINKS);
                    _delay_us(SEGMENTGEWARTE);
                    PORTB = counter[9];
                    PORTC = _BV(RECHTS);
                    _delay_us(SEGMENTGEWARTE);
                    playTone(1, 100);
                    _delay_ms(10);
                    
                    if (duration != 0)
                        return;
                }
            _delay_ms(855);
    }
}

void initInterrupt0(void)
{
    EIMSK  |= (1 << INT0);                  /*enablet INT0*/
    
    EICRA  |= (1 << ISC00) | (1 << ISC01);  /*triggert INT0 bei aufsteigender Kante*/
    
    PCICR  |= (1 << PCIE2);                 /*Versuch, PD$ mit einem Interruptbutton auszustatten*/
    //PCMSK2 |= (1 << RESETBUTTON);   //hardgewired
    PCMSK2 |= (1 << ENCODERBUTTON);
    PCMSK2 |= (1 << SET);       //Min,Sek
    PCMSK2 |= (1 << PRESET);
    sei();                                  /*setzt (global) interrupt enable bit*/
}

void initTimer(void)
{
    TCCR0A |= (1 << WGM01);                 /*entert ctc mode*/
    TCCR0A |= (1 << COM0A0);                /*Togglet OC0A bei Compare Match*/
    TCCR0B |= (1 << CS00) | (1 << CS01) ;    /*clk/64*/
    
    /*OCR0A enthält den tongebenden vergleichenden Wert*/
}

/*macht irgendwie brauchbare Toene*/
void playTone(uint8_t wavelength, uint16_t duration)
{
    /*lässt OCSR0A nach ja initialisierter Timer-ctc-Funktion den zu erreichenden Wert beinhalten*/
    OCR0A = wavelength;
    SPEAKER_DDR |= (1 << Speaker);
    
    while (duration)
    {
        _delay_ms(1);
        duration--;
    }
    SPEAKER_DDR &= ~(1 << Speaker);
}

int main(void)
{
    initTimer();
    initInterrupt0();
    DDRB = 0xff;
    DDRC = 0xff;
    DDRD = 0;
    DDRD |= (1 << RESETBUTTON) | (1 << PRESET2);
    PORTD &= ~(1 << SET);
    PORTD &= ~(1 << PRESET);
    
    /*Befähigt den Avr zur Wiederaufnahme der voreingestellten Presets nach Reset*/
    Preset1.minuto = eeprom_read_byte((uint8_t*)46);
    Preset1.sekundo = eeprom_read_byte((uint8_t*)47);
    Preset2.minuto = eeprom_read_byte((uint8_t*)48);
    Preset2.sekundo = eeprom_read_byte((uint8_t*)49);
    
    /*Überprüfung des Minuten/Sekundeschalters*/
    bit_is_set(PIND, SET) ? (Sekunden = 1) : (Sekunden = 0);
    
    /*Initialisiert den Startwert der Uhr.
     Die favoritisierten Werte sollten also mit dem ersten Preset gespeichert werden*/
    Sekunden ? (duration = Preset1.sekundo) : (Preset1.minuto);
    
    /*Begrueßung*/
    Hi();
    PORTB = 0xff;
    
    /*Initialisiert das wechsselseitige Erden der Segementanzeigen*/
    PORTC = (1 << PC4) & ~(1 << PC5);
    
    while(1)
    {
        /*Gibt einfach nur die Anzeige wieder*/
        Anzeige(duration);
    }
    return 0;
}

