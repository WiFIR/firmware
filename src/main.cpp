#include "main.h"

#define CLEAR_PIN D7

void setup() 
{
  Dprint.begin(115200, 23);

  config_wifi();
  config_bq27441();
}

void loop()
{
    printBatteryStats();
    delay(1000);
}

void config_wifi()
{
  WiFi.hostname("WiFIR");
  WiFiManager wifiManager;

  pinMode(CLEAR_PIN, INPUT_PULLUP);
  delay(10);

  // Check if the user reset via the factory-reset button
  if(!digitalRead(CLEAR_PIN)) // If reset via clear pin
  {
    // Wait 5 seconds
    uint32_t delay_start = millis();
    while(!digitalRead(CLEAR_PIN) && millis() - delay_start > 5000)
    {
      delay(50);
    }
    // Ensure any button bounce has cleared in case the button was released prematurely.
    delay(10);

    // If button still held, clear WiFi settings.
    if(!digitalRead(CLEAR_PIN))
    {
      wifiManager.resetSettings();
    }
  }

  wifiManager.autoConnect("WiFIR");
}

void config_bq27441()
{
  if (!lipo.begin()) // begin() will return true if communication is successful
  {
      // If communication fails, print an error message and loop forever.
      Dprint.println("Error: Unable to communicate with BQ27441.");
      Dprint.println("  Check wiring and try again.");
      Dprint.println("  (Battery must be plugged into Battery Babysitter!)");
      while (1)
          ;
  }
  Dprint.println("Connected to BQ27441!");

  if (lipo.itporFlag()) //write config parameters only if needed
  {
      Dprint.println("Writing gague config");

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
      Dprint.println("Using existing gague config");
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

void printBatteryStats()
{
    // Read battery stats from the BQ27441-G1A
    unsigned int soc = lipo.soc();                        // Read state-of-charge (%)
    unsigned int volts = lipo.voltage();                  // Read battery voltage (mV)
    int current = lipo.current(AVG);                      // Read average current (mA)
    unsigned int fullCapacity = lipo.capacity(FULL);      // Read full capacity (mAh)
    unsigned int capacity = lipo.capacity(REMAIN);        // Read remaining capacity (mAh)
    int power = lipo.power();                             // Read average power draw (mW)
    int health = lipo.soh();                              // Read state-of-health (%)
    unsigned int temp = lipo.temperature();               // Reads the battery temperature

    
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