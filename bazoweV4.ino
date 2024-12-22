#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <Adafruit_AHTX0.h>

#define I2C_ADDRESS 0x3C
#define RST_PIN -1

#define R1pin 9
#define R2pin 10
#define R3pin 11
#define Diopin 2
#define M13pin 6
#define Wentpin 4

SSD1306AsciiWire oled;

Adafruit_AHTX0 aht1;
Adafruit_AHTX0 aht2;

uint8_t col0 = 0;  // First value column
uint8_t col1 = 0;  // Last value column.
uint8_t rows;      // Rows per line.

int czas=0;   //czas pracy od uruchomienia
unsigned long currentMillis; //czas w ms
float temp, wilgoc; //średnie temp i wilgoci z obu czujników
int PWM=200; //wypełnienie na wentylatory 0 - 255
int PWM1=180;  // wypełnienie dla dwóch grzałek na raz
int obrot=1; 

void setup() {

pinMode(R1pin, OUTPUT);
pinMode(R2pin, OUTPUT);
pinMode(R3pin, OUTPUT);
pinMode(Diopin, OUTPUT);
pinMode(M13pin, OUTPUT);
pinMode(Wentpin, OUTPUT);
digitalWrite(R1pin, LOW);
digitalWrite(R2pin, LOW);
digitalWrite(R3pin, LOW);      
  
  const char* label[] = {"Temp.:", "Wilgoc:", "Czas:"};
  const char* units[] = {"*C", "rH", "h"};  
  
  Serial.begin(9600);
 
  if (! aht1.begin(&Wire, 0, AHTX0_I2CADDR_DEFAULT)) {
    Serial.println("Could not find AHT at 0x38");
    while (1) delay(10);
  }
  if (! aht2.begin(&Wire, 0, AHTX0_I2CADDR_ALTERNATE)) {
    Serial.println("Could not find AHT at 0x39");
    while (1) delay(10);
  }
   Serial.println("AHT10 found at 0x38 and 0x39.");


#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Callibri15);
   oled.setLetterSpacing(2);
   oled.clear();

// Setup form and find longest label.
  for (uint8_t i = 0; i < 3; i++) {
    oled.println(label[i]);
    uint8_t w = oled.strWidth(label[i]);
    col0 = col0 < w ? w : col0; 
  }
// Three pixels after label.
  col0 += 3;
  // Allow two or more pixels after value.
  col1 = col0 + oled.strWidth("99.9") + 2;
  // Line height in rows.
  rows = oled.fontRows();
    // Print units.  
  for ( uint8_t i = 0; i < 3; i++) {
    oled.setCursor(col1 + 1, i*rows);
    oled.print(units[i]);
  }
  delay(3000); 
}
//------------------------------------------------------------------------------
void clearValue(uint8_t row) {
  oled.clear(col0, col1, row, row + rows - 1);
}
//------------------------------------------------------------------------------
void loop() {
currentMillis = millis();
czas=(currentMillis/3600000);
 //////////////
   sensors_event_t humidity1, temp1;
  aht1.getEvent(&humidity1, &temp1);// populate temp and humidity objects with fresh data
Serial.print("Temperature1: "); Serial.print(temp1.temperature); Serial.println(" C");
  Serial.print("Humidity1: "); Serial.print(humidity1.relative_humidity); Serial.println("% rH");
  Serial.println("");
 
  sensors_event_t humidity2, temp2;
  aht2.getEvent(&humidity2, &temp2);// populate temp and humidity objects with fresh data
  Serial.print("Temperature2: "); Serial.print(temp2.temperature); Serial.println(" C");
  Serial.print("Humidity2: "); Serial.print(humidity2.relative_humidity); Serial.println("% rH");
 

///////////////
temp=(temp1.temperature+temp2.temperature)/2;
wilgoc=(humidity1.relative_humidity+humidity2.relative_humidity)/2;
 /////////////
 clearValue(0);
  oled.print(temp, 1);
  clearValue(rows);
  oled.print(wilgoc, 1);
  clearValue(2*rows);
  oled.print(czas);  
//////////////regulacja temperatury (<60) i wilgotności (<10)
if(wilgoc>10){
if(temp<58){
 
if (obrot==2){
analogWrite(R1pin, 0);    

analogWrite(R2pin,PWM1);
analogWrite(R3pin,PWM1);
}
else  if (obrot==3){
analogWrite(R2pin, 0);    

analogWrite(R3pin,PWM1);
analogWrite(R1pin,PWM1);
}
 else 
 {
obrot=1;
analogWrite(R3pin, 0);    

analogWrite(R1pin,PWM1);
analogWrite(R2pin,PWM1);
 
}
 delay(500);
 obrot=obrot+1;
}
else
{
digitalWrite(R1pin, LOW); 
digitalWrite(R2pin, LOW); 
digitalWrite(R3pin, LOW); 
}
}
else
{
digitalWrite(R1pin, LOW); 
digitalWrite(R2pin, LOW); 
digitalWrite(R3pin, LOW); 
}
/////////////
analogWrite(M13pin, PWM);
/////////////wentylacja równająca wilgotność
if(currentMillis>21600 && currentMillis<22500)
{
  digitalWrite(Wentpin, HIGH);
}
else if(currentMillis>22500 && currentMillis<43200)
{
  digitalWrite(Wentpin, LOW);
}
else if(currentMillis>43200 && currentMillis<44100)
{
  digitalWrite(Wentpin, HIGH);
}
else if(currentMillis>44100 && currentMillis<86400)
{
  digitalWrite(Wentpin, LOW);
}
else if(currentMillis>86400 && currentMillis<87300)
{
  digitalWrite(Wentpin, HIGH);
}
else
{
  digitalWrite(Wentpin, LOW);
}
///////////////////////// wizualny przypominacz o działaniu
if(czas>6 && czas<12){
analogWrite(Diopin, 64);
}
else if(czas>12 && czas<24){
  analogWrite(Diopin, 128);
}
else if(czas>24){
  analogWrite(Diopin, 255);
}

///////////////////////// bezpiecznik czasowy dla debili ze skleroza
if(czas>48)
{
digitalWrite(R1pin, LOW); 
digitalWrite(R2pin, LOW); 
digitalWrite(R3pin, LOW); 
analogWrite(M13pin, 0);
digitalWrite(Wentpin, LOW);
oled.clear();
  for(;;); // Don't proceed, loop forever
}

delay(200);  
}
