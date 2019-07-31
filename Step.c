/*
Step - Treppenstufen Timer
*/

#include <avr \io.h>
#include <avr \interrupt.h>
#include <avr \sleep.h>

/*
all my ATtiny85 chips have their 8MHz fuse set
by default they run at 1MHz, so adjust accordingly
this constant is used by delay.h, so make sure it stays above the include
*/
#define F_CPU 9600000
#include <util \delay.h>

#include <avr/interrupt.h>

#define TREPPE ((_BV(4) & PINB ) !=0)

typedef struct {
        uint8_t on ;          //50ms
        uint8_t off ;
        uint8_t cnt_pitch ;  // 4-600Hz
} BEEP_T ;

BEEP_T beep_muster []=
{
        {0,  100, 100}, // off
        {10, 2,    7},  // Warnzeit
        {2,  3,    5},  // Stufe fährt
        {10, 1,    3},  // Nachlauf
        {1,  1,    2},  // Fehlerfall
        {1,  20,   4}   // Stufe später runter
};

volatile uint8_t   beep_value       =1;
volatile uint8_t   timer_cnt        =0;
volatile uint16_t high_timer_cnt=0;       // 50ms Timer, bis zum Überlauf wird es wohl kaum kommen (3276sek ~1h)

volatile int     change_flag  =0;

void  do_beep (int ton)
{
        beep_value = ton;
        change_flag=1;
}

ISR(TIM0_OVF_vect )
{
   static uint8_t beep_cnt_end =0;
   static uint8_t pitch_cnt =0;
   static uint8_t beep_cnt =0;
   static uint8_t beep_on =0;

// 2.Pin inverse für lauteren Ton

        // Tonerzeugung, Frequenz 5kHz / 2 / cnt_pitch
        pitch_cnt++;
        if ( pitch_cnt >= beep_muster[ beep_value].cnt_pitch )
        {                     // 3-600Hz
               pitch_cnt=0;
               if ( beep_on)
               {
                      PORTB |= (_BV(3));  // nur an
                      PORTB ^= (_BV(0));   //toggle pin -> Sound
                      PORTB ^= (_BV(2));   //toggle pin -> Sound
               }
               else
               {
                      PORTB &= ~(_BV(3)); 
                      PORTB &= ~(_BV(0));   //Pin low
                      PORTB |=   (_BV(2));   //Pin high
               }
        }

        // Haupttimer 50ms
        timer_cnt++;
        if ( timer_cnt>238 || change_flag )        // 50ms
        {      
               // Sound wurde nicht geändert
               if (! change_flag)
               {
                      timer_cnt=0;
                      high_timer_cnt++;                 
               }
               else   // Sound wurde geändert
               {
                      change_flag = 0;
                      beep_cnt_end=0;
                      beep_on = 0;
               }

               // Soundmuster spielen            
               beep_cnt++;
               if ( beep_cnt > beep_cnt_end)
               {
                      if ( beep_on)
                      {
                            beep_cnt=0;
                            beep_cnt_end = beep_muster[ beep_value].off ;
                            beep_on=0;
                      }
                      else
                      {
                            beep_cnt=0;
                            beep_cnt_end = beep_muster[ beep_value].on ;
                            if ( beep_cnt_end!=0)  beep_on=1;
                      }
               }
        }
}

int main (int argc, char ** argv)
{
 uint8_t state = 1, old_treppe=0;
 uint16_t c=0;

 // go from internal 9,6MHz /8 -> /1
 CLKPR = _BV( CLKPCE) ;
 CLKPR = 0 ;
 
 cli();
 
 DDRB |= _BV(0) | _BV(1) | _BV(2) | _BV(3);
       
        // _BV(0)  PB6AddOn -> Sound
        // _BV(1)  PB7AddOn -> Relais
        // _BV(2)  PB8AddOn -> Test-> Sound invers
        // _BV(3) Test Ausgang PB5 Addon  -> Sound AN Signal für externen Summer
       
        // _BV(4) als Input eingang  PB4 Addon
 
 PORTB = 0 + _BV(2);
 
 //prescale timer to 8
 TCCR0B |= (1<< CS01);
 
 // 9,6MHz / 8 -> 1,2MHz  mit Timer 256 -> 4687Hz  213us Überlauf
 
 //enable timer overflow interrupt
 TIMSK0 |= (1<< TOIE0);

 sei();
 
 do_beep(0);

 while(1)
 {
        switch ( state)
        {
               case 1:      // Start
                      if ( TREPPE) 
                      {
                            state = 10;
                            do_beep(0);
                      }
                      else
                      {     // 2 Sek nur Signal
                            state=2;
                            c=high_timer_cnt +2000/50;
                            do_beep(1);                      
                      }
               break;

               case 2:      // nur Signal
                      if ( c<=high_timer_cnt )
                      {
                            state=3;
                            c=high_timer_cnt +3000/50;
                           
                            PORTB |= (_BV(1));   // Relais an
                            do_beep(2);
                      }
               break; 

               case 3:      // 3Sek Stufe hoch
                      if ( TREPPE)
                      {     // nur Nachlauf                          
                            c=high_timer_cnt +500/50;
                            state=4;
                            do_beep(3);
                      }

                      if ( c<=high_timer_cnt )
                      {     // Fehlerfall !!!!!!
                            PORTB &= ~(_BV(1));  // Relais aus
                            c=high_timer_cnt +10000/50;// Zeit abgelaufen 10sek Signal
                            state = 6;
                            do_beep(4);
                      }     
               break;

               case 4:      // 0,5sek Nachlauf
               if ( c<=high_timer_cnt )
               {
                      state=10;
                      old_treppe = TREPPE;
                      PORTB &= ~(_BV(1));  // Relais aus
                      do_beep(0);
               }
               break;

               case 6:      // Stufe kommt nicht hoch, Motor aus und Signal
                      if ( c<=high_timer_cnt )
                      {
                            state=10;
                            do_beep(0);
                      }
               break;

               case 10:     // Ende, alles aus und Endlosschleife.
                      PORTB &= ~(_BV(1));   // Relais aus - nur um sicher zu sein
                      if ( old_treppe != TREPPE)
                      {             // falls Treppe jetzt runter kommt, nur schwaches Signal
                            if (! TREPPE)  do_beep(5);
                            else do_beep (0);
                            old_treppe = TREPPE;
                      }
               break;

               default:
                      state = 10;  // for nothing to happen
                      break;
        } // switch
 } //while
}
// Ende
