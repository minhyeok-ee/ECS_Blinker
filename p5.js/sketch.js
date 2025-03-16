let serial;
let ledState = "OFF"; // 기본 LED 상태

function setup() {
    createCanvas(400, 200);

    // p5 SerialPort 설정
    serial = new p5.SerialPort();
    serial.on("list", gotList);  // 포트 목록 받기
    serial.on("data", serialEvent);  // 데이터 수신
    serial.on("open", () => console.log("✅ Serial Port Open"));
    serial.on("error", (err) => console.error("❌ Serial Port Error:", err));

    // 사용 가능한 포트 목록 확인 후 자동 연결
    serial.list();
    serial.open("COM4");  // 아두이노 연결된 포트 (필요 시 수정)
}

function draw() {
    background(30);
    fill(255);
    textSize(20);
    textAlign(CENTER, CENTER);
    text("LED 상태: " + ledState, width / 2, height / 2);
    
    updateTrafficLight();
}

function serialEvent() {
    let data = serial.readLine();
    if (data) {
        data = data.trim();
        console.log("Received Data: ", data);
        if (data.startsWith("LED: ")) {
            ledState = data.split("LED: ")[1];
        }
    }
}

function updateTrafficLight() {
    let redLED = document.querySelector(".led.red");
    let yellowLED = document.querySelector(".led.yellow");
    let greenLED = document.querySelector(".led.green");

    redLED.classList.remove("active");
    yellowLED.classList.remove("active");
    greenLED.classList.remove("active");

    switch (ledState) {
        case "RED":
            redLED.classList.add("active");
            break;
        case "YELLOW":
            yellowLED.classList.add("active");
            break;
        case "GREEN":
            greenLED.classList.add("active");
            break;
        case "ALL_ON":
            redLED.classList.add("active");
            yellowLED.classList.add("active");
            greenLED.classList.add("active");
            break;
        case "OFF":
        case "ALL_OFF":
            break; // 모든 LED 끄기 (기본 상태 유지)
    }
}

function gotList(portList) {
    console.log("Available Serial Ports:", portList);
}
