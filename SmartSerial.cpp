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
  serial(_serial), _inputChar()
{
  construct();
}
#endif

SmartSSP::SmartSSP(HardwareSerial* _serial, int pinTXen) :
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
  else Serial.begin(baud);
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

/// 
bool SmartSSP::handle() {
  static uint8_t processDataFlag = false;
  if(isTX() && (micros() > _txMicros)) disableTX();
  _ready = false;
  while(serial->available()) {
    char inChar = (char) serial->read();
    if(inChar != '\n') {
      if(inChar != '\r') {
        if(_inCounter<5) _checkMSP += inChar;
        else _inputChar[_inCounter-5] = inChar;
        _inCounter++;
      }
    } else {
      if(_checkMSP.equals(TAG_MSP)) {
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
  if(data<16) serial->print(0, HEX);
  serial->print(data, HEX);
}

/// HexPrinting: helper function to print data with a constant field width (2 hex values)
void SmartSSP::hexPrinting(int16_t& data) {
  if(data<4096) serial->print(0, HEX);
  if(data<256)  serial->print(0, HEX);
  if(data<16)   serial->print(0, HEX);
  serial->print(uint16_t(data), HEX);              // casting to suppress FFFF for negative int values
}

/// Convert HEX to Decimal
#define HEX_DEC_ERROR 0
uint8_t SmartSSP::hex_to_dec(uint8_t in) {
  if(((in >= '0') && (in <= '9'))) return in-'0';
  in |= 0x20;
  if(((in >= 'a') && (in <= 'f'))) return in-'a' + 10;
  return HEX_DEC_ERROR;
}

/// printInfo:
void SmartSSP::printInfo() {
#ifdef DEBUG_SERIAL
  printlnDebug("[DBG] >>> inPacket info <<<");
  printDebug  ("[DBG] Type:      "); printlnDebug(inPacket.packetType, HEX);
  printDebug  ("[DBG] NodeID:    "); printlnDebug(inPacket.nodeID,     HEX);
  printDebug  ("[DBG] CommandID: "); printlnDebug(inPacket.commandID,  HEX);
  printDebug  ("[DBG] Datasize:  "); printlnDebug(inPacket.datasize,   HEX);
  printDebug  ("[DBG] Payload:   "); printlnDebug("...");//printlnDebug(inPacket.payload,HEX);
  printDebug  ("[DBG] Parity:    "); printlnDebug(inPacket.parity,     HEX);
  printDebug  ("[DBG] Checked:   "); printlnDebug(_checkedParity,      HEX);
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
