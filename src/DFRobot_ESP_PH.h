/*
 * file DFRobot_ESP_PH.h * @ https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK
 *
 * Arduino library for Gravity: Analog pH Sensor / Meter Kit V2, SKU: SEN0161-V2
 * 
 * Based on the @ https://github.com/DFRobot/DFRobot_PH
 * Copyright   [DFRobot](http://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * ##################################################
 * ##################################################
 * ########## Fork on github by GreenPonik ##########
 * ############# ONLY ESP COMPATIBLE ################
 * ##################################################
 * ##################################################
 * 
 * version  V1.0
 * date  2019-05
 */

 #ifndef _DFROBOT_ESP_PH_H_
 #define _DFROBOT_ESP_PH_H_
 
 #include "Arduino.h"
 #include "EEPROM.h"
 
 #define PH_8_VOLTAGE 1300
 #define PH_6_VOLTAGE 1700
 #define PH_5_VOLTAGE 1800
 #define PH_3_VOLTAGE 2200
 #define PH_NEUTRAL_VOLTAGE 1500.0
 #define PH_ACID_VOLTAGE 2032.44
 #define PH_MAX_VOLTAGE 3000
 #define PH_SENSOR_MAX_RANGE 14
 #define ReceivedBufferLength_PH 30
 
 enum 
 {
     PH_CALIBRATION_MODE_ERROR,
     PH_CALIBRATION_MODE_READY,
     PH_CALIBRATION_MODE_RUNNING,
     PH_CALIBRATION_MODE_SAVE_AND_EXIT,
 };
 
 class DFRobot_ESP_PH
 {
 public:
     DFRobot_ESP_PH();
     ~DFRobot_ESP_PH();
     void begin(uint16_t eeprom_start_addr);
     void begin(float acidVoltage, float neutralVoltage);
     float readPH(float voltage, float temperature);
     void calibration(float voltage, float temperature, int mode);
     void calibration_by_serial_CMD(float voltage, float temperature, char *cmd);
     void calibration_by_serial_CMD(float voltage, float temperature);
     float acidVoltage;
     float neutralVoltage;
 
 private:
     boolean cmdSerialDataAvailable();
     byte cmdParse(const char *cmd);
     byte cmdParse();
     void phCalibration(byte mode);
     float _temperature;
     float _phValue;
     float _voltage;
     uint16_t _eepromStartAddress;
     char _cmdReceivedBuffer[ReceivedBufferLength_PH];
     byte _cmdReceivedBufferIndex;
 };
 
 #endif
 