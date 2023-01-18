#include <BlynkSimpleEsp8266.h>
#include<ESP8266WiFi.h>

char ssid[] = "mywifi";
char pass[] = "mywifi123";
char auth[] = "XKlqewldLNYHLlkVAHpkHCLO7U4hA0MQ";

#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
WidgetLCD lcdvirtual(V5);

volatile int flow_frequency;
float volume, debit, debitblynk;
int bataspemakaianair;
float konstantafrekuensi = 7.5;
int nilai;
bool state = false;
int counterBuzzer;

int statePeringatan;
int stateReset;

unsigned char tombolBuzzer = D7;
unsigned char tombolReset = D8;
unsigned char potensiometer = A0;
unsigned char flowsensor = D5;
unsigned char buzzer = D6;

unsigned long currentTime;
unsigned long cloopTime;

IRAM_ATTR void flow()
{
  flow_frequency++;
}

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) 
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  debit = 0.0;
  volume = 0.0;
  
  pinMode(flowsensor, INPUT);
  pinMode(tombolReset, INPUT);
  pinMode(tombolBuzzer, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(potensiometer, INPUT);
  
 
  digitalWrite(tombolReset, HIGH);
  digitalWrite(tombolBuzzer, HIGH);
 
  digitalWrite(flowsensor, HIGH);
  attachInterrupt(digitalPinToInterrupt(flowsensor), flow, RISING);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PURWARUPA MONITORING");
  lcd.setCursor(3,1);
  lcd.print("DAN PERINGATAN");
  lcd.setCursor(0,2);
  lcd.print("JUMLAH PEMAKAIAN AIR");
  lcd.setCursor(0,3);
  lcd.print("PDAM YANG BERLEBIHAN");

  currentTime = millis();
  cloopTime = currentTime;
}

void loop()
{ 
  Serial.println(statePeringatan);
  digitalWrite(buzzer, LOW);

  if(volume >= bataspemakaianair)
  {
    digitalWrite(buzzer, HIGH);   
    if(state == true)
    {
      state = !state;
      Blynk.email("primatara42@gmail.com", "Peringatan Jumlah Pemakaian Air Berlebihan", "Volume Air yang Digunakan Sudah Melewati Batas");
      Blynk.notify("Volume Air yang Digunakan Sudah Melewati Batas");
    }
  }

  else
  {
    state = true;
  }

  statePeringatan = digitalRead(tombolBuzzer);

if(statePeringatan == HIGH)
{
   digitalWrite(buzzer, HIGH);
   counterBuzzer++;
   delay(100);
   digitalWrite(buzzer, LOW);
   delay(100);
}

else if(statePeringatan == LOW)
{
  counterBuzzer = counterBuzzer;
}

  if(counterBuzzer == 1 )  
  {
    digitalWrite(buzzer, LOW);
  }
  
  else if(counterBuzzer == 2)
  {
    counterBuzzer = 0;
    digitalWrite(buzzer, LOW);
  }

  stateReset = digitalRead(tombolReset);
  if(stateReset == HIGH)
  {
    digitalWrite(buzzer, HIGH);
    Blynk.email("primatara42@gmail.com", "Jumlah Volume Air Sebelum Di Reset (Dalam Satuan Liter)", volume);
    volume = 0;
    delay(100);
    digitalWrite(buzzer, LOW);
    delay(100);
  }

  nilai = analogRead(potensiometer);
  bataspemakaianair = map(nilai, 0, 1023, 100, 300);
  delay(10);
  
  currentTime = millis();
  if(currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime;

  Blynk.run();
  Blynk.virtualWrite(V1, volume);
  Blynk.virtualWrite(V2, bataspemakaianair);
  Blynk.virtualWrite(V3, informasiperingatan);
    
    if(flow_frequency != 0)
    {
       debit = (flow_frequency / konstantafrekuensi);
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Debit : ");
       lcd.print(debit);
       lcd.print(" L/M"); 
       Blynk.virtualWrite(V0, debit);
       debit = debit / 60;
       volume = volume + debit;
       lcd.setCursor(0,1);
       lcd.print("Volume : ");
       lcd.print(volume);
       lcd.print(" L");
       flow_frequency = 0;
    }
    
    else
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Debit  : ");
      lcd.print(flow_frequency);
      lcd.print(" L/M"); 
      Blynk.virtualWrite(V0, flow_frequency);
      lcd.setCursor(0,1);
      lcd.print("Volume : ");
      lcd.print(volume);
      lcd.print(" L");
      debit = flow_frequency;
    }
    
    lcd.setCursor(0,2);
    lcd.print("Batas  : ");
    lcd.print(bataspemakaianair);
    lcd.print(" L");
  }
}
