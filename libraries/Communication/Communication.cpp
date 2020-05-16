#include "Communication.h"

/**
 * @brief Construct a new Communication object.
 *
 * @param rxPin RXD pin on radio module
 * @param txPin TXD pin on radio module
 * @param txPin SET pin on radio module
 */
Communication::Communication(uint8_t rxPin, uint8_t txPin, uint8_t setPin) :
    radio(HC12(rxPin, txPin, setPin))
{}


/**
 * @brief Initialize the communication with the HC12 module
 *
 * @param device_id Device ID used for communication
 */
void Communication::begin(String device_id)
{
    station_id = device_id;
    interval = 5;
    radio.begin(9600);
    radio.set_mode(radio.FU1);
    radio.set_power(6);
}


/**
 * @brief Receive ACK message.
 *
 */
bool Communication::receive_ack() {

    String response_header = HEADER_RESPONSE + station_id + SEP_CHAR;

    String response = radio.receive(1000, END_CHAR);

    // Check received response.
    if (response.startsWith(response_header) && response.endsWith(String(END_CHAR))) {
        String payload = response.substring(response_header.length(), response.indexOf(END_CHAR));
        token = payload.substring(0, payload.indexOf(SEP_CHAR));
        interval = atol(payload.c_str() + token.length() + 1);
        return true;
    }
    return false;
}

/**
 * @brief Send message to receiver.
 *
 * @return true on success, false otherwise.
 */
bool Communication::send(String message) {
    int attempts = 10;
    radio.wakeup();
    while(attempts > 0) {
        radio.send(message);
        // Wait for reply.
        if (receive_ack()) {
            radio.sleep();
            return true;
        }
        // Next attempt.
        attempts--;
    }
    radio.sleep();
    return false;
}


/**
 * @brief Send data to receiver.
 *
 * @param data Data to be sent.
 * @return true on success, false otherwise.
 */
bool Communication::send_msg(String data) {
    // Check if the station is already paired.
    if (token.length() > 0) {
        String msg = HEADER_MSGDATA + station_id + SEP_CHAR + token + SEP_CHAR + data + END_CHAR;
        return Communication::send(msg);
    }
    return false;
}


/**
 * @brief Send pairing message to receiver.
 *
 * @return true on success, false otherwise.
 */
bool Communication::pairing() {
    String pair_msg = HEADER_PAIRING + station_id + END_CHAR;
    return Communication::send(pair_msg);
}


/**
 * @brief Get the sample interval.
 *
 * @return Sample interval in milliseconds.
 */
unsigned int Communication::get_sample_interval(void) {
    return interval;
}
