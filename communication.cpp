#include "communication.h"

/**
 * @brief Construct a new Communication object.
 *
 * @param rxPin RXD pin on HC12 module
 * @param txPin TXD pin on HC12 module
 */
Communication::Communication(int rxPin, int txPin): _serial(SoftwareSerial(rxPin, txPin)) {}


/**
 * @brief Initialize the communication with the HC12 module
 *
 * @param device_id Device ID used for communication
 * @param atPin AT pin on HC12
 */
void Communication::begin(String device_id, int atPin) {
    station_id = device_id;
    interval = 5;
    _serial.begin(9600);
    AT_PIN = atPin;
    digitalWrite(AT_PIN, HIGH);
    delay(100);
}


/**
 * @brief Sleep mode.
 *
 */
void Communication::sleep_mode(void) {
    digitalWrite(AT_PIN, LOW);
    delay(100);
    // Send command
    _serial.print("AT+SLEEP");
    delay(100);
    // Read response
    while(_serial.available() > 0) {
        char c = _serial.read();
    }
    digitalWrite(AT_PIN, HIGH);
    delay(100);
}


/**
 * @brief Wake Up.
 *
 */
void Communication::wakeup(void) {
    digitalWrite(AT_PIN, LOW);
    delay(100);
    // Send command
    _serial.print("AT");
    delay(100);
    // Read response
    while(_serial.available() > 0) {
        char c = _serial.read();
    }
    digitalWrite(AT_PIN, HIGH);
    delay(100);
}


/**
 * @brief Receive ACK message.
 *
 */
bool Communication::receive() {

    String response_header = HEADER_RESPONSE + station_id + SEP_CHAR;

    // Wait for reply.
    unsigned int timeout = 100;
    while(!_serial.available() && timeout > 0) {
        delay(50);
        timeout--;
    }
    // No response from receiver.
    if (!timeout) {
        return false;
    }

    String response;
    char byte = 0;
    _serial.setTimeout(1000);
    while(_serial.available() && byte != END_CHAR) {
        byte = char(_serial.read());
        response += byte;
    }

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
    wakeup();
    while(attempts > 0) {
        _serial.print(message);
        // Wait for reply.
        if (receive()) {
            sleep_mode();
            return true;
        }
        // Next attempt.
        delay(5000);
        attempts--;
    }
    sleep_mode();
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
        return send(msg);
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
    return send(pair_msg);
}


/**
 * @brief Get the sample interval.
 *
 * @return Sample interval in milliseconds.
 */
unsigned int Communication::get_sample_interval(void) {
    return interval;
}