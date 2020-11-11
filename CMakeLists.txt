
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arduino-cmake/cmake/ArduinoToolchain.cmake)
set(ARDUINO_SDK_PATH /Applications/Arduino.app/Contents/Java)
cmake_minimum_required(VERSION 2.8)
project(Weather-Station)

message( ${CMAKE_TOOLCHAIN_FILE} )

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libraries)

generate_arduino_firmware(weather-station
      SRCS  Weather-Station.ino
      BOARD pro)
