#include "main.hpp"

/*************************** Sketch Code ************************************/

void setup() 
{
  Serial.begin(115200);
  Serial.println('\n');
  delay(10);

  LittleFS.begin();

  Serial.println(F("Adafruit IO MQTTS (SSL/TLS) Example using BearSSL certstore"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  set_clock(); // Required for X.509 validation
  int certs = init_certStore();
  if(certs == 0)
  {
    while(true)
    {
      Serial.println("Cert initialization failed. Can't do anything");
      delay(10000);
    }
  }
  else
  {
    Serial.printf("Found %d certs in certificate store\n", certs);
  }
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Get sensor data
  sens_temperature->getEvent(&temperature);
  sens_humidity->getEvent(&humidity);

  uint16_t eco2, tvoc;
  error_t rc = sgp_getter(&eco2, &tvoc);
  
  if (rc)
  {
    print_error(rc);
  }

  // Publish sensor data

  Serial.printf(
    "Publishing data:\n\ttemp: %f C\n\thumi: %f %%\n\teco2: %d ppm\n\ttvoc: %d ppb\n",
    temperature.temperature, humidity.relative_humidity, eco2, tvoc);

  if (! feed_temp.publish(temperature.temperature))
  {
    Serial.println("Failed to publish temperature");
  }

  if (! feed_humi.publish(humidity.relative_humidity))
  {
    Serial.println("Failed to publish relative humidity");
  }

  if (! feed_tvoc.publish(tvoc))
  {
    Serial.println("Failed to publish TVOC");
  }

  if (! feed_eco2.publish(eco2))
  {
    Serial.println("Failed to publish eCO2");
  }

  // Check for new state

    // get mqtt message with new AC state

  // Send state if new

    // send new state over IR if needed

  // wait a couple seconds to avoid rate limit
  delay(20000);

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }

  Serial.println("MQTT Connected!");
}

void set_clock() 
{
  configTime(current_TZ, "pool.ntp.org");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  Serial.print(ctime(&now));
}




void init_sensors()
{
  // IR
  // Recv
  irrecv.enableIRIn();
  irsend.begin();
}

int init_certStore(void)
{
  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.printf("Number of CA certs read: %d\n", numCerts);
  if (numCerts == 0) 
  {
    Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
    return 0; // Can't connect to anything w/o certs!
  }

  client->setCertStore(&certStore);
  return numCerts;
}

