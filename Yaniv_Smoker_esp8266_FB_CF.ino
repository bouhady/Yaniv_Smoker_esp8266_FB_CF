
#include "SSD1306Wire.h"
#include <ESP8266WiFi.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "images.h"

#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti wifiMulti;


SSD1306Wire  display(0x3c, D3, D5);

int demoMode = 0;
int counter = 1;
int adcval = 0;
float adc = 0;

float temp_celsius;

int dataArray[100];
int currentPointIndex = 0;


int i = 0;

int sensorPin = A0;    // select the input pin for the potentiometer

int sensorValue = 0;  // variable to store the value coming from the sensor


void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Hello");
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(Dialog_plain_7);

  initGraph();

  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

}

void initGraph() {
  display.setColor(WHITE);
  for (int i = 0; i < 100 ; i++)
    dataArray[i] = 0;
}

// the loop function runs over and over again forev


void loop() {

  Serial.println("Hello loop");
  // clear the display
  display.clear();

  display.drawHorizontalLine(10, 54, 100);
  display.drawVerticalLine(10, 5, 50);

  //  adcval = analogRead(ANALOG_PIN_0);
  adcval = analogRead(sensorPin);
  adc = adcval - 14 ;
  
  float lanVal = ((1024/adc)-1)/10 ;
  Serial.println("lanVal : " + String(lanVal) );
  float lanItem = 0.000253*log(lanVal) ;
  Serial.println("lanItem : " + String(lanItem) );
  float temp = 1/(0.00335 + lanItem) - 270 ;
  temp_celsius = (temp > 0) ? temp : 0 ;

  if (counter > 3) {
    updateDataToCloud(adcval, temp_celsius);
    counter = 0;
  }

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 54, "Temperture :" + String(temp_celsius));
  display.drawString(64, 0, "adc :" + String(adcval));

  updateGraph(temp_celsius / 4);
  display.display();
  counter++;
  delay(1000);
}

void updateGraph(int val) {
  dataArray[currentPointIndex] = val ;

  Serial.println("val : " + String(val) + " currentPoint : " + String(55 - dataArray[currentPointIndex]));

  for (int i = 0 ; i < 100 ; i++) {
    int x = i;
    int y = 0;
    if (100 - i < currentPointIndex) {
      y = dataArray[currentPointIndex - (100 - i)] ;
    } else {
      y = dataArray[currentPointIndex + i];
    }
    display.setPixel(x + 10, 54 - y );
  }
  currentPointIndex++;
  if (currentPointIndex >= 99) {
    currentPointIndex = 0;
  }

}

void updateDataToCloud(int data, float temperture) {
  if ((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    Serial.println("adc : " + String(data));
    http.begin("http://us-central1-smokingcontroller.cloudfunctions.net/tempUpdate?temperture1=" + String(temperture) + "&adc=" + String(data)); //HTTP
//    http.begin("http://us-central1-smokingcontroller.cloudfunctions.net/adcUpdate?adc=" + String(data)); //HTTP
    int httpCode = http.GET();
    http.end();
  }
}

