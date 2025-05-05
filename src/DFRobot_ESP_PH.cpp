/*
 * file DFRobot_ESP_PH.cpp * @ https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK
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

#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "EEPROM.h"

DFRobot_ESP_PH::DFRobot_ESP_PH()
{
    this->_temperature = 25.0;
    this->_phValue = 7.0;
    this->acidVoltage = 2032.44;   // buffer solution 4.0 at 25C
    this->neutralVoltage = 1500.0; // buffer solution 7.0 at 25C
    this->_voltage = 1500.0;
}

DFRobot_ESP_PH::~DFRobot_ESP_PH()
{
}

void DFRobot_ESP_PH::begin(uint16_t eeprom_start_addr)
{
    this->_eepromStartAddress = eeprom_start_addr;
    this->neutralVoltage = EEPROM.readFloat(this->_eepromStartAddress);
    this->acidVoltage = EEPROM.readFloat(this->_eepromStartAddress + (int)sizeof(float));
    if (isnan(this->neutralVoltage) || isnan(this->acidVoltage))
    {
        this->neutralVoltage = PH_NEUTRAL_VOLTAGE;
        this->acidVoltage = PH_ACID_VOLTAGE;
        EEPROM.writeFloat(this->_eepromStartAddress, this->neutralVoltage);
        EEPROM.writeFloat(this->_eepromStartAddress + (int)sizeof(float), this->acidVoltage);
        EEPROM.commit();
    }
    Serial.println(F(">>>pH Calibration Values<<<"));
    Serial.print(F(">>>NeutralVoltage: "));
    Serial.print(this->neutralVoltage);
    Serial.print(F(", AcidVoltage: "));
    Serial.print(this->acidVoltage);
    Serial.println(F("<<<"));
}

void DFRobot_ESP_PH::begin(float acidVoltage, float neutralVoltage)
{
    this->acidVoltage = acidVoltage;
    this->neutralVoltage = neutralVoltage;
    EEPROM.writeFloat(this->_eepromStartAddress, this->neutralVoltage);
    EEPROM.writeFloat(this->_eepromStartAddress + (int)sizeof(float), this->acidVoltage);
    EEPROM.commit();
    Serial.println(F(">>>pH Calibration Values<<<"));
    Serial.print(F(">>>NeutralVoltage: "));
    Serial.print(this->neutralVoltage);
    Serial.print(F(", AcidVoltage: "));
    Serial.print(this->acidVoltage);
    Serial.println(F("<<<"));
}

float DFRobot_ESP_PH::readPH(float voltage, float temperature)
{
    // Serial.print("_neutraVoltage:");
    // Serial.print(this->neutralVoltage);
    // Serial.print(", acidVoltage:");
    // Serial.print(this->acidVoltage);
    float slope = (7.0 - 4.0) / ((this->neutralVoltage - 1500.0) / 3.0 - (this->acidVoltage - 1500.0) / 3.0); // two point: (neutralVoltage,7.0),(acidVoltage,4.0)
    float intercept = 7.0 - slope * (this->neutralVoltage - 1500.0) / 3.0;
    // Serial.print(", slope:");
    // Serial.print(slope);
    // Serial.print(", intercept:");
    // Serial.println(intercept);
    this->_phValue = slope * (voltage - 1500.0) / 3.0 + intercept; // y = k*x + b
    Serial.print("[readPH]... phValue ");
    Serial.println(this->_phValue);
    return this->_phValue;
}

boolean DFRobot_ESP_PH::calibration(float voltage, float temperature, int mode)
{
    this->_voltage = voltage;
    this->_temperature = temperature;
    return phCalibration(mode);
}

boolean DFRobot_ESP_PH::calibration_by_serial_CMD(float voltage, float temperature, char *cmd)
{
    this->_voltage = voltage;
    this->_temperature = temperature;
    strupr(cmd);
    return phCalibration(cmdParse(cmd)); // if received Serial CMD from the serial monitor, enter into the calibration mode
}

boolean DFRobot_ESP_PH::calibration_by_serial_CMD(float voltage, float temperature)
{
    this->_voltage = voltage;
    this->_temperature = temperature;
    if (cmdSerialDataAvailable() > 0)
    {
        return phCalibration(cmdParse()); // if received Serial CMD from the serial monitor, enter into the calibration mode
    }
    return false; // no command received
}

boolean DFRobot_ESP_PH::cmdSerialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while (Serial.available() > 0)
    {
        if (millis() - cmdReceivedTimeOut > 500U)
        {
            this->_cmdReceivedBufferIndex = 0;
            memset(this->_cmdReceivedBuffer, 0, (ReceivedBufferLength_PH));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || this->_cmdReceivedBufferIndex == ReceivedBufferLength_PH - 1)
        {
            this->_cmdReceivedBufferIndex = 0;
            strupr(this->_cmdReceivedBuffer);
            return true;
        }
        else
        {
            this->_cmdReceivedBuffer[this->_cmdReceivedBufferIndex] = cmdReceivedChar;
            this->_cmdReceivedBufferIndex++;
        }
    }
    return false;
}

byte DFRobot_ESP_PH::cmdParse(const char *cmd)
{
    byte modeIndex = PH_CALIBRATION_MODE_ERROR;
    if (strstr(cmd, "ENTERPH") != NULL)
    {
        modeIndex = PH_CALIBRATION_MODE_READY;
    }
    else if (strstr(cmd, "EXITPH") != NULL)
    {
        modeIndex = PH_CALIBRATION_MODE_SAVE_AND_EXIT;
    }
    else if (strstr(cmd, "CALPH") != NULL)
    {
        modeIndex = PH_CALIBRATION_MODE_RUNNING;
    }
    return modeIndex;
}

byte DFRobot_ESP_PH::cmdParse()
{
    byte modeIndex = PH_CALIBRATION_MODE_ERROR;
    if (strstr(this->_cmdReceivedBuffer, "ENTERPH") != NULL)
    {
        modeIndex = PH_CALIBRATION_MODE_READY;
    }
    else if (strstr(this->_cmdReceivedBuffer, "EXITPH") != NULL)
    {
        modeIndex = PH_CALIBRATION_MODE_SAVE_AND_EXIT;
    }
    else if (strstr(this->_cmdReceivedBuffer, "CALPH") != NULL)
    {
        modeIndex = PH_CALIBRATION_MODE_RUNNING;
    }
    return modeIndex;
}

boolean DFRobot_ESP_PH::phCalibration(byte mode)
{
    char *receivedBufferPtr;
    static boolean phCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    switch (mode)
    {
    case PH_CALIBRATION_MODE_ERROR:
        if (enterCalibrationFlag)
        {
            Serial.println(F(">>>Command Error<<<"));
        }
        break;

    case PH_CALIBRATION_MODE_READY:
        enterCalibrationFlag = 1;
        phCalibrationFinish = 0;
        Serial.println();
        Serial.println(F(">>>Enter PH Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<"));
        Serial.println();
        break;

    case PH_CALIBRATION_MODE_RUNNING:
        if (enterCalibrationFlag)
        {
            if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_6_VOLTAGE))
            { // buffer solution:7.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:7.0"));
                this->neutralVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println(">>>NeutralVoltage:" + String(this->neutralVoltage) + "<<<");
                Serial.println();
                phCalibrationFinish = 1;
                return true;
            }
            else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
            { // buffer solution:4.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:4.0"));
                this->acidVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println(">>>AcidVoltage:" + String(this->acidVoltage) + "<<<");
                Serial.println();
                phCalibrationFinish = 1;
                return true;
            }
            else
            {
                Serial.println();
                Serial.print(F(">>>Buffer Solution Error Try Again<<<"));
                Serial.println(); // not buffer solution or faulty operation
                phCalibrationFinish = 0;
            }
        }
        break;

    case PH_CALIBRATION_MODE_SAVE_AND_EXIT: // store calibration value in eeprom
        if (enterCalibrationFlag)
        {
            Serial.println();
            if (phCalibrationFinish)
            {
                if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_5_VOLTAGE))
                {
                    EEPROM.writeFloat(this->_eepromStartAddress, this->neutralVoltage);
                    EEPROM.commit();
                }
                else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
                {
                    EEPROM.writeFloat(this->_eepromStartAddress + (int)sizeof(float), this->acidVoltage);
                    EEPROM.commit();
                }
                Serial.print(F(">>>Calibration Successful"));
            }
            else
            {
                Serial.print(F(">>>Calibration Failed"));
            }
            Serial.println(F(",Exit PH Calibration Mode<<<"));
            Serial.println();
            phCalibrationFinish = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
    return false;
}
