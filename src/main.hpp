
#include <Arduino.h>
#include "secrets.hpp"
#include "errors.hpp"

#include <ESP8266WiFi.h>

#include <TZ.h>
#include <time.h>
#include <CertStoreBearSSL.h>

#include <FS.h>
#include <LittleFS.h>

#include "sensors.hpp"

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"



#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRremoteESP8266.h>
#include <ir_Panasonic.h>

// Remember to define the secrets in secrets.h

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
// Using port 8883 for MQTTS
#define AIO_SERVERPORT  8883


/*********************** Secure client with CertStore  ***********************/
// Timezone
#define current_TZ TZ_Europe_Copenhagen

// Cert store object to read from LittleFS
BearSSL::CertStore certStore;

// WiFiFlientSecure for SSL/TLS support
BearSSL::WiFiClientSecure * client = new BearSSL::WiFiClientSecure();

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/******************************* Feeds ***************************************/

// Setup a feed called 'test' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish feed_temp  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/wifir-test.temp");
Adafruit_MQTT_Publish feed_humi  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/wifir-test.humi");
Adafruit_MQTT_Publish feed_tvoc  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/wifir-test.tvoc");
Adafruit_MQTT_Publish feed_eco2  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/wifir-test.eco2");
Adafruit_MQTT_Publish feed_state = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/wifir-test.state");

/****************************** Sensors **************************************/
// IR
const uint8_t  ir_pins[] = {D4, D6};
const uint16_t ir_recv_buffer = 1024;  // ~511 bits
const uint16_t ir_freq = 36700;
const uint8_t  ir_timeout = 50;

IRrecv irrecv(ir_pins[0], ir_recv_buffer, ir_timeout, false);
IRsend irsend(ir_pins[1]);
decode_results results;


/***************************** Prototypes ************************************/

// Networking
void MQTT_connect(void);
void set_clock(void);
int init_certStore(void);

// Sensors


// MISC
