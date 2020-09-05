#include <Arduino.h>


#include <NTPClient.h>
#include <WiFiUdp.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ArduinoOTA.h>

#include <SparkFunBQ27441.h>

#include <TimeLib.h>
#include <Timezone.h>
#include <DebugPrint.h>

// OTA
#define OTA_PUBKEY "-----BEGIN PUBLIC KEY-----MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArCsYpWoithi1pR0IEBvcqkOFWf0P1y/ks9kE2kTDuMM48ZPY30bAUIzOt9PEWnyMBy3LDjf4pF+AtYbtn4pmT6sTRJIQbC/nYIX6by8dsclSIuCzRyyLA/Z+6lBl98Mh8sXvkhTt/f4CX6snyIPxOjMETqL6+C2nwPPqGDwLP8izZz6n4bx8MoSXVIaE+jmahwPKY+71WVLeDGG0WXuznU3wKQBAIhuchEIYEHzXnrflNHqON2kKYB/q+lkK3/aOh3jpdiHz8/x4gwf6gut/vyMrTzRJH9/9pZe3xl6ty2dDvDM2QseGtOkiY4u0ZVcB0ij9LnEQudogUL0rOpQ6NQIDAQAB-----END PUBLIC KEY-----"
BearSSL::PublicKey signPubKey(OTA_PUBKEY);
BearSSL::HashSHA256 hash;
BearSSL::SigningVerifier sign(&signPubKey);

// Time
const char *serverPool = "pool.ntp.org";
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};


// Battery
const uint16_t BATTERY_CAPACITY = 3450;
const uint16_t TERMINATE_VOLTAGE = 3000;
const uint16_t TAPER_CURRENT = 33;

// Prototypes
void config_wifi(void);
void config_ota(void);
void config_time(void);
void config_bq27441(void);

void update_time(void);

void printBatteryStats(void);
char * uintToStr( const uint64_t, char *);
void schlep(uint64_t);

void sendNTPpacket(IPAddress &address);
time_t ntp_sync();

// Objects
DebugPrint Dprint(&Serial, false);
WiFiUDP ntpUDP;

// Central European timezone with DST rules applied
Timezone CE(CEST, CET);

// UTC time, Update every 12h
NTPClient timeClient(ntpUDP, serverPool, 0, 12*60*60*1000);