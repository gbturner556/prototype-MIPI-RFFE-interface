#ifndef MIPI_H
#define MIPI_H

#include <MCP4725.h>
#include <TCA9548.h>
#include <INA219.h>
#include <Wire.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include <Arduino.h>

//addresses for I2C expander
#define TCA_ADDR 0x70       //address of I2C expander
#define LOGIC_SELECT 0x03   //ch0 , ch1
#define POWER_SELECT 0x0C   //ch2 , ch3
#define ALL_SELECT 0x0F     //ch1 , ch2 , ch3 , ch4

//MCP4725 addresses
//DAT: ADDR1
//CLK: ADDR0
//VCC: ADDR1
//VIO: ADDR0
#define MCP_ADDR_0 0x60     
#define MCP_ADDR_1 0x61     //drew a dot on these dacs


//pins
#define OLED_RESET -1
#define ADC_PIN 26

//current sense parameters
#define SHUNT_ohm   40 

class RFFE_master
{
  public:
    RFFE_master(byte slave_address, double vio_volt, double vcc_volt);
	RFFE_master();
	void begin();
    void write(byte Register_addr, byte Register_value);
    void extended_write(byte Register_addr, byte Register_value);
    byte read(byte Register_addr);
    void set_slave(byte slave_address);
	void set_VIO_volts(double vio_volt);
	void set_VCC_volts(double vcc_volt);
	float vio_current_microamps();
	float vcc_current_microamps();
	
	
  private:
    //variables
    byte  _slave_address;
    byte  _EVEN_parity;
    byte  _ODD_parity;
    byte  _NO_parity;
    word  _VIO_code;
    word  _VCC_code;
	float _VIO_volts;
	float _VCC_volts;
    TCA9548 tca = TCA9548(TCA_ADDR);
    MCP4725 dat = MCP4725(MCP_ADDR_1);
    MCP4725 clk = MCP4725(MCP_ADDR_0);
    MCP4725 vcc = MCP4725(MCP_ADDR_1);
    MCP4725 vio = MCP4725(MCP_ADDR_0);
    INA219 vio_monitor = INA219(INA219::I2C_ADDR_41);
    INA219 vcc_monitor = INA219(INA219::I2C_ADDR_44);
	
    //functions
    void  _bus_park();
    void  _assert_SSC();
    void  _frame_write(word write_value, byte bit_count, byte parity);
    byte  _frame_read();
    void  _bit_write(byte bit_value);
    void  _command_frame(byte Slave_address, byte Command, byte Register_address);
    void  _register_write(byte Slave_address, byte Register_address, byte Register_value);
    void  _extended_register_write(byte Slave_address, byte Register_address, byte Register_value);
    byte  _register_read(byte Slave_address, byte Register_address);
    int   _volts_to_bits(double v);
	
}; /* class RFFE_master */

#endif