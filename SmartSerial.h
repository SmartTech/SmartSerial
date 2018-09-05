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
/* ------------------------------------------------------------------------
 * Release Notes:
 * ------------------------------------------------------------------------
 *   SmartSerial v1.0 :
 *    - First release to open source
 * ------------------------------------------------------------------------
 *   SmartSerial v1.1 :
 *    - Change 'serial' object from Stream* to USBSerial*
 *    - Fixed USBSerial buffer overflow by add "if(!serial->isConnected())"
 * ------------------------------------------------------------------------
 *   SmartSerial v1.2 :
 *    - Add CompositeSerial support (COMPOSITE_SERIAL_SUPPORT)
 *    - Fix debug() with 4 args
 * ------------------------------------------------------------------------
 */

#ifndef _SMART_SERIAL_H_
#define _SMART_SERIAL_H_

#include <Arduino.h>

#define COMPOSITE_SERIAL_SUPPORT

//#define DEBUG_SERIAL
#ifdef DEBUG_SERIAL
//#include <SmartDebug.h>
#endif

#ifdef COMPOSITE_SERIAL_SUPPORT
#include <USBComposite.h>
#endif

#define PIN_UNCONNECTED      -1

#define DEFAULT_BAUDRATE 115200

// status
#define CONNECTED          true
#define DISCONNECTED      false

// tags:
#define TAG_DBG         "[DBG] "
#define TAG_MSP         "[MSP]"
#define TAG_TYPE            "T"
#define TAG_NODE            "N"
#define TAG_CMD             "I"
#define TAG_SIZE            "S"
#define TAG_DATA            "P"
#define TAG_CRC             "Q"

// packet types:
#define TYPE_REPLY         0x01
#define TYPE_EVENT         0x02
#define TYPE_ARRAY         0x03
#define TYPE_VALUE         0x04
#define TYPE_REQUEST       0x05
#define TYPE_ERROR         0x07
#define TYPE_RESET         0x1F

#ifndef nullptr
#define nullptr            0x00
#endif

#define MSP_INPUT_BUFFER_SIZE    256

#ifndef SERIAL_USB
struct USBSerial {
	template<typename... ARGS> void begin(ARGS...) {}
	template<typename... ARGS> void print(ARGS...) {}
	template<typename... ARGS> void println(ARGS...) {}
	template<typename... ARGS> uint8_t isConnected(ARGS...) {return true;}
	template<typename... ARGS> uint8_t available(ARGS...) {}
	template<typename... ARGS> uint8_t read(ARGS...) {}
};
#endif

// -------------------------------------------
// SmartSSP - Smart Serial Protocol
// -------------------------------------------
class SmartSSP {
  
  private:
    void construct();
    bool isHardwareSerial = false;
    HardwareSerial* Hardwareserial;
    #ifdef _VARIANT_ARDUINO_STM32_
	USBSerial*           usbSerial = nullptr;
	#ifdef COMPOSITE_SERIAL_SUPPORT
	USBCompositeSerial*  compositeSerial = nullptr;
	#endif
	#endif
	
	Stream* serial;
		
    bool    parseData();
    void    processData();
    void    sendPacket();
	void    printHexPayload();
    void    printInfo();
    void    hexPrinting(uint8_t& data);
    void    hexPrinting(int16_t& data);
    uint8_t hex_to_dec(uint8_t in);
    
    void    setPacketType(uint8_t type);
    void    setCommandID(uint8_t& commandID);
    void    setSensorID(uint8_t& sensorID);
    void    setNodeID(uint8_t& nodeID);
	    
    struct Packet {
      uint8_t  packetType;
      uint8_t  nodeID;
      uint8_t  commandID;
      uint8_t  datasize;
      uint8_t  parity;
	  uint8_t* payload = nullptr;
    } inPacket, outPacket;
    
	int      _pinTX = PIN_UNCONNECTED;
    char     _inputChar[MSP_INPUT_BUFFER_SIZE];
    uint8_t  _inCounter;
    uint8_t  _checkedParity;
    String   _checkMSP              = "";
    bool     _ready                 = false;
	bool     _txEnabled             = false;
	uint16_t _callbackTimeout       = false;
	uint16_t _answerTimeout         = false;
	uint32_t _txMicros              = false;
	uint32_t _baud                  = DEFAULT_BAUDRATE;
	uint32_t _callbackTimeoutMicros = false;
	uint32_t _answerTimeoutMicros   = false;
    bool     _errorUsage            = true;
	uint8_t  _isDebug               = false;
	
	virtual void request(int) {}
	virtual void value(int, int) {}
	virtual void array(int, uint8_t*, int) {}
	virtual void event(int, int) {}
	virtual void error() {}
	virtual void reset() {}
	virtual void handler() {}
	
	void enableTX() {
		if(_pinTX != PIN_UNCONNECTED) {
			digitalWrite(_pinTX, _txEnabled=HIGH);
			uint16_t outDataSize = (outPacket.datasize*2) + strlen(TAG_MSP) + 20;
			uint16_t packetMicros = (10000000 / _baud) * outDataSize;
                        if(micros()<_txMicros) _txMicros += packetMicros;
                        else _txMicros = micros() + packetMicros;
		}
	}
	
	void disableTX() {
		if(_pinTX != PIN_UNCONNECTED) digitalWrite(_pinTX, _txEnabled=LOW);
	}
	
    void sendError() {
      if(outPacket.payload) delete[] outPacket.payload;
      outPacket.packetType = TYPE_ERROR;
      outPacket.commandID  = 0;
      outPacket.datasize   = 1;
      outPacket.datasize   = 1;
	  outPacket.payload = new uint8_t[1];
      outPacket.payload[0] = 0;
      sendPacket();
    }
		
  public:
  
    SmartSSP(HardwareSerial* _serial, int pinTXen = PIN_UNCONNECTED);
    #ifdef _VARIANT_ARDUINO_STM32_
      SmartSSP(USBSerial*    _serial);
	  #ifdef COMPOSITE_SERIAL_SUPPORT
	  SmartSSP(USBCompositeSerial*    _serial);
	  #endif
    #endif
	
    void     begin();
    void     begin(long baud, uint8_t nodeID = 0);
    bool     handle();
	void     setCallbackTimeout(uint16_t _timeout);
	void     setAnswerTimeout(uint16_t _timeout);
	    
    bool     available();
    uint8_t  getDataID();
    uint8_t  getCommandID();
    uint8_t  getPacketType();
    uint8_t  getSize();
    uint8_t  getPayload(byte num);
    uint8_t* getPayload();
	
	uint8_t isTX() {
		return _txEnabled;
	}
	
	void setErrorUsage(uint8_t _state) {
		_errorUsage = _state;
	}
    
    template < typename T >
    void sendData(uint8_t  dataID, T dataArray, uint8_t length) {
      if(outPacket.payload) delete[] outPacket.payload;
      outPacket.packetType = TYPE_ARRAY;
      outPacket.commandID  = dataID;
      outPacket.datasize   = length;
	  outPacket.payload    = new uint8_t[length];
      for(int i=0 ; i<length ; i++) {
        outPacket.payload[i] = ((uint8_t*)dataArray)[i];
      }
      sendPacket();
    }
    
    template < typename T >
    void sendData(uint8_t  dataID, T payload) {
      if(outPacket.payload) delete[] outPacket.payload;
      outPacket.packetType = TYPE_VALUE;
      outPacket.commandID  = dataID;
      outPacket.datasize   = 4;
	  outPacket.payload = new uint8_t[4];
      outPacket.payload[0] = payload >> 24;
      outPacket.payload[1] = payload >> 16;
      outPacket.payload[2] = payload >> 8;
      outPacket.payload[3] = payload >> 0;
      sendPacket();
    }
    
    template < typename T >
    void sendCommand(uint8_t dataID, T payload) {
      if(outPacket.payload) delete[] outPacket.payload;
      outPacket.packetType = TYPE_EVENT;
      outPacket.commandID  = dataID;
      outPacket.datasize   = 4;
	  outPacket.payload = new uint8_t[4];
      outPacket.payload[0] = payload >> 24;
      outPacket.payload[1] = payload >> 16;
      outPacket.payload[2] = payload >> 8;
      outPacket.payload[3] = payload >> 0;
      sendPacket();
    }
	
	void sendRequast(uint8_t dataID) {
	  if(outPacket.payload) delete[] outPacket.payload;
      outPacket.packetType = TYPE_REQUEST;
      outPacket.commandID  = dataID;
      outPacket.datasize   = 1;
	  outPacket.payload    = new uint8_t[1];
      outPacket.payload[0] = 0;
      sendPacket();
	  if(_answerTimeout) {
		  _answerTimeoutMicros = micros() + _answerTimeout;
	  }
	}
	
	void sendReset() {
      if(outPacket.payload) delete[] outPacket.payload;
      outPacket.packetType = TYPE_RESET;
      outPacket.commandID  = 0;
      outPacket.datasize   = 1;
	  outPacket.payload    = new uint8_t[1];
      outPacket.payload[0] = 0;
      sendPacket();
    }
	    
    //////////////////////////////////////////////////////////////
	
	void setDebug(uint8_t debugState) {
		_isDebug = debugState;
	}

    void    simplePrint() {}   
    template<class... T>
    void simplePrint(T... args) {
		if(!_isDebug) return;
		#ifdef _VARIANT_ARDUINO_STM32_
	    if(usbSerial!=nullptr && !usbSerial->isConnected()) return;
	    #ifdef COMPOSITE_SERIAL_SUPPORT
	    if(compositeSerial!=nullptr && !compositeSerial->isConnected()) return;
	    #endif
		#endif
		if(isHardwareSerial) Hardwareserial->print(args...);
        else serial->print(args...);
    }
	
	template<class F, class S, class... T >
	void debug(F arg, S index, T... args) {
		if(!_isDebug) return;
		#ifdef _VARIANT_ARDUINO_STM32_
	    if(usbSerial!=nullptr && !usbSerial->isConnected()) return;
	    #ifdef COMPOSITE_SERIAL_SUPPORT
	    if(compositeSerial!=nullptr && !compositeSerial->isConnected()) return;
	    #endif
		#endif
		if(isHardwareSerial)  {
			Hardwareserial->print(TAG_DBG);
			Hardwareserial->print(arg);
			Hardwareserial->print("[");
			Hardwareserial->print(index);
			Hardwareserial->print("]");
			if(sizeof...(args) > 0) {
			  Hardwareserial->print(": ");
			  simplePrint(args...);
			}
			Hardwareserial->println();
		} else {
			serial->print(TAG_DBG);
			serial->print(arg);
			serial->print("[");
			serial->print(index);
			serial->print("]");
			if(sizeof...(args) > 0) {
			  serial->print(": ");
			  simplePrint(args...);
			}
			serial->println();
		}
    };
    
    template<class F, class... T>
    void debug(F arg, T... args) {
		if(!_isDebug) return;
		#ifdef _VARIANT_ARDUINO_STM32_
	    if(usbSerial!=nullptr && !usbSerial->isConnected()) return;
	    #ifdef COMPOSITE_SERIAL_SUPPORT
	    if(compositeSerial!=nullptr && !compositeSerial->isConnected()) return;
	    #endif
		#endif
		if(isHardwareSerial)  {
			Hardwareserial->print(TAG_DBG);
			Hardwareserial->print(arg);
			if(sizeof...(args) > 0) {
			  Hardwareserial->print(": ");
			  simplePrint(args...);
			}
			Hardwareserial->println();
		} else {
			serial->print(TAG_DBG);
			serial->print(arg);
			if(sizeof...(args) > 0) {
			  serial->print(": ");
			  simplePrint(args...);
			}
			serial->println();
		}
    };
	
	template<class F>
    void debug(F arg, uint32_t size) {
		if(!_isDebug) return;
		#ifdef _VARIANT_ARDUINO_STM32_
	    if(usbSerial!=nullptr && !usbSerial->isConnected()) return;
	    #ifdef COMPOSITE_SERIAL_SUPPORT
	    if(compositeSerial!=nullptr && !compositeSerial->isConnected()) return;
	    #endif
		#endif
		if(isHardwareSerial)  {
			Hardwareserial->print(TAG_DBG);
			Hardwareserial->print(arg);
			Hardwareserial->println(size);
			//Hardwareserial->println(arg, size); // ???
		} else {
			serial->print(TAG_DBG);
			serial->print(arg);
			serial->println(size);
			//serial->println(arg, size); // ???
		}
    };
    
    template<class... T>
    void error(T... args) {
	    #ifdef _VARIANT_ARDUINO_STM32_
	    if(usbSerial!=nullptr && !usbSerial->isConnected()) return;
	    #ifdef COMPOSITE_SERIAL_SUPPORT
	    if(compositeSerial!=nullptr && !compositeSerial->isConnected()) return;
	    #endif
	    #endif
	    if(isHardwareSerial)  {
		    Hardwareserial->print(TAG_DBG);
		    Hardwareserial->print("error: ");
		    Hardwareserial->println(args...);
		} else {
		    serial->print(TAG_DBG);
		    serial->print("error: ");
		    serial->println(args...);
		}
    };

};

// -------------------------------------------
// SmartMSP - Smart Main Serial Protocol
// is extended functions of SmartSSP
// -------------------------------------------
class SmartMSP : public SmartSSP {
		
	private :
	
    void (*user_onRequest)(int) = nullptr;
    void (*user_onArray)(int, uint8_t*, int) = nullptr;
    void (*user_onEvent)(int, int) = nullptr;
    void (*user_onValue)(int, int) = nullptr;
    void (*user_onError)() = nullptr;
    void (*user_onReset)() = nullptr;
	
	void request(int id) override {
		if (user_onRequest) user_onRequest(id);
	}
	void value(int id, int payload) override {
		if (user_onValue) user_onValue(id, payload);
	}
	void array(int id, uint8_t* payload, int size) override {
		if (user_onArray) user_onArray(id, payload, size);
	}
	void event(int id, int value) override {
		if (user_onEvent) user_onEvent(id, value);
	}
	void error() override {
		if (user_onError) user_onError();
	}
	void reset() override {
		if (user_onReset) user_onReset();
	}
	
	public :
	
	SmartMSP(HardwareSerial* _serial, int pinTXen = PIN_UNCONNECTED) : SmartSSP(_serial, pinTXen) {}
    #ifdef _VARIANT_ARDUINO_STM32_
      SmartMSP(USBSerial*       _serial) : SmartSSP(_serial) {}
	  #ifdef COMPOSITE_SERIAL_SUPPORT
	  SmartMSP(USBCompositeSerial* _serial) : SmartSSP(_serial) {}
	  #endif
    #endif
	
    void attachRequest(void (*function)(int)) { user_onRequest = function; }
    void attachValue(void (*function)(int, int)) { user_onValue = function; }
    void attachArray(void (*function)(int, uint8_t*, int)) { user_onArray = function; }
    void attachEvent(void (*function)(int, int)) { user_onEvent = function; }
    void attachError(void (*function)()) { user_onError = function; }
    void attachReset(void (*function)()) { user_onReset = function; }
	
	
};

// -------------------------------------------
// === SSP protocol description ===
// -------------------------------------------
/* T01N00I01S01P05Q1A\r\n
 * [T][01][N][00][I][01][S][FF][P][0000...01][Q][1A][\r][\n]
 * "T" - packet type (byte, int, array, event, requast)
 * "N" - nodeID      (remote device)
 * "I" - commandID   ()
 * "S" - datasize    (size of data)
 * "P" - payload     (data)
 * "Q" - parity      (crc)
 * 
 */
// -------------------------------------------

#endif // _SMART_SERIAL_H_
