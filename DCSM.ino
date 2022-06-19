#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <SPI.h>

#define DHT_SENSOR_PIN  21 // ESP32 pin GIOP21 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22
#define pinAir 33 //Pin Sensor Water Level

int nilai = 0; 
int pinSensor = 5; 
int ledmerah = 15;
int ledhijau = 2;
int buzzer = 4;

int nilaiApi = 0;

int kondisi = 0;


const char* ssid = "[yout wifi name]";
const char* password = "[wifi password]";
const char* host = "[local database ip]";

#define BOTtoken "[bot token from BOT Father Telegram]"
#define CHAT_ID "[chat id / receiver]"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

void setup() {
  Serial.begin(9600);

  pinMode(pinSensor, INPUT);
  pinMode(ledhijau, OUTPUT);
  pinMode(ledmerah, OUTPUT);
  pinMode(buzzer, OUTPUT);

  dht_sensor.begin(); // initialize the DHT sensor

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Ayang cantik", "");

}

void loop() {
  float humi = dht_sensor.readHumidity();
  float tempC = dht_sensor.readTemperature();
  float tempF = dht_sensor.readTemperature(true);

  //fire
  int state = digitalRead(pinSensor);

  //water
  float ketinggian = bacaSensor();
  Serial.print("Humidity: ");
  Serial.print(humi);
  Serial.print("%");

  Serial.print("  |  ");

  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.print("°C ");
  Serial.print("Ketinggian Air :" );
  Serial.println(ketinggian);

  if (tempC > 32) {
    Serial.println("TERDETEKSI SUHU TINGGI");
    digitalWrite(ledmerah, HIGH);
    digitalWrite(ledhijau, LOW);
    digitalWrite(buzzer, HIGH);
    String report = "";
    report += "TERDETEKSI SUHU TINGGI";
    report += String(tempC) + " °C";
    bot.sendMessage(CHAT_ID, report, "");
    kondisi = 1;
  }
  else if (ketinggian > 50) {
    Serial.println("TERDETEKSI AIR");
    digitalWrite(ledmerah, HIGH);
    digitalWrite(ledhijau, LOW);
    digitalWrite(buzzer, HIGH);
    String report = "";
    report += "TERDETEKSI AIR ";
    report += String(ketinggian) + " cm";
    bot.sendMessage(CHAT_ID, report, "");
    kondisi = 1;
  }
  else if (state == LOW) {
    Serial.println("TERDETEKSI API");
    digitalWrite(ledmerah, HIGH);
    digitalWrite(ledhijau, LOW);
    digitalWrite(buzzer, HIGH);
    nilaiApi = 1;
    String report = "";
    report += "TERDETEKSI API";
    bot.sendMessage(CHAT_ID, report, "");
    kondisi = 1;
  }
  else {
    Serial.println("KONDISI AMAN");
    digitalWrite(ledhijau, HIGH);
    digitalWrite(ledmerah, LOW);
    digitalWrite(buzzer, LOW);
    nilaiApi = 0;
    kondisi = 0;
  }
      Serial.print("connecting to ");
      Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
    return;
  }

  client.print(String("GET http://localhost/iot_project/connect4.php?") +
               ("&temperature=") + tempC +
               ("&humidity=") + humi +
               ("&api=") + nilaiApi +
               ("&waterlevel=") + ketinggian +
               ("&kondisi=") + kondisi +
               " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
                  Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);

  }

  Serial.println();
  Serial.println("closing connection");

  // wait a 2 seconds between readings
  delay(2000);
}
int bacaSensor() {
  nilai = analogRead(pinAir);
  return nilai;
}
