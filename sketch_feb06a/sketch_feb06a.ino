

#include <SFE_BMP180.h>

#include <Wire.h>

#include <DHT_U.h>
#include <DHT.h>

#include <LiquidCrystal.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11

SFE_BMP180 pressure;
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(12, 13, 8, 9, 10, 11);


enum keys
{ none,
  select,
  left,
  down,
  up,
  right
};

String currentKeyName = "";
keys currentKey;

int workSpaceINT = 0;
int thermalSp = 16, humSp = 30;

float  screenButtonsPinAvrg = 0.0f;

float tempAnalog = 0.0f;

float hum, tempD, heatInd,thAvg;

float loopCircles = 0;

float thermalTimer = 0;
double hgpress;
double pressTemp;

String displayUpStrings[5];
String displayDownStrings[5];
void setup() {
  // put your setup code here, to run once:
  Inititalization();
}

void loop() {

  loopCircles++;
  if (loopCircles >= 2) {
    TemperatureNHumidityRead();
    AnalogueTempC();
    PressureRead();
  }

  GetKey();
  SetStrings();
  PrintStrings(workSpaceINT);
  ManageRelays();

}
void AnalogueTempC()
{
  int val = analogRead(A1);
  float mv = ( val / 1024.0) * 5000;
  float cel = mv / 10;
  tempAnalog = cel;
}
void Inititalization()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  Serial.begin(9600);

  dht.begin();

  pressure.begin();
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
}


void  GetKey()
{
  screenButtonsPinAvrg =  analogRead(A0);

  if (screenButtonsPinAvrg >= 818) ///nothing from4 volts and up
  {
    currentKey = none;
    currentKeyName = "none";
  }
  if (630 <= screenButtonsPinAvrg && screenButtonsPinAvrg < 800)
  {
    currentKey = select;
    currentKeyName = "select";
    currentKey = none;

  }
  if (400 <= screenButtonsPinAvrg && 600 > screenButtonsPinAvrg)
  {
    currentKey = left;
    currentKeyName = "left";
    lcd.clear();
    workSpacesIndex();
    currentKey = none;

  }
  if (204 <= screenButtonsPinAvrg && screenButtonsPinAvrg < 300)
  {
    currentKey = down;
    currentKeyName = "down";
    SetPoints(workSpaceINT);
    currentKey = none;

  }
  if (90 <= screenButtonsPinAvrg && screenButtonsPinAvrg < 150)
  {
    currentKey = up;
    currentKeyName = "up";
    SetPoints(workSpaceINT);
    currentKey = none;

  }
  if (screenButtonsPinAvrg < 61)
  {
    currentKey = right;
    currentKeyName = "right";
    lcd.clear();
    workSpacesIndex();
    currentKey = none;

  }
  delay(250);

}
void workSpacesIndex()
{
  if (currentKey == left && workSpaceINT >= 0)
  {
    workSpaceINT--;
  }
  if (currentKey == right && workSpaceINT <= 4)
  {
    workSpaceINT++;
  }
  if(workSpaceINT<0)
  {
    workSpaceINT=4;
  }
  if (workSpaceINT>4)
  {
    workSpaceINT=0;
  }
}

void TemperatureNHumidityRead() {
  hum = dht.readHumidity();
  tempD = dht.readTemperature();
  heatInd = dht.computeHeatIndex(tempD, hum, false);

  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(tempD)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }



}

void PressureRead()
{ pressure.startTemperature();
  pressure.getTemperature(pressTemp);
  pressure.startPressure(3);
  pressure.getPressure(hgpress, pressTemp);

  //lcd.print(String(hgpress,1)+" "+String(pressTemp,1));
}

void SetStrings()
{
  float tempTh = 0;
  for (int i = 0; i < 16; i++)
  {
    tempTh += (tempAnalog + tempD + pressTemp) / 3;
    delay(2);
  }
  thAvg = tempTh / 16;
  float pressTempf = pressTemp;
  displayUpStrings[0] = String("T:") + String(tempAnalog, 1) + " " + String(tempD, 1) + " " + String(pressTempf, 1);
  displayDownStrings[0] = String("A:") +  String(thAvg, 1) + String(" ID:") + String(heatInd, 1) + String(" C");
  displayUpStrings[1] = String("H:") + String(hum, 1) + "%";
  displayDownStrings[1] = String("P:") + String(hgpress, 1) + " hPa";
  displayUpStrings[2] = "THERMAL SP:";
  displayDownStrings[2] = String(thermalSp) + "C";
  displayUpStrings[3] = "HUMMIDITY SP:";
  displayDownStrings[3] = String(humSp) + "%";
  displayUpStrings[4] = "THERM STATE:"+CurrentState(CheckSpThermal());
  displayDownStrings[4] = "HUMMD STATE:"+CurrentState(CheckSpHum());
}
void PrintStrings(int index)
{
  lcd.setCursor(0, 0);
  lcd.print(displayUpStrings[index]);
  lcd.setCursor(0, 1);
  lcd.print(displayDownStrings[index]);
  Serial.println(displayUpStrings[index]+" "+displayDownStrings[index]);

}
void SetPoints(int workSpaceINDX)
{
  if (workSpaceINDX == 2)
  {

    SetVerticalINDX(thermalSp, 16, 30, workSpaceINDX);
  }
  if (workSpaceINDX == 3)
  {
    SetVerticalINDX( humSp, 25, 80, workSpaceINDX);
  }
}
void  SetVerticalINDX(int value, int minRange, int maxRange,int workSpaceINDX)
{
  if (value < maxRange && currentKey == up)
  {
    if (workSpaceINDX == 2)
    {
      thermalSp++;
    }
    if (workSpaceINDX == 3)
    {
      humSp++;

    }
  }
  if (value > minRange && currentKey == down)
  {
    if (workSpaceINDX == 2)
    {
      thermalSp--;
    }
    if (workSpaceINDX == 3)
    {
      humSp--;

    }
  }
}
bool CheckSpThermal()
{

return ((thermalSp-2)>=thAvg)||(thermalSp>=thAvg);  
}
bool CheckSpHum()
{
return ((humSp+2)<=hum)||(humSp)<=hum;  
}
String CurrentState(bool isItActive)
{
if (isItActive)return "ON ";
else return "OFF";
}
void ActivateRelays(int currentRelay,bool activate)
{
  if (!activate)
  {
    digitalWrite(currentRelay,HIGH);
  }
  else
  {
    digitalWrite(currentRelay,LOW);
  }
}
void ManageRelays()
{
  
ActivateRelays(6,CheckSpThermal());
ActivateRelays(7,CheckSpHum());
}
