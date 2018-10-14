/* ========================================================================
 * SmartSerial - Arduino based library protocol with RS485 support
 * ========================================================================
 * The library consists of two classes:
 *   SmartSSP - the low level driver and protocol implementation
 *   SmartMSP - heir protocol implements the callback function
 * ------------------------------------------------------------------------
 * Features:
 * - Support for the STM32 core Arduino
 * - Support data filters by tags [MSP][DBG]
 * - Support work on the RS485 interface
 * - Flexible configuration callbacks
 * - Ready-made templates of fixed data types: event, request, error & etc.
 * - Transmission and reception of data volume limited by the RAM
 * ------------------------------------------------------------------------
 * License:
 * GNU General Public License v3.0
 * https://github.com/denisn73/SmartSerial/blob/master/LICENSE
 * ------------------------------------------------------------------------
 * Author:
 * Developed by Denis Silivanov
 * VK: @silivanov
 * Instagram: @denisfpv
 * Copyright (c) Denis Silivanov 2018
 * https://github.com/denisn73/SmartSerial
 * ------------------------------------------------------------------------
 */

#include "SmartSerial.h"

#ifdef _VARIANT_ARDUINO_STM32_
SmartSSP::SmartSSP(USBSerial* _serial) :
  usbSerial(_serial), _inputChar()
{
  construct();
}
#ifdef COMPOSITE_SERIAL_SUPPORT
SmartSSP::SmartSSP(USBCompositeSerial* _serial) :
  compositeSerial(_serial), _inputChar()
{
  construct();
}
#endif
#endif

SmartSSP::SmartSSP(HardwareSerial* _serial, int pinTXen) :
#ifndef _VARIANT_ARDUINO_STM32_
  serial(_serial),
#endif
  Hardwareserial(_serial), _pinTX(pinTXen), _inputChar()
{
  construct();
  isHardwareSerial = true;
}

void SmartSSP::construct() {
  inPacket.packetType = 0;
  inPacket.nodeID     = 0;
  inPacket.commandID  = 0;
  inPacket.parity     = 0;
  inPacket.datasize   = 0;
  _inCounter          = 0;
  if(isHardwareSerial) serial = Hardwareserial;
  else {
	#ifdef _VARIANT_ARDUINO_STM32_
	  if(usbSerial!=nullptr) serial = usbSerial;
	  #ifdef COMPOSITE_SERIAL_SUPPORT
	  if(compositeSerial!=nullptr) serial = compositeSerial;
	  #endif
	#endif
  }
}

/// Begin using default settings:
///  - speed: 115200 baud
///  - nodeID: 0
void SmartSSP::begin() {
  begin(DEFAULT_BAUDRATE, 0);
}

/// Begin using custom settings
void SmartSSP::begin(long baud, uint8_t nodeID) {
  if(isHardwareSerial) Hardwareserial->begin(_baud=baud);
<<<<<<< HEAD
  //else serial->begin(baud);
=======
  else Serial.begin(baud);
>>>>>>> parent of 229edcc... непонятные доработки
  setNodeID(nodeID);
  if(_pinTX != PIN_UNCONNECTED) {
	  pinMode(_pinTX, OUTPUT);
	  digitalWrite(_pinTX, LOW);
  }
}

// 
void SmartSSP::setCallbackTimeout(uint16_t _timeout) {
  _callbackTimeout = _timeout;
}

// 
void SmartSSP::setAnswerTimeout(uint16_t _timeout) {
  _answerTimeout = _timeout;
}

// Send outPacket
void SmartSSP::sendPacket() {
  
  outPacket.parity  = outPacket.packetType;
  outPacket.parity ^= outPacket.nodeID;
  outPacket.parity ^= outPacket.commandID;
  outPacket.parity ^= outPacket.datasize;
  for(int i=0; i<outPacket.datasize; i++) {
    outPacket.parity ^= outPacket.payload[i];
  }
  
  enableTX();
  
  #ifdef _VARIANT_ARDUINO_STM32_
  if(!serial->isConnected()) return;
  #endif
  
<<<<<<< HEAD
  
  if(isHardwareSerial) {
	  //Hardwareserial->println();
	  Hardwareserial->print(TAG_MSP);
	  Hardwareserial->print(TAG_TYPE);  hexPrinting(outPacket.packetType);
	  Hardwareserial->print(TAG_NODE);  hexPrinting(outPacket.nodeID);
	  Hardwareserial->print(TAG_CMD);   hexPrinting(outPacket.commandID);
	  Hardwareserial->print(TAG_SIZE);  hexPrinting(outPacket.datasize);
	  Hardwareserial->print(TAG_DATA);  for(int i=0; i<outPacket.datasize; i++) hexPrinting(outPacket.payload[i]);
	  Hardwareserial->print(TAG_CRC);   hexPrinting(outPacket.parity);
	  Hardwareserial->println();

  } else {
	  #ifdef _VARIANT_ARDUINO_STM32_
	    if(usbSerial!=nullptr && !usbSerial->isConnected()) return;
	    #ifdef COMPOSITE_SERIAL_SUPPORT
	    if(compositeSerial!=nullptr && !compositeSerial->isConnected()) return;
	    #endif
	  #endif
	  //serial->println();
	  serial->print(TAG_MSP);
	  serial->print(TAG_TYPE);  hexPrinting(outPacket.packetType);
	  serial->print(TAG_NODE);  hexPrinting(outPacket.nodeID);
	  serial->print(TAG_CMD);   hexPrinting(outPacket.commandID);
	  serial->print(TAG_SIZE);  hexPrinting(outPacket.datasize);
	  serial->print(TAG_DATA);  for(int i=0; i<outPacket.datasize; i++) hexPrinting(outPacket.payload[i]);
	  serial->print(TAG_CRC);   hexPrinting(outPacket.parity);
	  serial->println();
  }
=======
  //serial->println();
  serial->print(TAG_MSP);
  serial->print(TAG_TYPE);  hexPrinting(outPacket.packetType);
  serial->print(TAG_NODE);  hexPrinting(outPacket.nodeID);
  serial->print(TAG_CMD);   hexPrinting(outPacket.commandID);
  serial->print(TAG_SIZE);  hexPrinting(outPacket.datasize);
  serial->print(TAG_DATA);  for(int i=0; i<outPacket.datasize; i++) hexPrinting(outPacket.payload[i]);
  serial->print(TAG_CRC);   hexPrinting(outPacket.parity);
  serial->println();
>>>>>>> parent of 229edcc... непонятные доработки
  
}

/// 
bool SmartSSP::handle() {
  static uint8_t processDataFlag = false;
  if(isTX() && (micros() > _txMicros)) disableTX();
  _ready = false;
<<<<<<< HEAD
  int available;
  if(isHardwareSerial) available = Hardwareserial->available();
  else available = serial->available();
  #ifdef DEBUG_SERIAL
  //if(available) if(debugPort!=nullptr) debugPort->debug("Available MSP data : ", available);
  #endif
  while(available) {
    char inChar;
	if(isHardwareSerial) inChar = (char) Hardwareserial->read();
	else inChar = (char) serial->read();
=======
  while(serial->available()) {
    char inChar = (char) serial->read();
>>>>>>> parent of 229edcc... непонятные доработки
    if(inChar != '\n') {
      if(inChar != '\r') {
        if(_inCounter<5) _checkMSP += inChar;
        else if(_inCounter<MSP_INPUT_BUFFER_SIZE+5) {
			_inputChar[_inCounter-5] = inChar;
		}
		_inCounter++;
      }
    } else {
	  #ifdef DEBUG_SERIAL
	  if(debugPort!=nullptr) debugPort->debug("End of line available : ", available);
	  if(debugPort!=nullptr) debugPort->debug("Data : ", _inputChar);
	  #endif
      if(_checkMSP.equals(TAG_MSP)) {
	    #ifdef DEBUG_SERIAL
		if(debugPort!=nullptr) debugPort->debug("Has MSP data");
		#endif
        if(parseData()) {
          //serial->flush();
		  if(_callbackTimeout) {
			  processDataFlag = true;
			  _callbackTimeoutMicros = micros() + _callbackTimeout;
		  } else processData();
          _ready     = true;
          _checkMSP  = "";
          _inCounter = 0;
          return _ready;
        }
        else { // Parse packet error
		  error();
		  if(_errorUsage) sendError();
        }
      } else {
		  
	  }
      _checkMSP = "";
      _inCounter = 0;
    }
<<<<<<< HEAD
    if(isHardwareSerial) {
		delay(1);
		available = Hardwareserial->available();
	}
    else available = serial->available();
	
=======
>>>>>>> parent of 229edcc... непонятные доработки
  }
  if(processDataFlag && (micros() >= _callbackTimeoutMicros)) {
	  processDataFlag = false;
      processData();
  }
  handler();
  return _ready;
}

void SmartSSP::processData() {
  long _payload;
  switch(inPacket.packetType) {
    case TYPE_REPLY :
      break;
    case TYPE_ERROR :
	  sendPacket();
      break;
    case TYPE_EVENT :
      _payload  = inPacket.payload[0] << 24;
      _payload += inPacket.payload[1] << 16;
      _payload += inPacket.payload[2] << 8;
      _payload += inPacket.payload[3];
	  event(inPacket.commandID, _payload);
      break;
    case TYPE_ARRAY :
	  array(inPacket.commandID, inPacket.payload, inPacket.datasize);
      break;
    case TYPE_VALUE :
	  _payload  = inPacket.payload[0] << 24;
      _payload += inPacket.payload[1] << 16;
      _payload += inPacket.payload[2] << 8;
      _payload += inPacket.payload[3];
	  value(inPacket.commandID, _payload);
      break;
    case TYPE_REQUEST :
	  request(inPacket.commandID);
      break;
    case TYPE_RESET :
	  reset();
      break;
    default : break;
  }
  
}

bool SmartSSP::parseData() {
  if(_inputChar[0] != 'T') return false;
  #ifdef DEBUG_SERIAL
  if(debugPort!=nullptr) debugPort->debug("Parse MSP data");
  #endif
  _checkedParity = 0;
  _checkedParity ^= inPacket.packetType = hex_to_dec(_inputChar[1])*16  + hex_to_dec(_inputChar[2]);
  _checkedParity ^= inPacket.nodeID     = hex_to_dec(_inputChar[4])*16  + hex_to_dec(_inputChar[5]);
  _checkedParity ^= inPacket.commandID  = hex_to_dec(_inputChar[7])*16  + hex_to_dec(_inputChar[8]);
  _checkedParity ^= inPacket.datasize   = hex_to_dec(_inputChar[10])*16 + hex_to_dec(_inputChar[11]);
  if(inPacket.payload) delete[] inPacket.payload;
  inPacket.payload = new uint8_t[inPacket.datasize];
  for(int i=0; i<inPacket.datasize; i++) {
	unsigned int indexPayload = i*2+13;
	inPacket.payload[i] = hex_to_dec(_inputChar[indexPayload])*16 + hex_to_dec(_inputChar[indexPayload+1]);
    _checkedParity ^= inPacket.payload[i];
  }
  unsigned int indexParity = inPacket.datasize*2+14;
  inPacket.parity = hex_to_dec(_inputChar[indexParity])*16 + hex_to_dec(_inputChar[indexParity+1]);
  #ifdef DEBUG_SERIAL
	if(debugPort!=nullptr) debugPort->debug("MSP", "inPacket.datasize", inPacket.datasize);
	if(debugPort!=nullptr) debugPort->debug("MSP", "indexParity",       indexParity);
	if(debugPort!=nullptr) debugPort->debug("MSP", "inPacket.parity",   inPacket.parity);
	if(debugPort!=nullptr) debugPort->debug("MSP", "_inputChar",       _inputChar);
    printInfo();
  #endif
  return (_checkedParity == inPacket.parity);
}

/// 
bool SmartSSP::available() {
  return serial->available();
}

/// HexPrinting: helper function to print data with a constant field width (1 hex values)
void SmartSSP::hexPrinting(uint8_t& data) {
<<<<<<< HEAD
  if(isHardwareSerial) {
	  if(data<16) Hardwareserial->print(0);
	  Hardwareserial->print(data, HEX);
  } else {
	  if(data<16) serial->print(0);
	  serial->print(data, HEX);
  }
=======
  if(data<16) serial->print(0, HEX);
  serial->print(data, HEX);
>>>>>>> parent of 229edcc... непонятные доработки
}

/// HexPrinting: helper function to print data with a constant field width (2 hex values)
void SmartSSP::hexPrinting(int16_t& data) {
<<<<<<< HEAD
  if(isHardwareSerial) {
	  if(data<4096) Hardwareserial->print(0);
	  if(data<256)  Hardwareserial->print(0);
	  if(data<16)   Hardwareserial->print(0);
	  Hardwareserial->print(uint16_t(data), HEX);
  } else {
	  if(data<4096) serial->print(0);
	  if(data<256)  serial->print(0);
	  if(data<16)   serial->print(0);
	  serial->print(uint16_t(data), HEX);
  }
=======
  if(data<4096) serial->print(0, HEX);
  if(data<256)  serial->print(0, HEX);
  if(data<16)   serial->print(0, HEX);
  serial->print(uint16_t(data), HEX);              // casting to suppress FFFF for negative int values
>>>>>>> parent of 229edcc... непонятные доработки
}

/// Convert HEX to Decimal
#define HEX_DEC_ERROR 0
uint8_t SmartSSP::hex_to_dec(uint8_t in) {
  if(((in >= '0') && (in <= '9'))) return in-'0';
  in |= 0x20;
  if(((in >= 'a') && (in <= 'f'))) return in-'a' + 10;
  return HEX_DEC_ERROR;
}

void SmartSSP::printHexPayload() {
#ifdef DEBUG_SERIAL
	String hexPayload = "";
	for(uint8_t i=0; i<inPacket.datasize; i++) {
		if(inPacket.payload[i] < 16) hexPayload += "0";
		hexPayload += String(inPacket.payload[i], HEX);
	}
	if(debugPort!=nullptr) debugPort->debug("Payload:   " + hexPayload);//printlnDebug(inPacket.payload,HEX);
#endif
}

/// printInfo:
void SmartSSP::printInfo() {
#ifdef DEBUG_SERIAL
  if(debugPort!=nullptr) debugPort->debug(">>> inPacket info <<<");
  if(debugPort!=nullptr) debugPort->debug("Type:      " + String(inPacket.packetType, HEX));
  if(debugPort!=nullptr) debugPort->debug("NodeID:    " + String(inPacket.nodeID,     HEX));
  if(debugPort!=nullptr) debugPort->debug("CommandID: " + String(inPacket.commandID,  HEX));
  if(debugPort!=nullptr) debugPort->debug("Datasize:  " + String(inPacket.datasize,   HEX));
  printHexPayload();
  if(debugPort!=nullptr) debugPort->debug("Parity:    " + String(inPacket.parity,     HEX));
  if(debugPort!=nullptr) debugPort->debug("Checked:   " + String(_checkedParity,      HEX));
#endif
}

/// Set nodeID
void SmartSSP::setNodeID(uint8_t& nodeID) {
  outPacket.nodeID = nodeID;
}

/// Set commandID
void SmartSSP::setCommandID(uint8_t& commandID) {
  outPacket.commandID = commandID;
}

/// Set packet type
void SmartSSP::setPacketType(uint8_t type) {
  outPacket.packetType = type;
}

/// Get packet type
uint8_t SmartSSP::getSize() {
  return inPacket.datasize;
}

/// Get packet type
uint8_t SmartSSP::getPacketType() {
  return inPacket.packetType;
}

/// Get commandID
uint8_t SmartSSP::getCommandID() {
  return inPacket.commandID;
}

/// Get getPayload
uint8_t SmartSSP::getPayload(byte num) {
  return inPacket.payload[num];
}

/// Get getPayload
uint8_t* SmartSSP::getPayload() {
  return &inPacket.payload[0];
}
