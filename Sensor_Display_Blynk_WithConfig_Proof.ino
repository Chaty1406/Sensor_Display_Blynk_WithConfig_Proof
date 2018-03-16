  /**
   DHT22 + SH1106 test program
   Pin connection scheme
   SH1106 OLED
    5V -> VCC
    D3 -> SDA
    D5 -> SCL
   SI7021
    3V3 -> VCC
    D1 -> SCL
    D2 -> SDA
*/

#include <Wire.h>
#include <BlynkSimpleEsp8266.h>
#include "SH1106.h"
#include "xbmFiles.h"
#include <WidgetRTC.h>

SH1106 display(0x3c, D3, D5);

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "439c6c8b9e6b43089fbba071d3d1983e";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Phong An";
char pass[] = "28091963";

WidgetLCD lcd(V6);

class SI7021Wrapper
{
#define si7021Addr 0x40
  private:
    int i2cAddr;
  public:
    SI7021Wrapper(int input = si7021Addr)
    {
      i2cAddr = input;
    }
    void init()
    {
      Wire.begin();
      Wire.beginTransmission(0x40);
      Wire.endTransmission();
      delay(300);
    }

    unsigned char readSensor(float & outTemp, float & outHumid)
    {
      unsigned int data[2];
      Wire.beginTransmission(0x40);
      //Send humidity measurement command
      Wire.write(0xF5);
      Wire.endTransmission();
      delay(500);

      // Request 2 bytes of data
      Wire.requestFrom(0x40, 2);
      // Read 2 bytes of data to get humidity
      if (Wire.available() == 2)
      {
        data[0] = Wire.read();
        data[1] = Wire.read();
      }
      else
      {
        Serial.println("Fail humid");
        Serial.println(Wire.available());
        while (Wire.available())
        {
          Wire.read();
        }
        return 0;
      }

      // Convert the data
      float humidity  = ((data[0] * 256.0) + data[1]);
      humidity = ((125 * humidity) / 65536.0) - 6;

      Wire.beginTransmission(0x40);
      // Send temperature measurement command
      Wire.write(0xF3);
      Wire.endTransmission();
      delay(500);

      // Request 2 bytes of data
      Wire.requestFrom(0x40, 2);

      // Read 2 bytes of data for temperature
      if (Wire.available() == 2)
      {
        data[0] = Wire.read();
        data[1] = Wire.read();
      }
      else
      {
        Serial.println("Fail temp");
        while (Wire.available())
        {
          Wire.read();
        }
        return 0;
      }
      // Convert the data
      float temp  = ((data[0] * 256.0) + data[1]);
      float celsTemp = ((175.72 * temp) / 65536.0) - 46.85;

      outTemp = celsTemp;
      outHumid = humidity;
      return 1;
    }
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
        lcd.print(0, 0, "Temp: WARNING");
        lcd.print(0, 1, "Humid: WARNING");
      }
      else if (t > tempAlarmLevel)
      {
        lcd.print(0, 0, "Temp: WARNING");
        lcd.print(0, 1, "Humid: OK");
      }
      else if (h > humidityAlarmLevel)
      {
        lcd.print(0, 0, "Temp: OK");
        lcd.print(0, 1, "Humid: WARNING");
      }
      else
      {
        lcd.print(0, 0, "Temp: OK");
        lcd.print(0, 1, "Humid: OK");
      }
    }


    void updateTempAlarmLevel(float inputLevel)
    {
      tempAlarmLevel = inputLevel;
    }

    float getTempAlarmLevel()
    {
      return tempAlarmLevel;
    }
    
    void updateTempAlarmEnabled(bool input)
    {
      tempAlarmEnabled = input;
    }
    void updateHumidityAlarmLevel(float inputLevel)
    {
      humidityAlarmLevel = inputLevel;
    }
    float getHumidityAlarmLevel()
    {
      return humidityAlarmLevel;
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
SI7021Wrapper sensorWrapper;
WidgetRTC rtc;

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

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while (!Serial) { }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running SI7021!");
  Serial.println("-------------------------------------");

  //Init the SSH1106
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawXbm(0, 0, logo_width, logo_height, logo_short);
  display.display();
  sensorWrapper.init();
  ourWrapper.init(auth, ssid, pass);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  delay(2000);
}

unsigned long timeSinceLastRead = 0;
unsigned long samplingInterval = 2000;
void loop() {
  ourWrapper.run();
  // Report every 2 seconds.
  if (millis() - timeSinceLastRead > samplingInterval)
  {
    display.clear();
    // Declare variables to hold sensor values
    // As these are allocated on the stack, no manual memory management is needed
    float h = 0;
    float t = 0;
    // Check if any reads failed and exit early (to try again).
    if (sensorWrapper.readSensor(t, h) == 0)
    {
      Serial.println("Failed to read from SI7021!");
      delay(500);
      t = -1;
      h = -1;
    }
    String tempString;
    String humidityString;
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    if ((t != -1) && (h != -1))
    {
      timeSinceLastRead = millis();
      tempString = String(t) + "C ";
      humidityString = String(h) + "%";
      Serial.println(tempString + " " + humidityString);
      display.drawXbm(17, 48, sensorSymbol_width, sensorSymbol_height, sensorSymbol);
    }
    else
    {
      tempString = "NaN";
      humidityString = "NaN";
    }
    if (Blynk.connected())
    {
      display.drawXbm(0, 48, wifiSymbol_width, wifiSymbol_height, wifiSymbol);
    }
    display.drawXbm(0, 0, tempSymbol_width, tempSymbol_height, tempSymbol);
    display.drawString(16, 0, tempString);
    if (t > ourWrapper.getTempAlarmLevel() || t == -1)
    {
      display.drawXbm(100, 0, crossedSymbol_width, crossedSymbol_height, crossedSymbol);
    }
    else
    {
      display.drawXbm(100, 0, checkedSymbol_width, checkedSymbol_height, checkedSymbol);
    }
    display.drawXbm(0, 16, humiditySymbol_width, humiditySymbol_height, humiditySymbol);
    display.drawString(16, 16, humidityString);
    if (h > ourWrapper.getHumidityAlarmLevel() || h == -1)
    {
      display.drawXbm(100, 16, crossedSymbol_width, crossedSymbol_height, crossedSymbol);
    }
    else
    {
      display.drawXbm(100, 16, checkedSymbol_width, checkedSymbol_height, checkedSymbol);
    }
    /*
     * Date and time
     */
    String currentTime = String(hour()) + ":" + minute();
    String currentDate = String(month()) + "/" + day() + "/" + year();
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 32, currentDate);
    display.drawString(128, 48, currentTime);
    display.display();
    ourWrapper.serviceAlarm(t, h);
    ourWrapper.updateLCD(lcd, t, h);
    ourWrapper.updateTempAndHumid(t, h);
  }
}
