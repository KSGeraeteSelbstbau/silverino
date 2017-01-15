  /*******************************************
 *
 * Name.......:  silverino
 * Description:  Arduino sketch for counting ppm/Q 
 * Author.....:  Peter S.
 * Version....:  0.4/1/2017 
 * Date.......:  11.01.17 
 * Status.....:  getestet ok 
 * Project....:  https://github.com/ceotjoe/silverino
 * Contact....:  https://www.facebook.com/peter.schmidt.52831
 * License....:  This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 *               To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
 *               Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 *
 ********************************************/
// Überschlagsrechnung M = 1/x mA * 15 * liter * ppm
// PetS 06.01.2017 Programmteile zusammengefügt
// PetS 08.01.2017 Test Audio,Polw, Start/Stop - Polwechselzeit korrigiert auf Min. - delay zwischen den Eingaben eingefügt
// PetS 09.01.2017 Zeitanzeige umgestellt auf hh:mm:ss 
// PetS 09.01.2017 sprintf kann %f nicht korrekt als format. Zeile 2 und teilweise1 komplett neu
// PetS 11.01.2017 pin13 blinken kommt vom optibootloader und kann über LED_START_FLASHES auf null gesetzt werden (bootloader neu!)
// PetS 11.01.2017 Test mit lm317 und 5mA, 0,4l und 50ppm. Ergebnis 1h 00Min 25Sek - perfekt!

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal.h>
 
Adafruit_INA219 ina219;

// Verwendete Pins der LCD anzeige
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Steuerleitungen def.
#define START 11
#define AUDIO 12
#define POLW  13

// Tastendefinition des LCD Keypad shield
#define tasteRECHTS   0
#define tasteHOCH     1
#define tasteRUNTER   2
#define tasteLINKS    3
#define tasteSELECT   4
#define tasteNICHTS   5

// Register des Timer1 zum Start/Stop
#define TIMER_OFF TCCR1B &= ~(1 << CS12);
#define TIMER_ON TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);  

char zeile1 [17]; 

int lcd_taste = 0;
int adc_taste_input = 0;
int stunde, minute, sekunde;

unsigned int polwechselzeit = 1;
boolean polaritaet = LOW;

unsigned int eingabe_ppm = 10;
unsigned int i;
float eingabe_wasser = 0.1;

float Q_gesamt = 0;
float Q_messung = 0;
float current_mA = 0;
float faktor = 0.001118083; // = M / z * F , 107,8782/1*96485
float ppm;
float zielmasse;
float masse;

long unsigned int sek = 0;
long unsigned int intervall = 1; // Jede Sekunde messen
long unsigned int start_mess, stop_mess; // nur für Testzwecke

float masse2ppm(float masse_in, float wasser_in){
  return (masse_in /(wasser_in * 1000 / 1000000));
  }
  
float ppm2masse(unsigned int ppm_in, float wasser_in){
  return (wasser_in * 1000 / 1000000) * ppm_in;
  }

void sek2hhmmss(long int zeit){
  if (zeit > 59){
    minute++;
    sek = 0;}
  if (minute > 59){
    stunde++;
    minute = 0;} 
  if (stunde > 9){
    stunde = 0;} // da einstellig 9 max. 
  }

// Interruptroutine wird jede Sekunde ausgeführt
ISR(TIMER1_COMPA_vect){
  // start_mess = micros();
  Q_messung =  current_mA  * intervall;// I * t
  Q_gesamt = Q_gesamt + Q_messung;// Q aufaddiert
  masse = (Q_gesamt/1000) * faktor; // Qgesamt von mC nach C 
  sek2hhmmss(sek);
    // Ausgabe 1. LCD Zeile
  lcd.setCursor(0,0); 
  sprintf(zeile1,"%01d:%02d:%02d       mA",stunde,minute,sek);
  lcd.print(zeile1);
  lcd.setCursor(9,0); lcd.print(current_mA);
    // Ausgabe 2. LCD Zeile
  ppm = masse2ppm(masse,eingabe_wasser);
  lcd.setCursor(0,1);
  lcd.print(ppm); lcd.setCursor(7,1); lcd.print("ppm");
  lcd.setCursor(11,1); lcd.print(eingabe_wasser);lcd.setCursor(15,1);lcd.print("l");
    // Polaritätswechsel 
  if (!(sek % (polwechselzeit * 60))) //Polaritätswechsel * 60 = n x 1 Min 
  {     
    digitalWrite(POLW, polaritaet);
    polaritaet = !polaritaet;
  }
  sek++;
  // zur Messung der interruptroutine
  // stop_mess = micros();lcd.setCursor(0,1);lcd.print(stop_mess - start_mess);
} 

int lese_tasten(){
 adc_taste_input = analogRead(0);      // liest den Wert des ADC 
 if (adc_taste_input > 1000) return tasteNICHTS; 
 if (adc_taste_input < 50)   return tasteRECHTS;  
 if (adc_taste_input < 250)  return tasteHOCH; 
 if (adc_taste_input < 450)  return tasteRUNTER; 
 if (adc_taste_input < 650)  return tasteLINKS; 
 if (adc_taste_input < 850)  return tasteSELECT;  
 return tasteNICHTS;  
}

void eingabe_benutzer(){
  lcd.setCursor(0,0);
  lcd.print("ppm:            ");
  lcd.setCursor(0,1);
  lcd.print("wasser:        ");

  do{ 
    delay(200);
    lcd.setCursor(8,0);
    lcd.print("     ");
    lcd.setCursor(8,0);
    lcd.print(eingabe_ppm);
    
    lcd_taste = lese_tasten();
    switch(lcd_taste)
      { case tasteHOCH:
        {if (eingabe_ppm < 200)
          eingabe_ppm++; 
        break;
        }
        case tasteRUNTER:
        {if (eingabe_ppm > 10)
          eingabe_ppm--; 
        break;}   
        } 
    }while(lcd_taste != tasteSELECT);
    
  delay(500); // Pause damit Select nict durchläuft
  
  do{
    delay(200);
    lcd.setCursor(8,1);
    lcd.print("     ");
    lcd.setCursor(8,1);
    lcd.print(eingabe_wasser);
    
    lcd_taste = lese_tasten();
    switch(lcd_taste)
      { case tasteHOCH:
        {if (eingabe_wasser < 1.0)
          eingabe_wasser = eingabe_wasser + 0.05 ; 
         break;
         }
        case tasteRUNTER:
        {if (eingabe_wasser > 0.1)
          eingabe_wasser = eingabe_wasser - 0.05; 
         break;}   
      } 
    }while(lcd_taste != tasteSELECT);   
}

//########################## SETUP ############################
void setup()
{
 //Serial.begin(9600);
 // Setup Steuerleitungen
 pinMode(START, OUTPUT);
 digitalWrite(START, LOW); // StartwertLow
 //pinMode(AUDIO,OUTPUT);
 pinMode(POLW, OUTPUT);
 digitalWrite(POLW, LOW); // StartwertLow
  
 // Start LCD mit 2 Zeilen und 16 Zeichen
 lcd.begin(16, 2);              
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("** Silverino **");
 lcd.setCursor(0,1); 
 lcd.print("Ver.0.4/1/2017");
 delay(3000);
 lcd.clear();
 
 // Start Strommodul INA219
 ina219.begin();
 ina219.setCalibration_16V_400mA();
 
 // Setup Timer 1 mit 1Hz
 cli(); 
 TCCR1A = 0; //Timer Counter Control Register A - Timer 1
 TCCR1B = 0; //Timer Counter Control Register B - Timer 1
 TCNT1  = 0; //Timer Counter Daten Register - Timer 1
 OCR1A = 62499; //Timer Counter Output Compare Register - Timer 1 
 TCCR1B |= (1 << WGM12); //Waveform Generation Mode auf CTC - OCRA1A 
 // TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10); //  CPU-Takt : 256 = 62500
 TIMSK1 |= (1 << OCIE1A); // Output Compare Match Interrupt Enable 
 sei(); 
 TIMER_OFF
}



//########################## LOOP ############################ 
void loop(){
  lcd.clear();
  eingabe_benutzer(); // Eingabemenü ppm und wasser
  lcd.clear();
  zielmasse = ppm2masse(eingabe_ppm,eingabe_wasser);
  
  TIMER_ON // Timer 1 starten
  digitalWrite(START, HIGH); // An,Aus Pin 11 auf high
  do{           
      current_mA = ina219.getCurrent_mA(); // Strom messen
        //Serial.println(current_mA);
      }while (masse <= zielmasse); // Solange Gesamtmasse kleiner als Zielmasse, mach weiter
  TIMER_OFF
  digitalWrite(START, LOW); // An,Aus Pin 11 auf low
  digitalWrite(POLW, LOW); // Polwender Aus
  
  // Ton ausgeben - Zielppm erreicht
  for (i = 0; i < 5; i++)
  {
    tone(AUDIO, 1500);
    delay(500);
    noTone(AUDIO);
    delay(500);
  }
  while(1); //STOP - weiter mit reset
}
