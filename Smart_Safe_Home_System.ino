#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h> 

#define FLAME_PIN 3      
#define GAS_PIN A0       
#define BUZZER_PIN 8     
#define LED_RED 12       
#define LED_BLUE 13      
#define DHTPIN 2         
#define DHTTYPE DHT22    


SoftwareSerial btSerial(10, 11); 

LiquidCrystal_I2C lcd(0x27, 16, 2); 
DHT dht(DHTPIN, DHTTYPE); 

unsigned long previousMillis = 0;
const long interval = 3000; 
bool showPage1 = true;

void setup() {
  btSerial.begin(9600); 
  
  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  dht.begin(); 
  
  lcd.setCursor(0, 0);
  lcd.print("SafeHome System");
  lcd.setCursor(0, 1);
  lcd.print("Bluetooth Ready");
  delay(3000);
  lcd.clear();
}

void loop() {

  Wire.beginTransmission(0x27);
  if (Wire.endTransmission() != 0) { lcd.init(); lcd.backlight(); }

  int gasValue = analogRead(GAS_PIN);      
  int isFlame = digitalRead(FLAME_PIN);    
  float t = dht.readTemperature(); 
  float h = dht.readHumidity(); 

  if (isnan(t)) t = 0;
  if (isnan(h)) h = 0;


  if (gasValue > 115 || isFlame == HIGH || (t > 0 && t > 30)) {
    digitalWrite(LED_RED, HIGH);   
    digitalWrite(LED_BLUE, LOW);   
    digitalWrite(BUZZER_PIN, HIGH);
    
    lcd.setCursor(0, 0);
    if (isFlame == HIGH) {
      lcd.print("FIRE DETECTED!  ");
      btSerial.println("ALERT: FIRE DETECTED!"); 
    } else if (gasValue > 115) {
      lcd.print("Gas Level: "); lcd.print(gasValue); lcd.print(" ");
      btSerial.print("ALERT: Gas Leak! Value: "); btSerial.println(gasValue);
    } else {
      lcd.print("High Temp: "); lcd.print(t, 1); lcd.print("C");
      btSerial.print("ALERT: High Temp: "); btSerial.println(t, 1);
    }

    lcd.setCursor(0, 1);
    lcd.print("!! DANGER !!    ");
    
    delay(200); 
    digitalWrite(BUZZER_PIN, LOW); 
  } 
  
  else {
    digitalWrite(LED_RED, LOW);    
    digitalWrite(LED_BLUE, HIGH);  
    digitalWrite(BUZZER_PIN, LOW);

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      showPage1 = !showPage1; 
      lcd.clear(); 
      
 
      btSerial.print("Status: OK | Temp: "); btSerial.print(t, 1);
      btSerial.print("C | Hum: "); btSerial.print((int)h);
      btSerial.print("% | Gas: "); btSerial.println(gasValue);
    }

    if (showPage1) {
      lcd.setCursor(0, 0);
      lcd.print("Temp: "); lcd.print(t, 1); lcd.print(" C  ");
      lcd.setCursor(0, 1);
      lcd.print("Humid: "); lcd.print((int)h); lcd.print(" %   ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Gas Level: "); lcd.print(gasValue); lcd.print("  ");
      lcd.setCursor(0, 1);
      lcd.print("Flame: NO FIRE  ");
    }
  }
  delay(100); 
}
