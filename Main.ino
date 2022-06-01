#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "time.h"
#include <DHT.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"

#define DHT_SENSOR_PIN  21 // ESP32 pin GIOP21 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// The Email Sending data object contains config and data to send
SMTPData smtpData;

unsigned long previousMillis = 0;
unsigned long interval = 5000;
bool flag=0;
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034

long duration;
float distanceCm;
float temp;
float humd;

#define emailSenderAccount    "caoproject03@gmail.com"
#define emailSenderPassword   "vgflnpucnieenjqy"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[ALERT] Plant Detection"

// Default Recipient Email Address
String inputMessage = "aliamirkhawaja1@gmail.com";
String enableEmailChecked = "checked";
String inputMessage2 = "true";
// Default Threshold Temperature Value
String inputMessage3 = "35.0";
bool emailSent;

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Ali Amir"
#define WIFI_PASSWORD "03355530116"

// Insert Firebase project API Key
#define API_KEY "AIzaSyB2lh3DaS6GU-5CxaNeeXZ0AZfDyQuesg0"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "aliamirkhawaja1@gmail.com"
#define USER_PASSWORD "12345678"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp-firebase-project-cao-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath = "/Temperature";
String humPath = "/Humidity";
String ultraPath = "/Distance";
String timePath = "/Timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// DHT22 sensor
float temperature;
float humidity;
float ultrasonic;

// Timer variables (send new readings every 5 secs)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 5000;


// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
//---------------------------------------------------------------------------------------------------------------------
void setup(){
  Serial.begin(115200);
  pinMode(22,OUTPUT);//Temp < 15'C
  pinMode(23,OUTPUT);//Temp > 35'C
  pinMode(12,OUTPUT);//WiFi NotConnected
  pinMode(13,OUTPUT);//WiFi Connected
  pinMode(14,OUTPUT);//Humidity > 45%
  
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}
//---------------------------------------------------------------------------------------------------------------------
void loop(){

  // Send new readings to database
    unsigned long currentMillis = millis();
   //check to reconnect WiFi
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
  
  if (WiFi.status()== WL_CONNECTED)//checks whether the led is on ,if wifi is connected.
  {
    digitalWrite(13,HIGH);
    digitalWrite(12,LOW);
  }
  else
  {
    digitalWrite(12,HIGH);
    digitalWrite(13,LOW);
  }
  
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    //Getting reading of the ultrasonic sensor
    // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = ((duration) * SOUND_SPEED)/2;
 
    temp = dht_sensor.readTemperature();
    humd = dht_sensor.readHumidity();
    
    if ( temp >= 35 ) // checks whether the temperature is within above range, if yes blink red led
      {
        digitalWrite(23,HIGH);
        digitalWrite(22,LOW);
        String emailMessage = String("Details:  Temperature Above Threshold. Current Temperature: ") + 
                          String(temp) + String("°C  Humidity is ") + String(humd) + String("  Distance: ") + String(distanceCm)+ String("cm.");
        if(sendEmailNotification(emailMessage)) 
            {
                Serial.println(emailMessage);
                emailSent = true;
            }
         else 
            {
              Serial.println("Email failed to send");
            }    
      }
      else 
      {
        digitalWrite(23,LOW); //else turn off red led 
      }
      
     if ( temp <= 15.0) // checks whether the temperature is within below range, if yes blink yellow led
      {
        digitalWrite(23,LOW);
        digitalWrite(22,HIGH);
        String emailMessage = String("Details:\nTemperature Below Threshold. Current Temperature: ") + 
                          String(temp) + String("°C\nHumidity is ") + String(humd) + String("  Distance: ") + String(distanceCm)+ String("cm.");
        if(sendEmailNotification(emailMessage)) 
            {
                Serial.println(emailMessage);
                emailSent = true;
            }
         else 
            {
              Serial.println("Email failed to send");
            }    
      }
      else 
      {
        digitalWrite(22,LOW);  //else turn off yellow led 
      }

  if (  humd < 40 ) // checks whether the plant is needs water or not, if below 40% dry and if above means does not need water
      {
        String emailMessage = String("Details:  Humidity below 40% Threshold. Plant Needs Water. \n Current Temperature: ") + 
                          String(temp) + String("°C  Humidity is ") + String(humd) + String("  Distance: ") + String(distanceCm)+ String("cm.");
        if(sendEmailNotification(emailMessage)) 
            {
                Serial.println(emailMessage);
                emailSent = true;
            }
         else 
            {
              Serial.println("Email failed to send");
            }    
      }

       if (humd < 40)//on board led blinks green if dry
    {
      digitalWrite(14,HIGH);
    }
    else 
    {
       digitalWrite(14,LOW);
    }
    
    if ( distanceCm < 20) // checks whether the distance is less than 20cm
      {
        digitalWrite(23,HIGH);
        digitalWrite(22,LOW);
        String emailMessage = String("Details: Something Is Near The Plant. \nCurrent Temperature: ") + 
                          String(temp) + String("°C  Humidity is ") + String(humd) + String("  Distance: ") + String(distanceCm) + String("cm.");
        if(sendEmailNotification(emailMessage)) 
            {
                Serial.println(emailMessage);
                emailSent = true;
            }
         else 
            {
              Serial.println("Email failed to send");
            }    
      }
     
      
   
    
    json.set(tempPath.c_str(), String(dht_sensor.readTemperature()));
    json.set(humPath.c_str(), String(dht_sensor.readHumidity()));
    json.set(ultraPath.c_str(),String(distanceCm));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}

bool sendEmailNotification(String emailMessage){
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be 
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  return true;
}

//PASSWORD vgflnpucnieenjqy for email 
