#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <LowPower.h>
#include "./src/Communication/Communication.h"
#include "./src/HC12/HC12.hpp"

#define DEVICE_ID "45C5"

#define HC12_SET 9
#define HC12_RX 10
#define HC12_TX 11
#define UVOUT A0   // Output from the sensor
#define BATTERY A3 // Analog read for battery status
#define REF3V3 3.317

#define ERR_BATTERY 0x01
#define ERR_TEMPHUM 0x02
#define ERR_UV 0x04
#define ERR_LIGH 0x08
#define ERR_COMM 0x10

Adafruit_BME280 _bme280;
BH1750 _bh1750;
Communication _comm = Communication(HC12_RX, HC12_TX, HC12_SET);
byte err;

void setup()
{
    Wire.begin();
    err = 0;
    pinMode(HC12_SET, OUTPUT);
    pinMode(UVOUT, INPUT);
    pinMode(BATTERY, INPUT);

    if (!_bme280.begin(0x76))
    {
        err |= ERR_TEMPHUM;
    }

    _bme280.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1, // Temperature
                        Adafruit_BME280::SAMPLING_X1, // Pressure
                        Adafruit_BME280::SAMPLING_X1, // Humidity
                        Adafruit_BME280::FILTER_OFF);

    if (!_bh1750.begin(BH1750::ONE_TIME_LOW_RES_MODE)) // Measurement at 4 lux resolution
    {
        err |= ERR_LIGH;
    }

    _comm.begin(DEVICE_ID);

    if (err)
    {
        digitalWrite(13, HIGH);
    }
}

void loop()
{

    // Read samples from BME280
    _bme280.takeForcedMeasurement();
    float temperature = _bme280.readTemperature();
    float humidity = _bme280.readHumidity();
    float pressure = _bme280.readPressure() / 100.0F;

    // Read luminosity
    float lux = _bh1750.readLightLevel();

    // Get sample interval from last message
    unsigned int sample_interval = _comm.getSampleInterval();

    // Read battery voltage
    float battery = averageAnalogRead(BATTERY) * REF3V3 / 1024.0F;
    // R1 = 463K
    // R2 = 1490K
    battery = (battery * 1953.0F) / 1490.0F;
    if (battery < 3.6)
    {
        err |= ERR_BATTERY;
    }

    // Read UV sensor voltage
    float outputVoltage = averageAnalogRead(UVOUT) * REF3V3 / 1024.0F;
    float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
    if (uvIntensity < 0)
    {
        uvIntensity = 0;
    }

    // Generate message
    String output;
    output = "I" + String(sample_interval) + SEP_CHAR;
    output += "S" + String(err, DEC) + SEP_CHAR;
    output += "B" + convertValue(battery) + SEP_CHAR;
    output += "T" + convertValue(temperature) + SEP_CHAR;
    output += "H" + convertValue(humidity) + SEP_CHAR;
    output += "P" + convertValue(pressure) + SEP_CHAR;
    output += "L" + convertValue(lux) + SEP_CHAR;
    output += "U" + convertValue(uvIntensity);

    // Send message
    _comm.sendMessage(output) ? err ^= ERR_COMM : err |= ERR_COMM;

    // Wait until next sample
    for (unsigned int i = 0; i < sample_interval / 8; i++)
    {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
}

String convertValue(float original)
{
    String value = String(original, 2);
    String clean = "";
    for (int i = 0; i < value.length(); i++)
    {
        if (value[i] != '.')
        {
            clean += value[i];
        }
    }
    return clean;
}

// Takes an average of readings on a given pin
int averageAnalogRead(int pinToRead)
{
    byte numberOfReadings = 8;
    unsigned int runningValue = 0;

    for (int i = 0; i < numberOfReadings; i++)
    {
        runningValue += analogRead(pinToRead);
        delay(200);
    }
    runningValue /= numberOfReadings;

    return (runningValue);
}

// The Arduino Map function but for floats
// From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
