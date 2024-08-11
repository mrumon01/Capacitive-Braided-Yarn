#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Protocentral_FDC1004.h>
#define UPPER_BOUND  0X4000                 // max readout capacitance
#define LOWER_BOUND  (-1 * UPPER_BOUND)
#define CHANNEL 0                          // channel to be read
#define MEASURMENT 0                       // measurment channel

int capdac = 0;
char result[100];
float cap=0;
FDC1004 FDC;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 20;



uint8_t broadcastAddress[] = {0x7C, 0xDF, 0xA1, 0x4D, 0xC9, 0xA6};


typedef struct struct_message {
  float a;
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {

  Wire.begin();        //i2c begin
  Serial.begin(115200); // serial baud rate
  startMillis = millis();  //initial start time
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;    
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {

currentMillis = millis();  //get the current time
 if (currentMillis - startMillis >= period)  //test whether the period has elapsed
 {
  FDC.configureMeasurementSingle(MEASURMENT, CHANNEL, capdac);
  FDC.triggerSingleMeasurement(MEASURMENT, FDC1004_100HZ);
  delay(15);
  uint16_t value[2];
  if (! FDC.readMeasurement(MEASURMENT, value))
  {
    int16_t msb = (int16_t) value[0];
    int32_t capacitance = ((int32_t)457) * ((int32_t)msb); //in attofarads
    capacitance /= 1000;   //in femtofarads
    capacitance += ((int32_t)3028) * ((int32_t)capdac);
    cap=(((float)capacitance/1000)),4;

    Serial.println((cap),4);

    if (msb > UPPER_BOUND)               // adjust capdac accordingly
  {
      if (capdac < FDC1004_CAPDAC_MAX)
    capdac++;
    }
  else if (msb < LOWER_BOUND)
  {
      if (capdac > 0)
    capdac--;
    }

  }

  strcpy;
  myData.a = cap;
  
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  startMillis = currentMillis;
 }
  
}
