  /*******************************************
 *
 * Name.......:  silverino
 * Description:  Arduino sketch for measure Q (=I*t) for calculating ppm for coloidal silver water (aqua dest.)
 * Hardware:     INA219 current sense modul https://www.adafruit.com/products/904
 * Author.....:  Peter S.
 * Version....:  0.1.2 
 * Date.......:  28.12.16 (Start 22.12.2016)
 * Status.....:  alpha version 
 * Project....:  https://www.facebook.com/groups/kolloidalessilbergeraetetechnik/permalink/1384594958238544/?match=cHBtIHrDpGhsZXI%3D
 * Contact....:  https://www.facebook.com/peter.schmidt.52831
 * License....:  BSD License (license.txt) wie Adafruit
 * Dependencies: Adafruit_INA219 Lib https://github.com/adafruit/Adafruit_INA219
 ********************************************/
 // PetS 22.12.16 INA219 max. Spannung 26V bei High Side Messung (in der Plusleitung) - geht so nicht für KS-Gen. wegen der hohen Spannung
 // deshalb als Shunt in der Masseleitung
 // PetS 28.12.16 INA219 Stromabfrage innerhalb einer Interruptroutine nicht möglich, weil Proz. keine verschachtelten Interupt kann.
 
#include <Wire.h>
#include <Adafruit_INA219.h>

#define TIMER_OFF TCCR1B &= ~(1 << CS12);
#define TIMER_ON TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);  

Adafruit_INA219 ina219;

float Q_gesamt = 0;
float Q_messung = 0;
float current_mA = 0;
float faktor = 0.001118083; // = M / z * F , 107,8782/1*96485
float wasser = 0.1;

char stringbuf[16];

// zur Kontrollmessung der Zeit mittels millis() Funktion
long unsigned int start_zeit;
long unsigned int end_zeit;
long unsigned int i,x;

long unsigned int sek = 0;
long unsigned int intervall = 1; // Jede Sekunde messen

float ppm = 10 ; // Zielppm
float zielmasse = (wasser * 1000 / 1000000) * ppm; // masse = (wassermenge * 1000 (gramm)/ 1000000) * ppm
float masse ;

float masse2ppm(float masse, float wasser){
  return (masse /(wasser * 1000 / 1000000));
  }

void ausgabe_seriell_werte(void){
    dtostrf(masse, 10, 6, stringbuf); 
    Serial.print("Zeit: "); Serial.print(sek); Serial.println(" sek.");   
    Serial.print("Q_messung:     "); Serial.print(Q_messung); Serial.println(" mC");
    Serial.print("Qgesamt: "); Serial.print(Q_gesamt); Serial.println(" mC");
    Serial.print("Erzeugte masse: "); Serial.print(stringbuf); Serial.println(" g");
    x= (int)masse2ppm(masse, wasser);
    Serial.print("akt. PPM: ");Serial.println(x);
    Serial.print(millis());Serial.println(" msek seit Start"); // kontrolle Messzeitpunkt
    Serial.print("loop Durchlaeufe bisher: ");  Serial.println(i);
    Serial.println("");
  }

//**********************************************************
void setup() {
  
  ina219.begin();
  ina219.setCalibration_16V_400mA();
  
  // TIMER 1 for interrupt frequency 1 Hz
  // http://www.physik.uni-regensburg.de/studium/edverg/elfort/C_KURS_Atmel_Programmieren%20htm/Index.htm
  cli(); 
  TCCR1A = 0; //Timer Counter Control Register A - Timer 1
  TCCR1B = 0; //Timer Counter Control Register B - Timer 1
  TCNT1  = 0; //Timer Counter Daten Register - Timer 1
  OCR1A = 62499; //Timer Counter Output Compare Register - Timer 1 
  TCCR1B |= (1 << WGM12); //Waveform Generation Mode auf CTC - OCRA1A 
  TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10); //  CPU-Takt : 256 = 62500
  TIMSK1 |= (1 << OCIE1A); // Output Compare Match Interrupt Enable 
  sei(); 
 
  Serial.begin(115200);
  Serial.println("Strommessung INA219 fuer ppm-Zaehler");
  dtostrf(zielmasse, 10, 6, stringbuf);
  Serial.print("Zielmasse:" );Serial.print(stringbuf);Serial.println(" g");
  Serial.println("************************************");
  Serial.println(" ");
}

// Interruptroutine
ISR(TIMER1_COMPA_vect){
  Q_messung =  current_mA  * intervall;// I * t
  Q_gesamt = Q_gesamt + Q_messung;// Q aufaddiert
  masse = (Q_gesamt/1000) * faktor; // Qgesamt von mC nach C
  sek++;
}

 //********************************************************** 
void loop() {
   start_zeit = millis();
    do{ 
      i++;
      if(!(i%2000))
        ausgabe_seriell_werte();            
      current_mA = ina219.getCurrent_mA();
      }while (masse <= zielmasse);
            
    end_zeit = millis();
    Serial.print("Fertig Gesamtzeit= ");Serial.print(end_zeit - start_zeit);Serial.println(" msek");
    ausgabe_seriell_werte();
    TIMER_OFF
    while(1);//Stop
}
