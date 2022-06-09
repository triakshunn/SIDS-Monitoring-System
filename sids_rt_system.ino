#include <Arduino.h>
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Temperature_LM75_Derived.h>
#include "Wire.h"
#include <MPU6050_light.h>
#include <OneWire.h>
#include <DallasTemperature.h>
const int oneWireBus = 4;     
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
MPU6050 mpu(Wire);
Generic_LM75 temperature;
unsigned long timer = 0;
const int PulseWire = 2;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 2;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 1000;
 int heartbeatStatus=0;  
PulseSensorPlayground pulseSensor;  
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "A_B_C_D"
#define WIFI_PASSWORD "lajpatnagar123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAwmpEvkajz_WAFSiJ2EMGMGdHZdEFMACY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://gyro-sensing-esp32-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void setup(){
  Serial.begin(115200);
  Wire.begin();
 byte status = mpu.begin();
 Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");
  Serial.println("Communicating with the temperature sensor via 1-wire protocol. \nFetching data...");
  sensors.begin(); //Begin talking to the sensor
  
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void loop() {
  
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

 sensors.requestTemperatures(); //Request temperatures from the sensor
  mpu.update();

Serial.print("Temp: "); //print "Temp: "
Serial.print(temperature.readTemperatureC()); //populate the temperature reading.
Serial.println("c"); //print "Temp: "
 if (digitalRead(2)==1){
  heartbeatStatus=1;
  Serial.println("♥  A HeartBeat Happened ! "); 
 }
                    // considered best practice in a simple sketch.
  
//Serial.print("Heart Beat: ");
//Serial.println(temp);
  Serial.print("X : ");
  Serial.print(mpu.getAngleX());
  Serial.print("\tY : ");
  Serial.print(mpu.getAngleY());
  Serial.print("\tZ : ");
  Serial.println(mpu.getAngleZ());
    if (Firebase.RTDB.pushFloat(&fbdo, "test/Y_angle",mpu.getAngleY())){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.pushFloat(&fbdo, "test/Temp",temperature.readTemperatureC())){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
     else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.pushFloat(&fbdo, "test/Heart",heartbeatStatus)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
     else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
   heartbeatStatus=0; 
 //  delay(3000);
  }
}
