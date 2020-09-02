#include <Arduino.h>

// WiFiManager imports
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <SparkFunBQ27441.h>
#include <DebugPrint.h>


// Battery
const uint16_t BATTERY_CAPACITY = 3450;
const uint16_t TERMINATE_VOLTAGE = 3000;
const uint16_t TAPER_CURRENT = 55;


// Prototypes
void config_wifi(void);
void config_bq27441(void);

void printBatteryStats(void);
char * uintToStr( const uint64_t, char *);
void schlep(uint64_t);

// Objects
DebugPrint Dprint(&Serial, false);