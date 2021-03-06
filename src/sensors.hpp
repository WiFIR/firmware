#pragma once

#include "errors.hpp"
#include <time.h>
#include <LittleFS.h>

#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP30.h>

/// A helper class for the sensor package
class Sensors
{
public:

    Sensors(void);
    /**
     * Initialize all sensors
     */
    void begin(void);
    error_t update();

    float t, rh;
    uint16_t eco2, tvoc;

private:
    // AHT20
        Adafruit_AHTX0 * m_aht = nullptr;
        Adafruit_Sensor *m_sens_humidity, *m_sens_temperature;
        sensors_event_t m_humidity, m_temperature;

    // SGP30
        Adafruit_SGP30 * m_sgp = nullptr;
        
        time_t m_sgp_start;
        boolean m_old_baseline = true;

        uint32_t m_get_absolute_humidity(float, float);

        error_t m_sgp_read_baseline(uint16_t *, uint16_t *);
        error_t m_sgp_save_baseline(uint16_t, uint16_t);
        error_t m_sgp_getter(uint16_t * eco2, uint16_t * tvoc);
};