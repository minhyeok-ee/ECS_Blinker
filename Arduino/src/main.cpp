#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

// -------------------------
// 핀 정의
// -------------------------
#define LED_RED_PIN    11
#define LED_YELLOW_PIN 10
#define LED_GREEN_PIN  9
#define POTENTIOMETER  A0  // 가변저항 핀 정의

#define BTN_MODE1      4
#define BTN_MODE2      3
#define BTN_MODE3      2

// -------------------------
// 작업 주기 설정 (밀리초)
// -------------------------
#define STATE_UPDATE_INTERVAL 10

Scheduler scheduler;

// -------------------------
// LED 상태 및 신호등 FSM 설정
// -------------------------
enum LEDState { OFF, RED, YELLOW, GREEN };
enum TrafficState { RED_ON, YELLOW_ON, GREEN_ON, GREEN_HOLD, GREEN_BLINK, YELLOW2_ON };

volatile LEDState currentLED = RED;
volatile TrafficState trafficState = RED_ON;
volatile bool isMode1 = false, isMode2 = false, isMode3 = false;
volatile bool isRedOnlyMode = false; // 모드 1
volatile bool isAllBlinkMode = false; // 모드 2
volatile bool isAllOffMode = false; // 모드 3

unsigned long stateTimer = 0;
unsigned int durationRed = 500, durationYellow = 2000, durationGreen = 500, durationGreenHold = 2000, flickerInterval = 500;
int flickerCount = 0;
volatile float outputVoltage = 5.0; // 출력 전압 변수 (0~5V)

// 🔹 **p5.js와 연동하기 위한 LED 상태 전송 함수 추가**
void sendLEDStateToSerial() {
    String state;
    if (isAllBlinkMode) state = "LED: ALL_ON";
    else if (isAllOffMode) state = "LED: ALL_OFF";
    else if (isRedOnlyMode) state = "LED: RED";
    else if (currentLED == RED) state = "LED: RED";
    else if (currentLED == YELLOW) state = "LED: YELLOW";
    else if (currentLED == GREEN) state = "LED: GREEN";
    else state = "LED: OFF";

    Serial.println(state);
}

// 🔹 **LED를 설정한 후 시리얼 데이터 전송**
void setLED(int r, int y, int g) {
    float redVoltage = (r / 255.0) * outputVoltage;
    float yellowVoltage = (y / 255.0) * outputVoltage;
    float greenVoltage = (g / 255.0) * outputVoltage;
    
    analogWrite(LED_RED_PIN, (redVoltage / 5.0) * 255);
    analogWrite(LED_YELLOW_PIN, (yellowVoltage / 5.0) * 255);
    analogWrite(LED_GREEN_PIN, (greenVoltage / 5.0) * 255);

    sendLEDStateToSerial(); // LED 변경될 때마다 상태 전송
}

void renderLED() {
    int rawBrightness = analogRead(POTENTIOMETER);
    outputVoltage = map(rawBrightness, 0, 1023, 0, 500) / 100.0; // 0~5V 변환

    if (isAllBlinkMode) { // Mode 2: 모든 LED 깜빡임
        static unsigned long lastToggleTime = millis();
        static bool isOn = false;
        if (millis() - lastToggleTime >= 500) { 
            isOn = !isOn;
            lastToggleTime = millis();
        }
        if (isOn) {
            setLED(255, 255, 255);
        } else {
            setLED(0, 0, 0);
        }
        return;
    }

    if (isRedOnlyMode) { // Mode 1: Red LED 켜짐
        setLED(255, 0, 0);
        return;
    }

    if (isAllOffMode) { // Mode 3: 모든 LED OFF
        setLED(0, 0, 0);
        return;
    }

    // 기본 모드 (신호등)
    switch (currentLED) {
        case RED: setLED(255, 0, 0); break;
        case YELLOW: setLED(0, 255, 0); break;
        case GREEN: setLED(0, 0, 255); break;
        default: setLED(0, 0, 0); break;
    }
}

// 기본 신호등 모드 업데이트
void updateTrafficState() {
    if (isRedOnlyMode || isAllBlinkMode || isAllOffMode) return; // 다른 모드가 활성화되었으면 기본 모드 동작 중지

    unsigned long now = millis();
    if (now - stateTimer < ((trafficState == RED_ON) ? durationRed : 
                             (trafficState == GREEN_ON) ? durationGreen : 
                             (trafficState == GREEN_HOLD) ? durationGreenHold : 
                             (trafficState == GREEN_BLINK) ? flickerInterval : durationYellow)) return;
    stateTimer = now;

    switch (trafficState) {
        case RED_ON: 
            currentLED = RED; 
            trafficState = YELLOW_ON; 
            break;
        case YELLOW_ON: 
            currentLED = YELLOW; 
            trafficState = GREEN_ON; 
            break;
        case GREEN_ON: 
            currentLED = GREEN; 
            trafficState = GREEN_HOLD; 
            break;
        case GREEN_HOLD:
            currentLED = GREEN;
            trafficState = GREEN_BLINK;
            flickerCount = 0;
            break;
        case GREEN_BLINK:
            currentLED = (currentLED == GREEN) ? OFF : GREEN;
            if (++flickerCount >= 6) { // ON/OFF 총 6회 (3회 깜빡임)
                trafficState = YELLOW2_ON;
            }
            break;
        case YELLOW2_ON: 
            currentLED = YELLOW; 
            trafficState = RED_ON; 
            break;
    }
    sendLEDStateToSerial(); // 상태 변경될 때마다 시리얼 전송
}

// 버튼 핸들러 (토글 방식)
void buttonHandler1() {
    isRedOnlyMode = !isRedOnlyMode;
    sendLEDStateToSerial();
}

void buttonHandler2() {
    isAllBlinkMode = !isAllBlinkMode;
    sendLEDStateToSerial();
}

void buttonHandler3() {
    isAllOffMode = !isAllOffMode;
    sendLEDStateToSerial();
}

Task taskUpdateState(STATE_UPDATE_INTERVAL, TASK_FOREVER, updateTrafficState);

void setup() {
    Serial.begin(9600);
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(POTENTIOMETER, INPUT);
    pinMode(BTN_MODE1, INPUT_PULLUP);
    pinMode(BTN_MODE2, INPUT_PULLUP);
    pinMode(BTN_MODE3, INPUT_PULLUP);
    attachPCINT(digitalPinToPCINT(BTN_MODE1), buttonHandler1, FALLING);
    attachPCINT(digitalPinToPCINT(BTN_MODE2), buttonHandler2, FALLING);
    attachPCINT(digitalPinToPCINT(BTN_MODE3), buttonHandler3, FALLING);
    scheduler.init();
    scheduler.addTask(taskUpdateState);
    taskUpdateState.enable();
}

void loop() {
    renderLED();
    scheduler.execute();
}
