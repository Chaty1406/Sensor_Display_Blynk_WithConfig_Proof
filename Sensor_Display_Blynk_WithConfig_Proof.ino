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

class BlynkWrapper
{
  private:
    const static unsigned long EMAIL_INTERVAL = 120000;
    bool lastTempAlarm;
    bool lastHumidAlarm;
    unsigned long lastTempEmailTime;
    unsigned long lastHumidEmailTime;
    float tempAlarmLevel;
    float humidityAlarmLevel;
    bool tempAlarmEnabled;
    bool humidityAlarmEnabled;
    
    void notifyTempOutOfRange()
    {
      Blynk.notify("Temperature is out of range"); 
    }
    void notifyHumidOutOfRange()
    {
      Blynk.notify("Humidity is out of range");  
    }
    void emailTempOutOfRange()
    {
      Blynk.email("Server room monitoring alarm", "The temperature is currently out of range");  
    }
    void emailHumidOutOfRange()
    {
      Blynk.email("Server room monitoring alarm", "The humidity is currently out of range");  
    }
    void emailBothOutOfRange()
    {
      Blynk.email("Server room monitoring alarm", "The humidity is currently out of range");
    }
  public:
    void init(char* auth_in, char* ssid_in, char* pass_in)
    {
      Blynk.begin(auth_in, ssid_in, pass_in);
      lastTempAlarm = false;
      lastHumidAlarm = false;
      lastTempEmailTime = millis();
      lastHumidEmailTime = millis();
      tempAlarmEnabled = true;
      tempAlarmLevel = 33;
      humidityAlarmEnabled = true;
      humidityAlarmLevel = 50;
    }
    
    void updateTempAlarmLevel(float inputLevel)
    {
      tempAlarmLevel = inputLevel;
    }
    void updateTempAlarmEnabled(bool input)
    {
      tempAlarmEnabled = input;
    }
    void updateHumidityAlarmLevel(float inputLevel)
    {
      humidityAlarmLevel = inputLevel;
    }
    void updateHumidityAlarmEnabled(bool input)
    {
      humidityAlarmEnabled = input;
    }
    
    void serviceAlarm(float t, float h)
    {
      if (t > tempAlarmLevel && tempAlarmEnabled)
      {
        if (lastTempAlarm)
        {
          // Do nothing
        }
        else
        {
          Serial.println("Temperature alarm");
          notifyTempOutOfRange();
          lastTempAlarm = true;
          if (millis() - lastTempEmailTime > EMAIL_INTERVAL)
          {
            emailTempOutOfRange();
            lastTempEmailTime = millis();
          }
        }
      }
      else
      {
        lastTempAlarm = false;
      }
      
      if (h > humidityAlarmLevel && humidityAlarmEnabled)
      {
        if (lastHumidAlarm)
        {
          
        }
        else
        {
          Serial.println("Humidity alarm");
          notifyHumidOutOfRange();
          lastHumidAlarm = true;
          if (millis() - lastHumidEmailTime > EMAIL_INTERVAL)
          {
            emailHumidOutOfRange();
            lastHumidEmailTime = millis();
          }
        }
      }
      else
      {
        lastHumidAlarm = false;
      }
    }
    
    void updateTempAndHumid(float temp, float humid)
    {
      Blynk.virtualWrite(V0, temp);
      Blynk.virtualWrite(V1, humid);
    }
    
    void run()
    {
      Blynk.run();
    }
};

BlynkWrapper ourWrapper;

BLYNK_WRITE(V2)
{
  Serial.print("Update temp alarm level ");
  Serial.println(param.asFloat());
  ourWrapper.updateTempAlarmLevel(param.asFloat());
}

BLYNK_WRITE(V3)
{
  Serial.print("Update humid alarm level ");
  Serial.println(param.asFloat());
  ourWrapper.updateHumidityAlarmLevel(param.asFloat());
}

BLYNK_WRITE(V4)
{
  Serial.print("Update temp alarm enabled ");
  Serial.println(param.asInt());
  ourWrapper.updateTempAlarmEnabled((bool) param.asInt());
}

BLYNK_WRITE(V5)
{
  Serial.print("Update humid alarm enabled ");
  Serial.println(param.asInt());
  ourWrapper.updateHumidityAlarmEnabled((bool) param.asInt());
}


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
  //Blynk.begin(auth, ssid, pass);
  ourWrapper.init(auth, ssid, pass);
}

int timeSinceLastRead = 0;
void loop() {
  //Blynk.run();
  ourWrapper.run();
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
    ourWrapper.serviceAlarm(t, h);
    ourWrapper.updateTempAndHumid(t, h);
    timeSinceLastRead = 0;
  }
  delay(10);
  timeSinceLastRead += 10;
}
