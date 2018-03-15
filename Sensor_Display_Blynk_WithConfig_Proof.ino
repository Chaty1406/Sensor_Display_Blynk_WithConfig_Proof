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
char ssid[] = "ASX_Public";
char pass[] = "!AsxP@ssw0rd";

WidgetLCD lcd(V6);

#define logo_width 46
#define logo_height 40
const char logo_short[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0xE0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x06, 0x00, 0x00, 
  0x00, 0x00, 0x70, 0x06, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0C, 0x00, 0x00, 
  0x00, 0x00, 0x78, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x78, 0x08, 0x00, 0x00, 
  0x00, 0x00, 0x7C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x18, 0x00, 0x00, 
  0x00, 0x00, 0x7E, 0xB0, 0x0F, 0x00, 0x00, 0x00, 0x3E, 0xF8, 0x50, 0x00, 
  0x00, 0x00, 0x7F, 0x3F, 0xC0, 0x01, 0x00, 0x00, 0xFF, 0x33, 0x80, 0x01, 
  0x00, 0x80, 0xFF, 0x60, 0x80, 0x03, 0x00, 0x80, 0x3F, 0x60, 0x80, 0x01, 
  0x00, 0x80, 0x3F, 0xE0, 0x80, 0x03, 0x00, 0xC0, 0x3F, 0x60, 0x80, 0x01, 
  0x00, 0xE0, 0x3F, 0xC0, 0xC0, 0x00, 0x00, 0xF8, 0x3F, 0xE0, 0xE0, 0x00, 
  0x00, 0xFC, 0x3F, 0xC0, 0x71, 0x00, 0x00, 0xE6, 0x3F, 0xC0, 0x71, 0x00, 
  0x80, 0xE7, 0x3F, 0x80, 0x1D, 0x00, 0xC0, 0xE3, 0x3F, 0xC0, 0x1F, 0x00, 
  0xC0, 0xF0, 0x3F, 0x80, 0x07, 0x00, 0xF0, 0xF0, 0x3F, 0xC0, 0x03, 0x00, 
  0x70, 0xF8, 0x1F, 0xE0, 0x03, 0x00, 0x38, 0xF8, 0x3F, 0xF0, 0x03, 0x00, 
  0x38, 0xF8, 0x1F, 0x1C, 0x07, 0x00, 0x1C, 0xF8, 0x1F, 0x8F, 0x07, 0x00, 
  0x18, 0xFC, 0xDF, 0x03, 0x07, 0x00, 0x1C, 0xFC, 0xFF, 0x01, 0x07, 0x00, 
  0x18, 0xFC, 0x3F, 0x00, 0x0F, 0x00, 0x38, 0xFE, 0x1F, 0x00, 0x0F, 0x00, 
  0xE0, 0xFE, 0x1F, 0x00, 0x0F, 0x00, 0x80, 0xFF, 0x1F, 0x00, 0x0E, 0x00, 
  0x00, 0xFE, 0x1F, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  };


class BlynkWrapper
{
  private:
    const static unsigned long EMAIL_INTERVAL = 20000;
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
      tempAlarmEnabled = false;
      Blynk.virtualWrite(V4, LOW);
      tempAlarmLevel = 22;
      Blynk.virtualWrite(V2, 22);
      humidityAlarmEnabled = false;
      Blynk.virtualWrite(V5, LOW);
      humidityAlarmLevel = 55;
      Blynk.virtualWrite(V3, 55);
    }

    void updateLCD(WidgetLCD lcd, float t, float h)
    {
      lcd.clear();
      if ((t > tempAlarmLevel) && (h > humidityAlarmLevel))
      {
        lcd.print(0,0, "Temp: WARNING");
        lcd.print(0,1, "Humid: WARNING");
      }
      else if (t > tempAlarmLevel)
      {
        lcd.print(0,0, "Temp: WARNING");
        lcd.print(0,1, "Humid: OK");      
      }
      else if (h > humidityAlarmLevel)
      {
        lcd.print(0,0, "Temp: OK");
        lcd.print(0,1, "Humid: WARNING");  
      }
      else
      {
        lcd.print(0,0, "Temp: OK");
        lcd.print(0,1, "Humid: OK");
      }
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
        Serial.println("Temperature alarm");
        notifyTempOutOfRange();
        if (lastTempAlarm)
        {
          // Do nothing
        }
        else
        {
          lastTempAlarm = true;
          if (millis() - lastTempEmailTime > EMAIL_INTERVAL)
          {
            Serial.println("Temp email sent");
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
        Serial.println("Humidity alarm");
        notifyHumidOutOfRange();
        if (lastHumidAlarm)
        {
          
        }
        else
        {
          lastHumidAlarm = true;
          if (millis() - lastHumidEmailTime > EMAIL_INTERVAL)
          {
            Serial.println("Humid email sent");
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
  display.drawXbm(41, 23, logo_width, logo_height, logo_short);
  display.display();
  ourWrapper.init(auth, ssid, pass);
  delay(5000);
}

unsigned long timeSinceLastRead = 0;
unsigned long samplingInterval = 5000;
void loop() {
  ourWrapper.run();
  // Report every 2 seconds.
  if(millis() - timeSinceLastRead > samplingInterval) 
  {
    display.clear();
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      //Serial.println("Failed to read from DHT sensor!");
      delay(500);
      return;
    }
    timeSinceLastRead = millis();
    String displayString = String(h) + "% " + String(t) + " C";
    Serial.println(displayString);
    display.drawString(0, 16, displayString);  
    display.display();
    ourWrapper.serviceAlarm(t, h);
    ourWrapper.updateLCD(lcd, t, h);
    ourWrapper.updateTempAndHumid(t, h);
  }
}
