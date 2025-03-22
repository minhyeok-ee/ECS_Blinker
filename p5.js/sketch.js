// sketch.js (제스처 기능 제외 버전)
let serialPort, writer, reader;
let redSlider, yellowSlider, greenSlider;
let currentMode = 0;
let dataBuffer = "";

function setup() {
  noCanvas();

  redSlider = document.getElementById("redSlider");
  yellowSlider = document.getElementById("yellowSlider");
  greenSlider = document.getElementById("greenSlider");

  redSlider.addEventListener("input", updateTimes);
  yellowSlider.addEventListener("input", updateTimes);
  greenSlider.addEventListener("input", updateTimes);

  document
    .getElementById("connectButton")
    .addEventListener("click", connectToArduino);
}

function updateTimes() {
  if (currentMode === 0) {
    document.getElementById("redTime").textContent = redSlider.value;
    document.getElementById("yellowTime").textContent = yellowSlider.value;
    document.getElementById("greenTime").textContent = greenSlider.value;
    sendData();
  }
}

async function connectToArduino() {
  try {
    serialPort = await navigator.serial.requestPort();
    await serialPort.open({ baudRate: 9600 });
    writer = serialPort.writable.getWriter();
    reader = serialPort.readable.getReader();
    console.log("Connected to Arduino!");
    readData();
  } catch (err) {
    console.error("Connection failed:", err);
  }
}

async function sendData() {
  if (writer) {
    let data = `${redSlider.value},${yellowSlider.value},${greenSlider.value},${currentMode}\n`;
    await writer.write(new TextEncoder().encode(data));
    console.log("🔄 데이터 전송:", data);
  }
}

async function readData() {
  while (serialPort.readable) {
    try {
      const { value, done } = await reader.read();
      if (done) break;

      let chunk = new TextDecoder().decode(value);
      dataBuffer += chunk;

      let lines = dataBuffer.split("\n");
      while (lines.length > 1) {
        let line = lines.shift().trim();
        processData(line);
      }
      dataBuffer = lines[0];
    } catch (err) {
      console.error("Read error:", err);
    }
  }
}

function processData(data) {
  console.log("Received Data:", data);
  let parts = data.split(",");
  if (parts.length !== 5) return;

  currentMode = parseInt(parts[0]);
  document.getElementById("modeText").textContent = currentMode;
  document.getElementById("brightnessValue").textContent = parts[4];

  let redState = parseInt(parts[1]);
  let yellowState = parseInt(parts[2]);
  let greenState = parseInt(parts[3]);

  document.getElementById("redLed").classList.toggle("on", redState);
  document.getElementById("yellowLed").classList.toggle("on", yellowState);
  document.getElementById("greenLed").classList.toggle("on", greenState);
}
