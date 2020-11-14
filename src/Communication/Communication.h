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

#define HEADER_MSGDATA  "S"
#define HEADER_RESPONSE "K"
#define SEP_CHAR ';'
#define END_CHAR '#'

class Communication {

    private:

        String station_id;
        unsigned int interval;
        HC12 radio;
        bool sendFrame(String data);
        bool processPayload(String payload);

    public:

        Communication(uint8_t rxPin, uint8_t txPin, uint8_t setPin);
        bool receiveResponse(void);
        void begin(String device_id);
        bool sendMessage(String data);
        unsigned int getSampleInterval(void);
};
#endif
