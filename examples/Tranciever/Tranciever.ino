/* Tranciever - example show how to work with SmartSerial.
 * ------------------------------------------------------------------------
 * Description:
 * This file declares settings, variables and classes,
 * called initialization and processing functions.
 * All the basic configuration and operation of the protocol
 * is implemented in the file MSP.ino
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

#include <SmartSerial.h>   // Smart Serial library
#include <TaskScheduler.h> // TaskScheduler library from BoardManager

//--------------------------------------------------------------------------------------------
// *** MSP Configuration ***
//--------------------------------------------------------------------------------------------

#define MSP_SERIAL          Serial
#define MSP_BAUDRATE         38400
#define MSP_NODE_ID              1
#define RS485_TX_EN            PA0

//--------------------------------------------------------------------------------------------
// *** Create MSP class object ***
//--------------------------------------------------------------------------------------------
SmartMSP MSP(&MSP_SERIAL);
//SmartMSP GUI(&MSP_SERIAL, RS485_TX_EN); // Activate RS485

//--------------------------------------------------------------------------------------------
// *** Create taskManager class object --- external libraries (from Libraries Manager) ***
//--------------------------------------------------------------------------------------------
Scheduler taskManager;

//--------------------------------------------------------------------------------------------
// *** Create structure with variables ***
//--------------------------------------------------------------------------------------------
struct config_t {
  uint8_t      someVariable1; //
  uint8_t      someVariable2; //
  uint8_t      someVariable3; //
} config;

struct param_t {
  uint8_t      someVariable1; //
  uint8_t      someVariable2; //
  uint8_t      someVariable3; //
} value;

//--------------------------------------------------------------------------------------------
// *** Setup function ***
//--------------------------------------------------------------------------------------------
void setup() {

  // init task manager
  taskManager.init();
	
  setupMSP();

}

//--------------------------------------------------------------------------------------------
// *** Loop function ***
//--------------------------------------------------------------------------------------------
void loop() {
	
	static uint32_t lastMillis = 0;
	
	if(millis() - lastMillis >= 1000) {
		
		MSP.debug("Time", "millis", millis());
		
		lastMillis = millis();
	}

  taskManager.execute(); // process tasks
	
	MSP.handle(); // handle library process
	
}


