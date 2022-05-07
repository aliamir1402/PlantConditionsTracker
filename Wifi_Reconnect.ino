#include <WiFi.h>

// Replace with your network credentials (STATION)
const char* ssid = "Ali Amir";
const char* password = "03355530116";

unsigned long previousMillis = 0;
unsigned long interval = 30000;
bool flag=0;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  initWiFi();
   Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.println("Connected to Wifi");
}

void loop() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  //flag=0;
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.println(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
    flag=1;
    }
    if (WiFi.status()== WL_CONNECTED && flag==1){
        Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.println("Connected to Wifi");
  flag=0;
  }
  
  
}
