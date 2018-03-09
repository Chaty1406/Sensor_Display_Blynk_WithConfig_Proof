/**
 * DHT22 + SH1106 test program
 * Pin connection scheme
 * SH1106 OLED
 *  3V3 -> VCC
 *  D3 -> SDA
 *  D5 -> SCL
 * DHT22
 *  5V -> VCC
 *  D2 -> DOUT
 * All powered by 3V3
 */

#include "DHT.h"
#include <Wire.h>
#include <BlynkSimpleEsp8266.h>
#include "SH1106.h" //alis for `#include "SH1106Wire.h"`

#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

DHT dht(DHTPIN, DHTTYPE);
SH1106 display(0x3c, D3, D5);

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "439c6c8b9e6b43089fbba071d3d1983e";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Phong An";
char pass[] = "28091963";

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

  // Init the SSH1106
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 16, "Xin chao Udo");
  display.display();
  Blynk.begin(auth, ssid, pass);
}

int timeSinceLastRead = 0;
void loop() {
  Blynk.run();
  // Report every 2 seconds.
  if(timeSinceLastRead > 2000) {
    display.clear();
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");
    String displayString = String(h) + "% " + String(t) + " C";
    Serial.println(displayString);
    display.drawString(0, 16, displayString);  
    display.display();
    Blynk.virtualWrite(V0, t);
    Blynk.virtualWrite(V1, h);
    if (t > 32)
    {
      Blynk.notify("Udo trym be");  
    }
    timeSinceLastRead = 0;
  }
  delay(10);
  timeSinceLastRead += 10;
}
