# ECS Blinker System  
**202100319 장민혁**  

---

[![유튜브 썸네일](https://img.youtube.com/vi/MYyBEV7Se1Y/0.jpg)](https://www.youtube.com/watch?v=MYyBEV7Se1Y)  

---

## 📌 프로젝트 개요  
이번 프로젝트에서는 **신호등의 REAL-TIME SYSTEM을 구축**하고,  
**인터럽트가 발생하면 즉시 동작을 변경할 수 있는 시스템**을 설계하는 활동을 진행했습니다.  

이 시스템은 **세 가지 작동 모드**를 지원하며, **가변저항을 통해 LED 밝기 조절**이 가능합니다.  

---

## ⚡ 회로 구성  

| 구성 요소   | 핀 번호 |
|------------|--------|
| 🔴 빨간 LED | DIGITAL 11 |
| 🟡 노란 LED | DIGITAL 10 |
| 🟢 초록 LED | DIGITAL 9  |
| 🔘 버튼 1   | DIGITAL 2  |
| 🔘 버튼 2   | DIGITAL 3  |
| 🔘 버튼 3   | DIGITAL 4  |
| 🎛 가변저항 | 아날로그 A0 |

---

## 🔧 기능 설명  

### 1️⃣ **기본 신호등 모드**  
- 🚦 **자동 신호등 시스템**이 동작하며, 아래 순서대로 LED가 변경됨.  
  1. **빨간불(RED) → 노란불(YELLOW) → 초록불(GREEN)**
  2. **초록불 깜빡임(GREEN_BLINK) → 노란불(YELLOW) → 빨간불(RED)**  
- 각 상태는 정해진 시간 동안 유지됨.  

### 2️⃣ **모드 변경 기능 (버튼 입력)**  
각 버튼을 누르면 **신호등 모드가 즉시 변경**됨.  

| 버튼 | 동작 |
|------|------|
| 🔘 **버튼 1 (Mode 1)** | **빨간불 고정** 모드 (RED ONLY) |
| 🔘 **버튼 2 (Mode 2)** | **모든 LED 깜빡임** 모드 (ALL BLINK) |
| 🔘 **버튼 3 (Mode 3)** | **모든 LED OFF** 모드 (ALL OFF) |

버튼을 다시 누르면 기본 신호등 모드로 돌아감.

### 3️⃣ **LED 밝기 조절 (가변저항 A0 활용)**  
- 가변저항을 돌리면 **출력 전압 (0~5V) 변화**에 따라 LED 밝기가 조절됨.
- 밝기 값은 **실시간으로 시리얼 모니터에 출력됨**.  

---

## 🛠 코드 설명  

### 🚦 **신호등 상태 (FSM, Finite State Machine)**
```cpp
enum TrafficState { RED_ON, YELLOW_ON, GREEN_ON, GREEN_HOLD, GREEN_BLINK, YELLOW2_ON };
```

FSM을 활용해 자동 신호등 시스템을 구현했으며, 특정 시간이 지나면 상태가 변경됨.

신호등 상태는 다음과 같이 정의됨.

- **RED_ON**: 빨간불 ON
- **YELLOW_ON**: 노란불 ON
- **GREEN_ON**: 초록불 ON
- **GREEN_HOLD**: 초록불 유지
- **GREEN_BLINK**: 초록불 깜빡임
- **YELLOW2_ON**: 노란불 ON

FSM을 통해 신호등의 상태를 주기적으로 업데이트하며, 초록불이 깜빡이는 과정(GREEN_BLINK)에서는 3회 깜빡인 후 노란불(YELLOW2_ON) 상태로 전환됨.

### 🔘 **인터럽트 기반 버튼 입력 처리**
Pin Change Interrupt를 활용하여 버튼 입력을 감지하고, 버튼을 누를 때마다 각 모드가 토글(toggle) 방식으로 변경됨.

- **버튼 1**을 누르면 빨간불 고정 모드가 활성화됨.
- **버튼 2**를 누르면 모든 LED가 깜빡이는 모드가 활성화됨.
- **버튼 3**을 누르면 모든 LED가 꺼지는 모드가 활성화됨.

한 번 더 버튼을 누르면 기본 신호등 모드로 돌아감.

각 버튼은 Falling Edge(버튼을 눌렀을 때)에서 동작하도록 설정됨.

### 🎛 **가변저항을 통한 LED 밝기 조절**
가변저항(A0)의 입력값을 0~1023 범위에서 읽고, 0~5V로 변환하여 LED 밝기를 조절함. 이 값을 이용하여 LED의 PWM 값을 조절하고, 시리얼 모니터를 통해 현재 전압 상태를 출력함.

### 🖥 **시리얼 모니터 출력 예시**
시리얼 모니터에서 현재 LED 상태를 실시간으로 확인 가능!

- LED: RED
- LED: YELLOW
- LED: GREEN
- LED: GREEN_BLINK
- LED: ALL_ON
- LED: ALL_OFF

버튼을 누를 때마다 상태가 즉시 변경됨.

---

## 📌 프로젝트 요약

- ✅ Real-Time System 기반의 신호등 시스템 구축
- ✅ 인터럽트 활용하여 버튼 입력 처리
- ✅ FSM (Finite State Machine) 기반으로 신호등 동작
- ✅ 가변저항을 통한 LED 밝기 조절 기능 추가
- ✅ 시리얼 모니터 출력으로 LED 상태 확인 가능

---

