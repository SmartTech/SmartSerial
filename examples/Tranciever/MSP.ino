/* MSP - main serial protocol.
 * ------------------------------------------------------------------------
 * Description:
 * Here it processes work in the protocol
 * The example shows the operation of stream control
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

// --- MSP data ID's
#define MSP_ARRAY_CONFIG         1
#define MSP_ARRAY_PARAMS         2

// --- MSP Stream events
#define MSP_EVENT_STREAM_START   1
#define MSP_EVENT_STREAM_STOP    2
#define MSP_EVENT_STREAM_TYPE    3

// --- MSP Stream type flags
#define MSP_STREAM_NONE          0
#define MSP_STREAM_MAIN          1
#define MSP_STREAM_ALL           4

// --- MSP Stream timings
#define DEF_STREAM_RATE        100 // ms
#define MIN_STREAM_RATE          1 // ms

//--------------------------------------------------------------------------------------------
// *** prototypes ***
//--------------------------------------------------------------------------------------------
void tStreamMspHandle();
void streamMSP_stop();
void streamMSP_start(uint16_t rate = DEF_STREAM_RATE);
void arrayMSP(int id, uint8_t* payload, int size);
void eventMSP(int id, int _value);
void errorMSP();

//--------------------------------------------------------------------------------------------
// *** Create Stream Task object ***
//--------------------------------------------------------------------------------------------
Task streamMSP(DEF_STREAM_RATE, TASK_FOREVER, NULL);

uint8_t  streamType   = MSP_STREAM_NONE;
uint16_t streamRate   = DEF_STREAM_RATE;
uint32_t streamMicros = 0;

//--------------------------------------------------------------------------------------------
// *** Initialize MSP ***
//--------------------------------------------------------------------------------------------
void setupMSP() {
  
  // init class object
  MSP.begin();                            // Default: 115200, NodeID = 0
  //MSP.begin(MSP_BAUDRATE);              // User: baudrate, Default: NodeID = 0
  //MSP.begin(MSP_BAUDRATE, MSP_NODE_ID); // User: baudrate & NodeID
  
  // for RS485 usage
  //MSP.setCallbackTimeout(100);            // in ms
  //MSP.setAnswerTimeout(100);              // in ms
  
  // attach callbacks
  MSP.attachArray(arrayMSP);              // callback of recieved array data
  MSP.attachEvent(eventMSP);              // callback of recieved event data
  MSP.attachError(errorMSP);              // callback of recieved error
  
}

//--------------------------------------------------------------------------------------------
// *** parse packet error callback ***
//--------------------------------------------------------------------------------------------
void errorMSP() {
  MSP.debug("Parse packet error");
}

//--------------------------------------------------------------------------------------------
// *** event MSP ***
//--------------------------------------------------------------------------------------------
void eventMSP(int _id, int _value) {
  switch(_id) {
    case MSP_EVENT_STREAM_START : streamMSP_start(_value); break;
    case MSP_EVENT_STREAM_TYPE  : streamMSP_type(_value);  break;
    case MSP_EVENT_STREAM_STOP  : streamMSP_stop();        break;
    default: MSP.debug("Unknown MSP command");             break;
  }
}

//--------------------------------------------------------------------------------------------
// *** array MSP ***
//--------------------------------------------------------------------------------------------
void arrayMSP(int _id, uint8_t* _payload, int _size) {
  switch(_id) {
    case MSP_ARRAY_PARAMS : memcpy(&value, _payload, _size); break;
    case MSP_ARRAY_CONFIG : memcpy(&config, _payload, _size); break;
    default: break;
  }
}

//--------------------------------------------------------------------------------------------
// *** stream MSP start/stop/handle ***
//--------------------------------------------------------------------------------------------

// --- set stream type
void streamMSP_type(uint8_t type) {
  streamType = type;
}

// --- stop stream
void streamMSP_stop() {
  streamType = MSP_STREAM_NONE;
  if(streamMSP.isEnabled()) {
    streamMSP.disable();
    taskManager.deleteTask(streamMSP);
  }
  MSP.debug("StreamMSP", "stoped");
}

// --- start stream
void streamMSP_start(uint16_t rate) {
  // ---
  if(!rate) return streamMSP_stop();
  else if(rate < MIN_STREAM_RATE) streamRate = DEF_STREAM_RATE;
  else streamRate = rate;
  // ---
  if(streamType == MSP_STREAM_NONE) streamType = MSP_STREAM_MAIN;
  // ---
  taskManager.addTask(streamMSP);
  streamMSP.setInterval(rate);
  streamMSP.setCallback(&tStreamMspHandle);
  streamMSP.enable();
  MSP.debug("StreamMSP", "started");
}

// --- stream task handler
void tStreamMspHandle() {
  switch(streamType) {
    case MSP_STREAM_MAIN :
      MSP.sendData(MSP_ARRAY_PARAMS,   &value,    sizeof(value));
    break;
    case MSP_STREAM_ALL :
      MSP.sendData(MSP_ARRAY_CONFIG,   &value,    sizeof(value));
      MSP.sendData(MSP_ARRAY_PARAMS,   &config,   sizeof(config));
    break;
    default: streamMSP_stop(); break;
  }
}

