# SmartSerial:
Arduino based library of 'SmartSerial' protocol with RS485 support

## Features:
 - Support for the STM32 core Arduino
 - Support data filters by tags [MSP][DBG]
 - Support work on the RS485 interface
 - Flexible configuration callbacks
 - Ready-made templates of fixed data types: event, request, error & etc.
 - Transmission and reception of data volume limited by the RAM
 
## Classes:
 - SmartSSP - the low level driver and protocol implementation
 - SmartMSP - heir protocol implements the callback function
 
## Example:
 - The example shows the operation of stream control
 - here it processes work in the protocol
 - You need to install "TaskScheduler" library from 'Libraries Manager'

## License:
 [GNU General Public License v3.0](https://github.com/denisn73/SmartSerial/blob/master/LICENSE)

## Author:
 Copyright (c) Denis Silivanov 2018
 
 VK: [@silivanov](https://vk.com/silivanov)
 
 Instagram: [@denisfpv](https://www.instagram.com/denisfpv)
