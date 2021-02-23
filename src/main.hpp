
#include <Arduino.h>
#include "secrets.hpp"

#include <ESP8266WiFi.h>
#include <CertStoreBearSSL.h>
#include <time.h>
#include <FS.h>
#include <LittleFS.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Remember to define the secrets in secrets.h

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
// Using port 8883 for MQTTS
#define AIO_SERVERPORT  8883


/*********************** Secure client with CertStore  ***********************/
// Cert store object to read from LittleFS
BearSSL::CertStore certStore;

// WiFiFlientSecure for SSL/TLS support
BearSSL::WiFiClientSecure * client = new BearSSL::WiFiClientSecure();

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'test' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish test = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test");

void MQTT_connect(void);
void set_clock(void);
int init_certStore(void);