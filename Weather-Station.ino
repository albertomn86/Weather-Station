#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <LowPower.h>
#include <Communication.h>

#define DEVICE_ID "45C5"

#define HC12_AT 9
#define HC12_RX 10
#define HC12_TX 11
#define UVOUT   A0 // Output from the sensor
#define BATTERY A3 // Analog read for battery status
#define REF3V3  3.31

#define ERR_BATTERY  0x01
#define ERR_TEMPHUM  0x02
#define ERR_UV       0x04
#define ERR_LIGH     0x08
#define ERR_COMM     0x10


Adafruit_BME280 _bme280;
BH1750 _bh1750;
Communication _comm = Communication(HC12_RX, HC12_TX);
byte err;


void setup() {
    Wire.begin();
    err = 0;
    pinMode(HC12_AT, OUTPUT);
    pinMode(UVOUT, INPUT);
    pinMode(BATTERY, INPUT);

    if (!_bme280.begin(0x76)) {  // BME280_ADDRESS (0x76)
        err |= ERR_TEMPHUM;
    }

    _bme280.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1, // Temperature
                        Adafruit_BME280::SAMPLING_X1, // Pressure
                        Adafruit_BME280::SAMPLING_X1, // Humidity
                        Adafruit_BME280::FILTER_OFF);

    if (!_bh1750.begin(BH1750::ONE_TIME_LOW_RES_MODE)) { // Measurement at 4 lux resolution
        err |= ERR_LIGH;
    }

    _comm.begin(DEVICE_ID, HC12_AT);
    if (!_comm.pairing()) {
        err |= ERR_COMM;
    }

    if (err) {
        digitalWrite(13, HIGH);
    }
}


void loop() {

    // Read samples from BME280
    _bme280.takeForcedMeasurement();
    float temperature = _bme280.readTemperature();
    float humidity = _bme280.readHumidity();
    float pressure = _bme280.readPressure() / 100.0F;

    // Read luminosity
    float lux = _bh1750.readLightLevel();

    // Get sample interval from last message
    unsigned int sample_interval = _comm.get_sample_interval();

    // Read battery voltage
    float battery = averageAnalogRead(BATTERY) * REF3V3 / 1024.0F;
    // R1 = 463K
    // R2 = 1490K
    battery = (battery * 1953.0F) / 1490.0F;
    if (battery < 3.5) {
        err |= ERR_BATTERY;
    }

    // Read UV sensor voltage
    float outputVoltage = averageAnalogRead(UVOUT) * REF3V3 / 1024.0F;
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
    output += String(lux, 2) + SEP_CHAR;
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
