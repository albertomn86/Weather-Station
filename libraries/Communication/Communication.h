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
#include "../HC12/HC12.hpp"

#define HEADER_PAIRING  "S0;"
#define HEADER_MSGDATA  "S1;"
#define HEADER_RESPONSE "R0;"
#define SEP_CHAR ';'
#define END_CHAR '#'

class Communication {

    private:

        String station_id;
        String token;
        unsigned int interval;
        HC12 radio;
        bool send(String data);

    public:

        Communication(uint8_t rxPin, uint8_t txPin, uint8_t setPin);
        bool receive_ack(void);
        void begin(String device_id);
        bool pairing(void);
        bool send_msg(String data);
        unsigned int get_sample_interval(void);
        void setup_mode(void);
        void setup_power(short power);

};
#endif
