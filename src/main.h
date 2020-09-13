#include <Arduino.h>
#include <Wire.h>


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

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Panasonic.h>

#include <Temperature_LM75_Derived.h>

#include <AsyncMqttClient.h>

// Pin mapping
enum pin_map {
	P_IR_I     = D4,
	P_IR_O     = D5,
	P_LOW_BAT  = D6,
	P_FCT_RST  = D7,
	P_IR_R_VCC = D8
};

const uint8_t pins[][2] = {
	{P_IR_I, INPUT},
	{P_IR_O, OUTPUT},
	{P_LOW_BAT, INPUT},
	{P_FCT_RST, INPUT_PULLUP},
	{P_IR_R_VCC, OUTPUT}
};


// OTA
BearSSL::PublicKey signPubKey("-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArCsYpWoithi1pR0IEBvc\nqkOFWf0P1y/ks9kE2kTDuMM48ZPY30bAUIzOt9PEWnyMBy3LDjf4pF+AtYbtn4pm\nT6sTRJIQbC/nYIX6by8dsclSIuCzRyyLA/Z+6lBl98Mh8sXvkhTt/f4CX6snyIPx\nOjMETqL6+C2nwPPqGDwLP8izZz6n4bx8MoSXVIaE+jmahwPKY+71WVLeDGG0WXuz\nnU3wKQBAIhuchEIYEHzXnrflNHqON2kKYB/q+lkK3/aOh3jpdiHz8/x4gwf6gut/\nvyMrTzRJH9/9pZe3xl6ty2dDvDM2QseGtOkiY4u0ZVcB0ij9LnEQudogUL0rOpQ6\nNQIDAQAB\n-----END PUBLIC KEY-----");
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
void config_ac(void);
void config_mqtt(void);

void update_time(void);

void printBatteryStats(void);
char * uintToStr( const uint64_t, char *);
void schlep(uint64_t);

void sendNTPpacket(IPAddress &address);
time_t ntp_sync();

// Objects
DebugPrint Dprint(&Serial, false);
WiFiUDP ntpUDP;

IRPanasonicAc ac(P_IR_O);

// 
AsyncMqttClient mqtt_client;

ON_NCT75 temp_sens(0x4F);

// Central European timezone with DST rules applied
Timezone CE(CEST, CET);

// UTC time, Update every 12h
NTPClient timeClient(ntpUDP, serverPool, 0, 12*60*60*1000);