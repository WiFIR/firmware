#include "sensors.hpp"

Sensors::Sensors()
{
  m_aht = new Adafruit_AHTX0();
  m_sgp = new Adafruit_SGP30();
}

error_t Sensors::update()
{
  m_sgp_getter(&eco2, &tvoc);
  m_sens_temperature->getEvent(&m_temperature);
  m_sens_humidity->getEvent(&m_humidity);
  t = m_temperature.temperature;
  rh = 
}

void Sensors::begin()
{
  // AHT20
  if(!m_aht->begin())
  {
    Serial.println("Failed to find AHT10/AHT20 chip");
    while (true)
    {
      delay(10);
    }
  }
  
  m_sens_temperature = m_aht->getTemperatureSensor();
  m_sens_humidity = m_aht->getHumiditySensor();

  m_sens_temperature->printSensorDetails();
  m_sens_humidity->printSensorDetails();
  
  // SGP30
  if (!m_sgp->begin())
  {
    Serial.println("SGP30 not found :(");
    while (true)
    {
      delay(10);
    }
  }

  m_sgp_start = time(nullptr);

  uint16_t eco2_base, tvoc_base;
  error_t rc = m_sgp_read_baseline(&eco2_base, &tvoc_base);
  switch(rc)
  {
    case E_SUCCESS:
      m_old_baseline = false;
      m_sgp->setIAQBaseline(eco2_base, tvoc_base);
      break;

    case W_OLD_BASELINE:
      m_old_baseline = true;
      m_sgp->setIAQBaseline(eco2_base, tvoc_base);
      break;

    default:
      print_error(rc);
      break;
  }
  
  
  m_sens_temperature->getEvent(&m_temperature);
  m_sens_humidity->getEvent(&m_humidity);
  m_sgp->setHumidity(m_get_absolute_humidity(m_temperature.temperature, m_humidity.relative_humidity));
}

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t Sensors::m_get_absolute_humidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

error_t Sensors::m_sgp_read_baseline(uint16_t * eco2_base, uint16_t * tvoc_base)
{
  if ( !LittleFS.exists(F("sgp_baseline")) )
  {
    return W_FILE_NOT_FOUND;
  }

  File baseline = LittleFS.open(F("sgp_baseline"), "r");
  if (!baseline)
  {
    return E_FILE_ACCESS;
  }

  *eco2_base = baseline.parseInt();
  *tvoc_base = baseline.parseInt();
  baseline.close();

  time_t now = time(nullptr);
  // Baseline with sensor off is valid for 7 days
  if (now - baseline.getLastWrite() > 7 * 24 * 60 * 60)
  {
    return W_OLD_BASELINE;
  }
  return E_SUCCESS;
}

error_t Sensors::m_sgp_save_baseline(uint16_t eco2_base, uint16_t tvoc_base)
{
  File baseline = LittleFS.open(F("sgp_baseline"), "w");
  if (!baseline)
  {
    return E_FILE_ACCESS;
  }

  baseline.println(eco2_base);
  baseline.println(tvoc_base);
  baseline.close();
  return E_SUCCESS;
}

error_t Sensors::m_sgp_getter(uint16_t * eco2, uint16_t * tvoc)
{
  error_t rc = E_SUCCESS;
  if (!m_old_baseline)
  {
    // If the baseline isn't old, save it every hour
    if ( (time(nullptr) - m_sgp_start) % (60*60) )
    {
      uint16_t eco2_base, tvoc_base;
      if(m_sgp->getIAQBaseline(&eco2_base, &tvoc_base))
      {
        rc = m_sgp_save_baseline(eco2_base, tvoc_base);
        if (rc == E_SUCCESS)
        {
          Serial.printf("Saved baseline:\n\teco2: %X\n\ttvoc: %X", eco2_base, tvoc_base);
        }
      }
    }
  }
  else // Old sample, wait 12h before saving
  {
    if (time(nullptr) - m_sgp_start > 12 * 60 * 60)
    {
      // If the sensor has had 12h to get a baseline, it's no-longer old
      m_old_baseline = false;
    }
  }

  if (! m_sgp->IAQmeasure())
  {
    return E_SENSOR;
  }
  *eco2 = m_sgp->eCO2;
  *tvoc = m_sgp->TVOC;
  return rc;
}