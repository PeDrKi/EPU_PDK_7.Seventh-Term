#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
// ---------- WiFi ----------
const char *ssid = "Hoaisuonq28";
const char *password = "hoaisuong28";

// ---------- LCD I2C ----------
#define I2C_SDA 21
#define I2C_SCL 22
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// ---------- LED ----------
#define LED_OK_GR 2
#define LED_NOTOK_RED 15

// ---------- Buzzer ----------
#define BUZZER 19

// ---------- DHT11 ----------
#define DHT_DATA 4
#define DHTTYPE DHT11
DHT dht(DHT_DATA, DHTTYPE);

// ---------- MQ9 ----------
#define MQ9_A0 34
#define MQ9_D0 32

// ---------- Ngưỡng ----------
#define TEMP_THRESHOLD 50.0
#define MQ9_THRESHOLD 400.0

// Web server
WebServer server(80);

// Biến lưu trạng thái sensor
float t = 0, h = 0, mq9_ppm = 0;
bool danger = false;

// Hàm đọc cảm biến
void readSensors()
{
  h = dht.readHumidity();
  t = dht.readTemperature();

  // Đọc MQ9 (trung bình 10 lần)
  long sum = 0;
  for (int i = 0; i < 10; i++)
  {
    sum += analogRead(MQ9_A0);
    delay(5);
  }
  mq9_ppm = (sum / 10.0) / 4095.0 * 1000.0;

  // Xác định nguy hiểm
  danger = (t >= TEMP_THRESHOLD || mq9_ppm >= MQ9_THRESHOLD);
}

// Cập nhật LCD & LED
void updateLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t, 1);
  lcd.print((char)223);
  lcd.print("C H:");
  lcd.print(h, 1);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("G:");
  lcd.print(mq9_ppm, 0);
  lcd.print("ppm ");

  if (danger)
  {
    lcd.print("ALM");
    digitalWrite(LED_OK_GR, LOW);
    digitalWrite(LED_NOTOK_RED, HIGH);
    digitalWrite(BUZZER, HIGH);
  }
  else
  {
    lcd.print("SAFE");
    digitalWrite(LED_OK_GR, HIGH);
    digitalWrite(LED_NOTOK_RED, LOW);
    digitalWrite(BUZZER, LOW);
  }
}

//////////////////////////////////////////////////////////
// Web routes
const char* apiKey = "F2Y7FEDC1Y4UVHBA";  
void sendToThingSpeak()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "https://api.thingspeak.com/update?api_key=" + String(apiKey) +
                 "&field1=" + String(t, 1) +
                 "&field2=" + String(h, 1) +
                 "&field3=" + String(mq9_ppm, 1);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      Serial.println("Data sent to ThingSpeak!");
    }
    else
    {
      Serial.println("Failed to send data!");
    }
    http.end();
  }
}

void handleRoot()
{
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <title>🔥 ESP32 Fire Monitoring Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
      body {
        font-family: 'Segoe UI', Arial, sans-serif;
        background: linear-gradient(135deg, #dfe9f3 0%, #ffffff 100%);
        margin: 0;
        padding: 0;
        display: flex;
        flex-direction: column;
        align-items: center;
        min-height: 100vh;
      }
      header {
        width: 100%;
        background: linear-gradient(90deg, #0045ff, #0099ff);
        color: white;
        text-align: center;
        padding: 18px 0;
        font-size: 26px;
        letter-spacing: 1px;
        box-shadow: 0 4px 10px rgba(0,0,0,0.15);
      }
      main {
        margin-top: 25px;
        background: #fff;
        border-radius: 15px;
        box-shadow: 0 6px 20px rgba(0,0,0,0.1);
        padding: 25px 40px;
        max-width: 1250px;
        width: 90%;
      }
      .stats {
        display: flex;
        justify-content: space-around;
        flex-wrap: wrap;
        gap: 20px;
        margin-bottom: 25px;
      }
      .card {
        flex: 1 1 30%;
        background: linear-gradient(145deg, #f9f9f9, #ffffff);
        border-radius: 12px;
        box-shadow: 0 3px 10px rgba(0,0,0,0.1);
        padding: 20px;
        text-align: center;
        transition: 0.3s;
      }
      .card:hover {
        transform: translateY(-5px);
        box-shadow: 0 6px 18px rgba(0,0,0,0.15);
      }
      .card h2 {
        margin: 10px 0;
        font-size: 20px;
        color: #333;
      }
      .value {
        font-size: 36px;
        font-weight: bold;
      }
      .unit {
        font-size: 18px;
        color: #666;
      }
      #alert {
        text-align: center;
        margin: 20px auto;
        font-size: 22px;
        font-weight: bold;
        border-radius: 10px;
        padding: 15px;
        width: 80%;
        transition: all 0.4s ease;
      }
      #alert.safe {
        background: #d4edda;
        color: #155724;
        border: 2px solid #c3e6cb;
      }
      #alert.danger {
        background: #f8d7da;
        color: #721c24;
        border: 2px solid #f5c6cb;
        animation: blink 1s infinite alternate;
      }
      @keyframes blink {
        from {opacity: 1;}
        to {opacity: 0.5;}
      }
      .chart-row {
        display: flex;
        flex-wrap: wrap;
        justify-content: space-around;
        gap: 25px;
      }
      .chart-box {
        flex: 1 1 30%;
        background: #fafafa;
        border-radius: 12px;
        box-shadow: 0 3px 12px rgba(0,0,0,0.08);
        padding: 15px;
        min-width: 300px;
      }
      .chart-box h3 {
        text-align: center;
        margin-bottom: 10px;
      }
      footer {
        margin: 20px;
        font-size: 14px;
        color: #777;
      }
      @media (max-width: 900px) {
        .chart-row { flex-direction: column; align-items: center; }
        .chart-box { width: 90%; }
        .card { width: 90%; }
      }
    </style>
  </head>
  <body>
    <header>🔥 ESP32 Fire Monitoring Dashboard</header>
    <main>
      <div class="stats">
        <div class="card">
          <h2>🌡️ Temperature</h2>
          <div class="value" id="temp">--</div>
          <div class="unit">°C</div>
        </div>
        <div class="card">
          <h2>💧 Humidity</h2>
          <div class="value" id="humi">--</div>
          <div class="unit">%</div>
        </div>
        <div class="card">
          <h2>🧪 Gas Level</h2>
          <div class="value" id="gas">--</div>
          <div class="unit">ppm</div>
        </div>
      </div>

      <div id="alert" class="safe">✅ System Safe</div>

      <div class="chart-row">
        <div class="chart-box">
          <h3>🌡️ Temperature</h3>
          <canvas id="chartTemp"></canvas>
        </div>
        <div class="chart-box">
          <h3>💧 Humidity</h3>
          <canvas id="chartHumi"></canvas>
        </div>
        <div class="chart-box">
          <h3>🧪 Gas Concentration</h3>
          <canvas id="chartGas"></canvas>
        </div>
      </div>
    </main>

    <footer>© 2025 Fire Monitor | ESP32 + DHT11 + MQ9</footer>

    <script>
      const ctxT = document.getElementById('chartTemp').getContext('2d');
      const ctxH = document.getElementById('chartHumi').getContext('2d');
      const ctxG = document.getElementById('chartGas').getContext('2d');

      const chartTemp = new Chart(ctxT, {
        type: 'line',
        data: { labels: [], datasets: [{ label: 'Temp (°C)', borderColor: '#ff4d4d', backgroundColor: 'rgba(255,77,77,0.1)', data: [], fill: true, tension: 0.3 }] },
        options: { scales: { y: { beginAtZero: true } } }
      });
      const chartHumi = new Chart(ctxH, {
        type: 'line',
        data: { labels: [], datasets: [{ label: 'Humidity (%)', borderColor: '#0099ff', backgroundColor: 'rgba(0,153,255,0.1)', data: [], fill: true, tension: 0.3 }] },
        options: { scales: { y: { beginAtZero: true } } }
      });
      const chartGas = new Chart(ctxG, {
        type: 'line',
        data: { labels: [], datasets: [{ label: 'Gas (ppm)', borderColor: '#00cc66', backgroundColor: 'rgba(0,204,102,0.1)', data: [], fill: true, tension: 0.3 }] },
        options: { scales: { y: { beginAtZero: true } } }
      });

      let counter = 0;
      async function updateData() {
        const res = await fetch("/data.json");
        const data = await res.json();

        document.getElementById('temp').innerText = data.temperature.toFixed(1);
        document.getElementById('humi').innerText = data.humidity.toFixed(1);
        document.getElementById('gas').innerText = data.mq9.toFixed(1);

        chartTemp.data.labels.push(counter);
        chartHumi.data.labels.push(counter);
        chartGas.data.labels.push(counter);

        chartTemp.data.datasets[0].data.push(data.temperature);
        chartHumi.data.datasets[0].data.push(data.humidity);
        chartGas.data.datasets[0].data.push(data.mq9);

        [chartTemp, chartHumi, chartGas].forEach(chart => {
          if (chart.data.labels.length > 20) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
          }
          chart.update();
        });

        counter++;
        const alertBox = document.getElementById('alert');
        if (data.danger) {
          alertBox.innerText = "🚨 WARNING: Fire Risk Detected!";
          alertBox.className = "danger";
        } else {
          alertBox.innerText = "✅ System Safe";
          alertBox.className = "safe";
        }
      }
      setInterval(updateData, 2000);
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}


void handleDataJson()
{
  String data = "{";
  data += "\"temperature\":" + String(t, 1) + ",";
  data += "\"humidity\":" + String(h, 1) + ",";
  data += "\"mq9\":" + String(mq9_ppm, 1) + ",";
  data += "\"danger\":" + String(danger ? "true" : "false");
  data += "}";
  server.send(200, "application/json", data);
}

//////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_OK_GR, OUTPUT);
  pinMode(LED_NOTOK_RED, OUTPUT);
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED_OK_GR, LOW);
  digitalWrite(LED_NOTOK_RED, LOW);

  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();

  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("ESP32 Project");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data.json", handleDataJson);
  server.begin();
}

void loop()
{
  readSensors();
  updateLCD();
  server.handleClient();
  sendToThingSpeak();
  delay(2000);
}
