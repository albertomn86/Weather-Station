#include <Arduino.h>
#include <Adafruit_BMP280.h> // BMP280_ADDRESS (0x76)
#include <Adafruit_SHT31.h>
#include <BH1750.h>
#include <LowPower.h>
#include "communication.h"

#define DEVICE_ID "45C5"

#define ERR_LED 5
#define HC12_AT 6
#define HC12_RX 10
#define HC12_TX 11
#define UVOUT   A0 // Output from the sensor
#define REF_3V3 A1 // 3.3V power on the Arduino board.
#define BATTERY A3 // Analog read for battery status.

#define ERR_PRESSURE 0x01
#define ERR_HUMIDITY 0x02
#define ERR_UV       0x04
#define ERR_LIGH     0x08
#define ERR_COMM     0x10

Adafruit_SHT31 _sht31 = Adafruit_SHT31();
Adafruit_BMP280 _bmp280;
BH1750 _bh1750;
Communication _comm = Communication(HC12_RX, HC12_TX);
byte err;


void setup() {
    err = 0;

    pinMode(ERR_LED, OUTPUT);
    pinMode(HC12_AT, OUTPUT);

    pinMode(UVOUT, INPUT);
    pinMode(REF_3V3, INPUT);

    _comm.begin(DEVICE_ID, HC12_AT);
    if (!_comm.pairing()) {
        err |= ERR_COMM;
    }

    if(!_sht31.begin(0x44)){
        err |= ERR_HUMIDITY;
    }

    if(!_bmp280.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)){
        err |= ERR_PRESSURE;
    }

    _bh1750.begin();

    if (err) {
        digitalWrite(ERR_LED, HIGH);
    }
}


void loop() {

    float humidity = _sht31.readHumidity();
    float temperature = _sht31.readTemperature();
    float pressure = _bmp280.readPressure();
    uint16_t lux = _bh1750.readLightLevel();
    int uvLevel = averageAnalogRead(UVOUT);
    int refLevel = averageAnalogRead(REF_3V3);
    unsigned int sample_interval = _comm.get_sample_interval();

    // Read battery voltage
    int battery = averageAnalogRead(BATTERY);
    float battery_level = (battery * 5) / 1024.0;

    // Read UV sensor voltage
    float outputVoltage = 3.3 / refLevel * uvLevel;
    float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
    if (uvIntensity < 0) uvIntensity = 0;

    // Generate message
    String output;
    output = String(sample_interval) + SEP_CHAR;
    output += String(err, DEC) + SEP_CHAR;
    output += String(battery_level, 2) + SEP_CHAR;
    output += String(temperature, 2) + SEP_CHAR;
    output += String(humidity, 2) + SEP_CHAR;
    output += String(pressure, 2) + SEP_CHAR;
    output += String(lux) + SEP_CHAR;
    output += String(uvIntensity, 4);

    // Send message
    if (!_comm.send_msg(output)) {
        err |= ERR_COMM;
    }

    // Check errors
    digitalWrite(ERR_LED, err? HIGH:LOW);

    // Wait until next sample
    for (unsigned int i = 0 ;  i  <  sample_interval * 60 / 8; i++) {
             LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }

}


// Takes an average of readings on a given pin
int averageAnalogRead(int pinToRead) {
    byte numberOfReadings = 8;
    unsigned int runningValue = 0;

    for(int x = 0 ; x < numberOfReadings ; x++) {
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
