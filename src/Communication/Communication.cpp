#include "Communication.h"

Communication::Communication(uint8_t rxPin, uint8_t txPin, uint8_t setPin) : radio(HC12(rxPin, txPin, setPin))
{
}

void Communication::begin(String device_id)
{
    station_id = device_id;
    interval = 300;
    radio.begin(9600);
    radio.set_mode(radio.FU1);
    radio.set_power(6);
}

bool Communication::receiveResponse()
{
    String response = radio.receive(1000, END_CHAR);
    String response_header = HEADER_RESPONSE + station_id;
    if (response.startsWith(response_header) && response.endsWith(String(END_CHAR)))
    {
        String payload = response.substring(response_header.length(), response.indexOf(END_CHAR));
        processPayload(payload);
        return true;
    }
    return false;
}

bool Communication::processPayload(String payload)
{
    char *payloadStr = const_cast<char *>(payload.c_str());
    char *pch = NULL;
    bool requiredFound = false;
    pch = strtok(payloadStr, ";");
    while (pch != NULL)
    {
        if (pch[0] == 'I')
        {
            interval = atoi(pch + 1);
            requiredFound = true;
        }
        pch = strtok(NULL, ";");
    }
    return requiredFound;
}

bool Communication::sendMessage(String data)
{
    String msg = HEADER_MSGDATA + station_id + data + END_CHAR;
    return Communication::sendFrame(msg);
}

bool Communication::sendFrame(String message)
{
    int attempts = 10;
    radio.wakeup();
    while (attempts > 0)
    {
        radio.send(message);
        if (receiveResponse())
        {
            radio.sleep();
            return true;
        }
        attempts--;
        delay(1000);
    }
    radio.sleep();
    return false;
}

unsigned int Communication::getSampleInterval(void)
{
    return interval;
}
