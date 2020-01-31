#include <Arduino.h>
#include <Adafruit_BMP280.h> // BMP280_ADDRESS (0x76)
#include <Adafruit_SHT31.h>
#include <BH1750.h>
#include <LowPower.h>
#include "communication.h"

#define DEVICE_ID "45C5"

#define HC12_AT 9
#define HC12_RX 10
#define HC12_TX 11
#define UVOUT   A0 // Output from the sensor
#define BATTERY A3 // Analog read for battery status
#define REF3V3  3.31

#define ERR_PRESSURE 0x01
#define ERR_HUMIDITY 0x02
#define ERR_UV       0x04
#define ERR_LIGH     0x08
#define ERR_COMM     0x10
#define ERR_BATTERY  0x20

Adafruit_SHT31 _sht31 = Adafruit_SHT31();
Adafruit_BMP280 _bmp280;
BH1750 _bh1750;
Communication _comm = Communication(HC12_RX, HC12_TX);
byte err;


void setup() {
    err = 0;

    pinMode(HC12_AT, OUTPUT);

    pinMode(UVOUT, INPUT);
    pinMode(BATTERY, INPUT);

    if(!_sht31.begin(0x44)){
        err |= ERR_HUMIDITY;
    }

    if(!_bmp280.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)){
        err |= ERR_PRESSURE;
    }

    _bh1750.begin();

    _comm.begin(DEVICE_ID, HC12_AT);

    if (!_comm.pairing()) {
        err |= ERR_COMM;
    }

    if (err) {
        digitalWrite(13, HIGH);
    }
}


void loop() {

    while (! _sht31.readTempHum());
    float temperature = _sht31.readTemperature();
    float humidity = _sht31.readHumidity();
    float pressure = _bmp280.readPressure();
    uint16_t lux = _bh1750.readLightLevel();
    unsigned int sample_interval = _comm.get_sample_interval();

    // Read battery voltage
    float battery = averageAnalogRead(BATTERY) * REF3V3 / 1024.0;
    // R1 = 463K
    // R2 = 1490K
    battery = (battery * 1953) / 1490;
    if (battery < 3.5) {
        err |= ERR_BATTERY;
    }

    // Read UV sensor voltage
    float outputVoltage = averageAnalogRead(UVOUT) * REF3V3 / 1024.0;
    float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
    if (uvIntensity < 0) {
        uvIntensity = 0;
    }

    // Generate message
    String output;
    output = String(sample_interval) + SEP_CHAR;
    output += String(err, DEC) + SEP_CHAR;
    output += String(battery, 2) + SEP_CHAR;
    output += String(temperature, 2) + SEP_CHAR;
    output += String(humidity, 2) + SEP_CHAR;
    output += String(pressure, 2) + SEP_CHAR;
    output += String(lux) + SEP_CHAR;
    output += String(uvIntensity, 4);

    // Send message
    if (!_comm.send_msg(output)) {
        err |= ERR_COMM;
    }

    // Wait until next sample
    for (unsigned int i = 0 ;  i  <  sample_interval * 60 / 8; i++) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
}


// Takes an average of readings on a given pin
int averageAnalogRead(int pinToRead) {
    byte numberOfReadings = 8;
    unsigned int runningValue = 0;

    for(int i = 0; i < numberOfReadings; i++) {
        runningValue += analogRead(pinToRead);
        delay(200);
    }
    runningValue /= numberOfReadings;

    return(runningValue);
}


// The Arduino Map function but for floats
// From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
