#include <LiquidCrystal.h>

/*******************************************************
Eingabe ppm und wassermenge über LCD-Keypad-shield
********************************************************/

// Verwendete Pins der LCD anzeige
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int lcd_taste = 0;
int adc_taste_input = 0;
#define tasteRECHTS   0
#define tasteHOCH     1
#define tasteRUNTER   2
#define tasteLINKS    3
#define tasteSELECT   4
#define tasteNICHTS   5

unsigned int ziel_ppm = 10;
float wasser = 0.1;

int lese_tasten()
{
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
  lcd.print("ppm:");
  lcd.setCursor(0,1);
  lcd.print("wasser:");

  do{ 
    delay(200);
    lcd.setCursor(8,0);
    lcd.print("   ");
    lcd.setCursor(8,0);
    lcd.print(ziel_ppm);
    
    lcd_taste = lese_tasten();
    switch(lcd_taste)
      { case tasteHOCH:
        {if (ziel_ppm < 200)
          ziel_ppm++; 
        break;
        }
        case tasteRUNTER:
        {if (ziel_ppm > 10)
          ziel_ppm--; 
        break;}   
        } 
    }while(lcd_taste != tasteSELECT);

  do{
    delay(200);
    lcd.setCursor(8,1);
    lcd.print("    ");
    lcd.setCursor(8,1);
    lcd.print(wasser);
    
    lcd_taste = lese_tasten();
    switch(lcd_taste)
      { case tasteHOCH:
        {if (wasser < 1.0)
          wasser = wasser + 0.05 ; 
         break;
         }
        case tasteRUNTER:
        {if (wasser > 0.1)
          wasser = wasser - 0.05; 
         break;}   
      } 
    }while(adc_taste_input != tasteSELECT);


    
}

void setup()
{
 lcd.begin(16, 2);              // start der Lib. mit 2 Zeilen und 16 Zeichen
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("PPM COUNTER");
 lcd.setCursor(0,1); 
 lcd.print("Ver.0.1/1/2017");
 delay(3000);
 lcd.clear();
}
 
void loop()
{
 eingabe_benutzer();

}

