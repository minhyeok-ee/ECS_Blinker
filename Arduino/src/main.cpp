// gesture 기능을 제거한 아두이노 코드
#include <Arduino.h>
#include "PinChangeInterrupt.h"
#include <TaskScheduler.h>

// LED 및 버튼 핀 설정
#define LED_RED 11
#define LED_YELLOW 10
#define LED_GREEN 9
#define BTN_1 4
#define BTN_2 3
#define BTN_3 2
#define POT A0

int currentMode = 0;
int timeRed = 2000, timeYellow = 500, timeGreen = 2000;
int brightness = 255;
int stateRed = 0, stateYellow = 0, stateGreen = 0;

Scheduler scheduler;

void deactivateAllTasks();
void updateBrightness();
void processSerialInput();
void runRedLED();
void runYellowLED();
void runGreenLED();
void blinkGreenLED();
void runYellowLED2();
void modeRedOnly();
void modeAllBlink();

Task taskRedLED(10, TASK_FOREVER, &runRedLED, &scheduler, false);
Task taskYellowLED(10, TASK_FOREVER, &runYellowLED, &scheduler, false);
Task taskGreenLED(10, TASK_FOREVER, &runGreenLED, &scheduler, false);
Task taskGreenBlink(10, TASK_FOREVER, &blinkGreenLED, &scheduler, false);
Task taskYellowLED2(10, TASK_FOREVER, &runYellowLED2, &scheduler, false);
Task taskBlinkAllLED(10, TASK_FOREVER, &modeAllBlink, &scheduler, false);
Task taskCheckSerial(100, TASK_FOREVER, &processSerialInput, &scheduler, true);
Task taskModeRedOnly(10, TASK_FOREVER, &modeRedOnly, &scheduler, false);

void onPressBtn1() {
  if (currentMode != 1) {
    currentMode = 1;
    deactivateAllTasks();
    taskModeRedOnly.enable();
  } else {
    currentMode = 0;
    deactivateAllTasks();
    taskRedLED.enable();
  }
}

void onPressBtn2() {
  if (currentMode != 2) {
    currentMode = 2;
    deactivateAllTasks();
    taskBlinkAllLED.enable();
  } else {
    currentMode = 0;
    deactivateAllTasks();
    taskRedLED.enable();
  }
}

void onPressBtn3() {
  currentMode = (currentMode == 3) ? 0 : 3;
  deactivateAllTasks();
  if (currentMode == 0) {
    taskRedLED.enable();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(POT, INPUT);
  attachPCINT(digitalPinToPCINT(BTN_1), onPressBtn1, FALLING);
  attachPCINT(digitalPinToPCINT(BTN_2), onPressBtn2, FALLING);
  attachPCINT(digitalPinToPCINT(BTN_3), onPressBtn3, FALLING);
  taskRedLED.enable();
}

void loop() {
  scheduler.execute();
  updateBrightness();
  Serial.println(String(currentMode) + "," + String(stateRed) + "," + String(stateYellow) + "," + String(stateGreen) + "," + String(brightness)); 
}

void deactivateAllTasks() {
  taskRedLED.disable();
  taskYellowLED.disable();
  taskGreenLED.disable();
  taskGreenBlink.disable();
  taskYellowLED2.disable();
  taskBlinkAllLED.disable();
  taskModeRedOnly.disable();
  analogWrite(LED_RED, 0);
  analogWrite(LED_YELLOW, 0);
  analogWrite(LED_GREEN, 0);
  stateRed = stateYellow = stateGreen = 0;
}

void updateBrightness() {
  int potValue = analogRead(POT);
  brightness = map(potValue, 0, 1023, 0, 255);
}

void processSerialInput() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    int newRedTime, newYellowTime, newGreenTime, newMode;
    if (sscanf(data.c_str(), "%d,%d,%d,%d", &newRedTime, &newYellowTime, &newGreenTime, &newMode) == 4) {
      timeRed = newRedTime;
      timeYellow = newYellowTime;
      timeGreen = newGreenTime;
      if (newMode != currentMode) {
        currentMode = newMode;
        if (currentMode == 1) { deactivateAllTasks(); taskModeRedOnly.enable(); }
        else if (currentMode == 2) { deactivateAllTasks(); taskBlinkAllLED.enable(); }
        else if (currentMode == 3) { deactivateAllTasks(); }
        else if (currentMode == 0) { deactivateAllTasks(); taskRedLED.enable(); }
      }
    }
  }
}

void runRedLED() {
  static unsigned long startTime = 0;
  static bool isOn = false;
  if (currentMode != 0) { isOn = false; return; }
  if (!isOn) {
    analogWrite(LED_RED, brightness);
    stateRed = 1;
    startTime = millis();
    isOn = true;
  } else {
    analogWrite(LED_RED, brightness);
    if (millis() - startTime >= (unsigned long)timeRed) {
      analogWrite(LED_RED, 0);
      stateRed = 0;
      isOn = false;
      taskRedLED.disable();
      taskYellowLED.enable();
    }
  }
}

void runYellowLED() {
  static unsigned long startTime = 0;
  static bool isOn = false;
  if (currentMode != 0) { isOn = false; return; }
  if (!isOn) {
    analogWrite(LED_YELLOW, brightness);
    stateYellow = 1;
    startTime = millis();
    isOn = true;
  } else {
    analogWrite(LED_YELLOW, brightness);
    if (millis() - startTime >= (unsigned long)timeYellow) {
      analogWrite(LED_YELLOW, 0);
      stateYellow = 0;
      isOn = false;
      taskYellowLED.disable();
      taskGreenLED.enable();
    }
  }
}

void runGreenLED() {
  static unsigned long startTime = 0;
  static bool isOn = false;
  if (currentMode != 0) { isOn = false; return; }
  if (!isOn) {
    analogWrite(LED_GREEN, brightness);
    stateGreen = 1;
    startTime = millis();
    isOn = true;
  } else {
    analogWrite(LED_GREEN, brightness);
    if (millis() - startTime >= (unsigned long)timeGreen) {
      analogWrite(LED_GREEN, 0);
      stateGreen = 0;
      isOn = false;
      taskGreenLED.disable();
      taskGreenBlink.enable();
    }
  }
}

void blinkGreenLED() {
  static int toggleCount = 0;
  static unsigned long lastToggleTime = 0;
  const unsigned long interval = 143UL;

  if (currentMode != 0) { toggleCount = 0; return; }

  if (millis() - lastToggleTime >= interval) {
    lastToggleTime = millis();
    if (toggleCount % 2 == 0) {
      analogWrite(LED_GREEN, 0);
      stateGreen = 0;
    } else {
      analogWrite(LED_GREEN, brightness);
      stateGreen = 1;
    }
    toggleCount++;
  }
  if (toggleCount >= 7) {
    analogWrite(LED_GREEN, 0);
    stateGreen = 0;
    toggleCount = 0;
    taskGreenBlink.disable();
    taskYellowLED2.enable();
  }
}

void runYellowLED2() {
  static unsigned long startTime = 0;
  static bool isOn = false;
  if (currentMode != 0) { isOn = false; return; }
  if (!isOn) {
    analogWrite(LED_YELLOW, brightness);
    stateYellow = 1;
    startTime = millis();
    isOn = true;
  } else {
    analogWrite(LED_YELLOW, brightness);
    if (millis() - startTime >= (unsigned long)timeYellow) {
      analogWrite(LED_YELLOW, 0);
      stateYellow = 0;
      isOn = false;
      taskYellowLED2.disable();
      taskRedLED.enable();
    }
  }
}

void modeAllBlink() {
  static unsigned long toggleTimer = 0;
  static bool ledOn = false;

  if (currentMode != 2) { ledOn = false; return; }

  if (millis() - toggleTimer >= 500UL) {
    toggleTimer = millis();
    ledOn = !ledOn;
    if (ledOn) {
      analogWrite(LED_RED, brightness);
      analogWrite(LED_YELLOW, brightness);
      analogWrite(LED_GREEN, brightness);
      stateRed = stateYellow = stateGreen = 1;
    } else {
      analogWrite(LED_RED, 0);
      analogWrite(LED_YELLOW, 0);
      analogWrite(LED_GREEN, 0);
      stateRed = stateYellow = stateGreen = 0;
    }
  } else {
    if (ledOn) {
      analogWrite(LED_RED, brightness);
      analogWrite(LED_YELLOW, brightness);
      analogWrite(LED_GREEN, brightness);
    }
  }
}

void modeRedOnly() {
  if (currentMode == 1) {
    analogWrite(LED_RED, brightness);
    stateRed = 1;
  }
}
