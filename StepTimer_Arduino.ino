/*
Step - Treppenstufen Timer
Anpassung für Arduino - PWM Version

ATTiny25 reicht
Sketch uses 1710 bytes (83%) of program storage space. Maximum is 2048 bytes.
Global variables use 51 bytes (39%) of dynamic memory, leaving 77 bytes for local variables. Maximum is 128 bytes.

*/

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define ATTINY
#endif

#ifdef ATTINY
  #define TREPPE_IN 4   // Pin3 PB4 
  #define RELAIS    1   // Pin6 PB1
  #define SOUND_INV 0   // Pin5 PB0
  #define SOUND     3   // Pin2 PB3
  #define TEST      2   // Pin7 PB2
#else   // Arduino UNO
  #define TREPPE_IN 4 
  #define RELAIS    6 
  #define SOUND_INV 5 
  #define SOUND     3 
  #define TEST      2 
  #define START      7 
#endif   

#ifdef ATTINY    
 #define TREPPE digitalRead(TREPPE_IN)
#else 
 #define TREPPE (!digitalRead(TREPPE_IN))
#endif

#define TIME_START  2000  // ms
#define TIME_RELAIS 3000
#define TIME_AFTER  500
#define TIME_ERROR  10000

#define STEP_WAIT   1
#define STEP_SIGNAL 2
#define STEP_TREPPE 3
#define STEP_AFTER  4
#define STEP_ERROR  5
#define STEP_END    6
        
#define BEEP_OFF    0
#define BEEP_SIGNAL 1
#define BEEP_RELAIS 2
#define BEEP_AFTER  3
#define BEEP_ERROR  4
#define BEEP_LATER  5

typedef struct {
        uint8_t on ;          //50ms
        uint8_t off ;
        uint8_t cnt_pitch ;  // Frequenz Vorteiler (4-600Hz)
} BEEP_T ;

/*
  // Tonerzeugung, Frequenz 5kHz / 2 / cnt_pitch
BEEP_T beep_muster []=
{
  {0,  100, 100}, // off
  {10, 2,    7},  // Warnzeit   357Hz
  {2,  3,    5},  // Stufe fährt  500Hz
  {10, 1,    3},  // Nachlauf     833Hz
  {1,  1,    2},  // Fehlerfall   1250Hz
  {1,  20,   4}   // Stufe später runter  625Hz
};
*/
/*
// mit 1ms  recht ähnlich
BEEP_T beep_muster []=
{
  {0,  100, 100}, // off
  {10, 2,    3},  // Warnzeit 333Hz
  {2,  3,    2},  // Stufe fährt    500Hz
  {10, 1,    4},  // Nachlauf   250Hz
  {1,  1,    1},  // Fehlerfall   1kHz
  {1,  20,   2}   // Stufe später runter  500Hz
};
*/
// PWM
BEEP_T beep_muster []=
{
  {0,  0,  0}, // off
  {10, 2,    357},  // Warnzeit   357Hz
  {2,  3,    500},  // Stufe fährt  500Hz
  {10, 1,    833},  // Nachlauf     833Hz
  {1,  1,    1250},  // Fehlerfall   1250Hz
  {1,  20,   625}   // Stufe später runter  625Hz
};

uint8_t  beep_value    =1;  // Index
bool     change_flag   =false;  // ein anderer Beep wurde verlangt

unsigned long nextstep; // nächster Statemachine Wechsel
unsigned long now_;     // millis() Wert
unsigned long b=0;      // für beep Timer

uint8_t state    = STEP_WAIT;
bool    old_treppe=false;   // Abfrage Änderung beim Treppenkontakt
bool    beep_on  = false;   // Ein Ton, keine Pause

#ifndef ATTINY 
int     oldstate = -1;  // nur für Ausgabe
#endif

// einen neuen/anderen Beep beginnen
void do_beep(int ton)
{
  beep_value = ton;
  change_flag = true;

#ifndef ATTINY 
  Serial.println("Beep:" + String(beep_value));
#endif
   if (ton==BEEP_OFF)  
   {
      noTone(SOUND);
      digitalWrite(SOUND_INV,LOW);
   }
}

// Haupttimer 50ms
void run_beep(void)
{
  static uint8_t beep_cnt_end =0; // Endzeit des Tons in 50ms Einheiten
  static uint8_t beep_cnt =0;     // Zeit in 50ms seit Beginn des Tons
  // könnte man auch direkt mit ms Werten machen
  
   // Sound wurde nicht geändert
  if (change_flag)   // Sound wurde geändert
  {
    change_flag = false;
    beep_cnt_end=0;
    beep_on = false;
    noTone(SOUND); 
  }

  // Soundmuster spielen            
  beep_cnt++;
  if ( beep_cnt > beep_cnt_end)
  {
    if ( beep_on)
    {
      beep_cnt=0;
      beep_cnt_end = beep_muster[ beep_value].off ;
      beep_on=false;
      noTone(SOUND);  
    }
    else
    {
      beep_cnt=0;
      beep_cnt_end = beep_muster[ beep_value].on ;
      if ( beep_cnt_end!=0)  
      {
        beep_on=true;
        //tone(SOUND, beep_muster[ beep_value].cnt_pitch ,beep_muster[ beep_value].on); // automatisches Abschalten geht nicht ??
        tone(SOUND, beep_muster[ beep_value].cnt_pitch); 
      }
    }
  }
}

void setup()
{  
  pinMode(TREPPE_IN, INPUT_PULLUP);
#ifndef ATTINY  
  pinMode(START, INPUT_PULLUP);

  Serial.begin(115200); // Debug
  Serial.println("\nTreppe\n");
#endif
  pinMode(RELAIS, OUTPUT);
  pinMode(SOUND_INV, OUTPUT);
  pinMode(SOUND, OUTPUT);
  pinMode(TEST, OUTPUT);

  digitalWrite(SOUND,LOW);
  digitalWrite(SOUND_INV,HIGH);
  digitalWrite(RELAIS,LOW);
  digitalWrite(TEST,LOW);

  do_beep(BEEP_OFF);
}

void loop()
{
#ifndef ATTINY        // Testbetrieb mit UNO
  if (!digitalRead(START))  
  {
    state=STEP_WAIT;  // neu beginnen, falls nicht der erste Test
    delay(1000);
    Serial.print(".");
    noTone(SOUND);
    return;   // erst bei Taste anfangen, sonst einfach Abbruch
  }   
#endif

// 2.Pin invers für lauteren Ton
  if (beep_on)  
    digitalWrite(SOUND_INV, !digitalRead(SOUND));
  else
    digitalWrite(SOUND_INV, digitalRead(SOUND));
    
  now_=millis();

  if (now_ >= b)
  {
    run_beep();
    b=now_+50;   //50ms Auflösung Tonlänge
  }

#ifndef ATTINY 
  if (state != oldstate)
  {
    Serial.println("State:" + String(state));
    oldstate=state;
  }
#endif
  
  switch ( state)
  {
    case STEP_WAIT:      // Start
      if ( TREPPE)    // Treppe ist schon oben -> fertig
      {
        state = STEP_END;
        do_beep(BEEP_OFF);
      }
      else
      {     // 2 Sek nur Signal
        state=STEP_SIGNAL;
        nextstep=now_ + TIME_START;
        do_beep(BEEP_SIGNAL);                      
      }
     break;

    case STEP_SIGNAL:      // nur Signal
      if ( nextstep<=now_ )
      {
        state=STEP_TREPPE;
        nextstep=now_ + TIME_RELAIS;
        digitalWrite(RELAIS,HIGH);   // Relais an
        do_beep(BEEP_RELAIS);
      }
     break; 

    case STEP_TREPPE:      // 3Sek Stufe hoch
      if ( TREPPE)      // Treppe ist oben angekommen
      {     // nur Nachlauf                          
        nextstep=now_ + TIME_AFTER;
        state=STEP_AFTER;
        do_beep(BEEP_AFTER);
      }

      if ( nextstep<=now_ )   // nach 3sek ist die Treppe noch nicht oben -> Fehler
      {     // Fehlerfall !!!!!!
        digitalWrite(RELAIS,LOW);  // Relais aus
        nextstep=now_ + TIME_ERROR;// Zeit abgelaufen 10sek Signal
        state = STEP_ERROR;
        do_beep(BEEP_ERROR);
      }     
     break;

    case STEP_AFTER:      // 0,5sek Nachlauf - nach dem Schalter noch etwas weiter, damit sie auch wirklich am Anschlag ist
      if ( nextstep<=now_ )
      {
        state=STEP_END;
        old_treppe = TREPPE;
        digitalWrite(RELAIS,LOW);  // Relais aus
        do_beep(BEEP_OFF);
      }
      break;
      
    case STEP_ERROR:      // Stufe kommt nicht hoch, Motor aus und Signal
      if ( nextstep<=now_ )
      {
        state=STEP_END;
        do_beep(BEEP_OFF);
      }

      if ( TREPPE)      // Treppe ist doch oben angekommen -> kann das passieren ?
      {     // nur Nachlauf                          
        nextstep=now_ + TIME_AFTER;
        state=STEP_AFTER;
        do_beep(BEEP_AFTER);
      }
     break;

    case STEP_END:     // Ende, alles aus und Endlosschleife.
      digitalWrite(RELAIS,LOW);   // Relais aus - nur um sicher zu sein
      if ( old_treppe != TREPPE)
      {             // falls Treppe jetzt wieder runter kommt, nur schwaches Signal
        if (! TREPPE)  do_beep(BEEP_LATER);
        else           do_beep(BEEP_OFF);
        old_treppe = TREPPE;
      }

// Sleep ???      
     break;

    default:
      state = STEP_END;  // for nothing to happen, kein neuerliches Anfangen
      do_beep(BEEP_OFF);
      break;
  } // switch
}

// Ende
