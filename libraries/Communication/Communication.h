/**
 *  @file communication.h
 *  @author Alberto MN
 *
 *  This is a library for the weather station communication.
 *
 *  Designed specifically to work with the HC-12 Wireless Serial Port Communication Module.
 */
#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <Arduino.h>
#include <SoftwareSerial.h>

#define HEADER_PAIRING  "S0;"
#define HEADER_MSGDATA  "S1;"
#define HEADER_RESPONSE "R0;"
#define SEP_CHAR ';'
#define END_CHAR '#'

class Communication {

    private:

        SoftwareSerial _serial;
        String station_id;
        String token;
        unsigned int interval;
        unsigned int AT_PIN;

        void sleep_mode(void);
        void wakeup(void);
        bool send(String message);
        bool receive(void);

    public:

        Communication(int rxPin, int txPin);
        void begin(String device_id, int atPin);
        bool pairing(void);
        bool send_msg(String data);
        unsigned int get_sample_interval(void);
        void setup_mode(void);
        void setup_power(short power);

};
#endif
