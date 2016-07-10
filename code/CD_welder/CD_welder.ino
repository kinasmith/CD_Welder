#define ENCODER_DO_NOT_USE_INTERRUPTS

#include "Encoder.h"
#include "LiquidCrystal.h"
#include "Bounce2.h"

const int UI_SW = 10;
const int FT_SW = 8;
const int LED = 13;
const int CAP_V_INPUT = 0;
const int TRIG = 9;
const float div_z1_val = 11990;
const float div_z2_val = 3227;

//vOut = z2/(z1+z2) * vIn

Encoder myEnc(6, 7);
Bounce UI_SW_input = Bounce();
Bounce FT_SW_input = Bounce();
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int pulse1Time = 100;
int dwellTime = 100;
int pulse2Time = 100;

float capVoltage = 0;

const int numReadings = 10;  // use this value to determine the size of the readings array.
int voltageReadings[numReadings];      // the readings from the analog input
int voltageReadIndex = 0;              // the index of the current reading
float voltageTotal = 0;                  // the running total
float voltageAverage = 0.0;            // the average
float voltageAverageOld = 0.0;          // the old voltage average reading

boolean voltageSteady = false;    //boolean to monitor whether the voltage is fluctuating - a weld can only be performed when the voltage is stable
int weldComplete = 1;

char* statusText[2] = {"Stabalising volts", "*Ready to weld*"};  // text for the screen to indicate readiness for welding
int  statusTextNum = 0;

int displayUpdateInterval = 500;
long lastUpdate = 0;

int positionCounter = 0;
long encoderPosition  = 100;

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(UI_SW, INPUT_PULLUP);
  pinMode(FT_SW, INPUT_PULLUP);
  UI_SW_input.attach(UI_SW);
  UI_SW_input.interval(5);
  FT_SW_input.attach(FT_SW);
  FT_SW_input.interval(5);

  lcd.begin(16, 2);
  lcd.print("Electrocution!");
  delay(1000);
  lcd.cursor();
}

void loop() {
  UI_SW_input.update();
  FT_SW_input.update();
  capVoltage = (analogRead(CAP_V_INPUT) / 1024.0 * 5.0) / (div_z2_val / (div_z1_val + div_z2_val));
  if (UI_SW_input.fell()) {
    positionCounter++;
    positionCounter %= 3;
  }
  if (FT_SW_input.fell()) {
    weldSchedule();
  }
  updateDisplay();
  updateEncoder();
}

void updateDisplay() {
  if (millis() > lastUpdate + displayUpdateInterval) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("p1");
    lcd.setCursor(3, 0);
    lcd.print(pulse1Time);

    lcd.setCursor(0, 1);
    lcd.print("p2");
    lcd.setCursor(3, 1);
    lcd.print(pulse2Time);

    lcd.setCursor(7, 0);
    lcd.print("dw");
    lcd.setCursor(10, 0);
    lcd.print(dwellTime);

    lcd.setCursor(7, 1);
    lcd.print("cV");
    lcd.setCursor(10, 1);
    lcd.print(capVoltage);
    switch (positionCounter) {
      case 0:
        lcd.setCursor(2, 0);
        break;
      case 1:
        lcd.setCursor(2, 1);
        break;
      case 2:
        lcd.setCursor(9, 0);
        break;
    }
    lastUpdate = millis();
  }
}

void updateEncoder() {
  long newPos = myEnc.read();
  if (newPos != encoderPosition) {
    encoderPosition = newPos;
    switch (positionCounter) {
      case 0:
        pulse1Time = encoderPosition / 4;
        break;
      case 1:
        pulse2Time = encoderPosition / 4;
        break;
      case 2:
        dwellTime = encoderPosition / 4;
        break;
    }
  }
}

void weldSchedule() {
  digitalWrite(TRIG, HIGH);
  delay(pulse1Time/10);
  digitalWrite(TRIG, LOW);
  delay(dwellTime/10);
  digitalWrite(TRIG, HIGH);
  delay(pulse2Time/10);
  digitalWrite(TRIG, LOW);
}

