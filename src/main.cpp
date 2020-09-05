#include "main.h"

#define CLEAR_PIN D7

void setup() 
{
  Dprint.begin(115200, 23, true);


  config_wifi();
  config_time();
  config_bq27441();
}

void config_ota()
{
  Update.installSignature(&hash, &sign);
  ArduinoOTA.onStart([]()
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else // U_FS
    {
      type = "filesystem";
    }
    Dprint.println("OTA firmware started. Target: " + type);
  });

  ArduinoOTA.onEnd([]()
  {
    Dprint.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    Dprint.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) 
  {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
}

void loop()
{
  ArduinoOTA.handle();
  printBatteryStats();
}

void config_wifi()
{
  WiFi.hostname("WiFIR");
  WiFiManager wifiManager;

  pinMode(CLEAR_PIN, INPUT_PULLUP);
  pinMode(D4, OUTPUT);
  delay(10);

  // Check if the user reset via the factory-reset button
  if(!digitalRead(CLEAR_PIN)) // If reset via clear pin
  {
    // Wait 5 seconds
    uint32_t delay_start = millis();
    while(!digitalRead(CLEAR_PIN) && millis() - delay_start < 5000)
    {
      digitalWrite(D4, !digitalRead(D4));
      delay(200);
    }
    // Turn on D4 to signal reset
    digitalWrite(D4, LOW);
    // Ensure any button bounce has cleared in case the button was released prematurely.
    delay(10);

    // If button still held, clear WiFi settings.
    if(!digitalRead(CLEAR_PIN))
    {
      wifiManager.resetSettings();
    }
  }

  wifiManager.autoConnect("WiFIR");
  
  //Flash the LED
  for (int i=0; i<10; i++)
  {
    digitalWrite(D4, !digitalRead(D4));
    delay(50);
  }
  // Turn off D4
  digitalWrite(D4, HIGH);
}

void config_time()
{
  Dprint.print(F("Syncronizing time with NTP"));
  timeClient.begin();

  while (year(timeClient.getEpochTime()) == 1970)
  {
    timeClient.update();
    delay(500);
    Dprint.print(F("."));
    /* code */
  }
  Dprint.println();
  setTime(CE.toLocal(timeClient.getEpochTime()));
  
}

void update_time()
{
  timeClient.update();
  setTime(CE.toLocal(timeClient.getEpochTime()));
}

void config_bq27441()
{
  if (!lipo.begin()) // begin() will return true if communication is successful
  {
      // If communication fails, print an error message and loop forever.
      Dprint.println(F("Error: Unable to communicate with BQ27441."));
      Dprint.println(F("  Check wiring and try again."));
      Dprint.println(F("  (Battery must be plugged into Battery Babysitter!)"));
      while (1){delay(0);}
  }
  Dprint.println(F("Connected to BQ27441!"));

  if (lipo.itporFlag()) //write config parameters only if needed
  {
      Dprint.println(F("Writing gague config"));

      lipo.enterConfig();                 // To configure the values below, you must be in config mode
      lipo.setCapacity(BATTERY_CAPACITY); // Set the battery capacity

      /*
          Design Energy should be set to be Design Capacity × 3.7 if using the bq27441-G1A or Design
          Capacity × 3.8 if using the bq27441-G1B
      */
      lipo.setDesignEnergy(BATTERY_CAPACITY * 3.7f);

      /*
          Terminate Voltage should be set to the minimum operating voltage of your system. This is the target
          where the gauge typically reports 0% capacity
      */
      lipo.setTerminateVoltage(TERMINATE_VOLTAGE);

      /*
          Taper Rate = Design Capacity / (0.1 * Taper Current)
      */
      lipo.setTaperRate(10 * BATTERY_CAPACITY / TAPER_CURRENT);

      lipo.exitConfig(); // Exit config mode to save changes
  }
  else
  {
      Dprint.println(F("Using existing gague config"));
  }
}

char * uintToStr( const uint64_t num, char *str )
{
  uint8_t i = 0;
  uint64_t n = num;
 
  do
    i++;
  while ( n /= 10 );
 
  str[i] = '\0';
  n = num;
 
  do
    str[--i] = ( n % 10 ) + '0';
  while ( n /= 10 );

  return str;
}

void schlep(uint64_t duration)
{
  char dur[21];
  uintToStr(duration, dur);
  Dprint.println("Sleeping for " + String(dur) + "uS");
  ESP.deepSleepInstant(duration);
}

int check_changed(int previous, int current, bool * status)
{
  if (previous != current)
  {
    *status = true;
  }
  return current;
}

unsigned int soc, volts, fullCapacity, capacity, temp;
int current, power, health;
unsigned long last_status = 0;
void printBatteryStats()
{
  if (millis() - last_status < 500)
  {
    return;
  }
  last_status = millis();

  bool changed = false;
  // Read battery stats from the BQ27441-G1A
  soc = check_changed(soc, lipo.soc(), &changed);                             // Read state-of-charge (%)
  volts = check_changed(volts, lipo.voltage(), &changed);                     // Read battery voltage (mV)
  current = lipo.current(AVG);                                                // Read average current (mA)
  fullCapacity = check_changed(fullCapacity, lipo.capacity(FULL), &changed);  // Read full capacity (mAh)
  capacity = check_changed(capacity, lipo.capacity(REMAIN), &changed);        // Read remaining capacity (mAh)
  power = lipo.power()                                ;                       // Read average power draw (mW)
  health = check_changed(health, lipo.soh(), &changed);                       // Read state-of-health (%)
  temp = check_changed(temp,lipo.temperature(), &changed);                    // Reads the battery temperature

  if(changed)
  {
    // Insert . into temp
    String tempS = String(temp).substring(0,2) + "." + String(temp).substring(2);

    // Assemble a string to print
    String toPrint = String(soc) + "% | ";
    toPrint += String(volts) + " mV | ";
    toPrint += String(current) + " mA | ";
    toPrint += String(capacity) + " / ";
    toPrint += String(fullCapacity) + " mAh | ";
    toPrint += String(power) + " mW | ";
    toPrint += String(health) + "% | ";
    toPrint += String(tempS) + " °C ";

    //fast charging allowed
    if (lipo.chgFlag())
        toPrint += " CHG";

    //full charge detected
    if (lipo.fcFlag())
        toPrint += " FC";

    //battery is discharging
    if (lipo.dsgFlag())
        toPrint += " DSG";

    // Print the string
    Dprint.println(toPrint);
  }
}
