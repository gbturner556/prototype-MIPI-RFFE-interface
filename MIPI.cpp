#include "MIPI.h"
/**
   constructor for RFFE_master
   
   @param slave_address is the slave address for the DUT
   @param vio_volt is the desired output voltage for the vio port this will aslo be used as the voltage level for the SDATA and SCLK ports
   @param vcc_volt is the desired output voltage for the vdd port 

*/
RFFE_master::RFFE_master(byte slave_address, double vio_volt, double vcc_volt){
  //set up class variables
  this->_VIO_volts = vio_volt;
  this->_VCC_volts = vcc_volt;
  this->_slave_address = slave_address;
  this->_VIO_code = _volts_to_bits(vio_volt); // use to convert from voltage to DAC code
  this->_VCC_code = _volts_to_bits(vcc_volt);
  this->_EVEN_parity = 0;
  this->_ODD_parity  = 1;
  this->_NO_parity   = 2; 
  
} /* RFFE_master */

/**
	the default constructor for the RFFE master
	vio_volts => 1.8 V	
	vcc_volts => 1.8 V
	slave_address => 0x01

*/
RFFE_master::RFFE_master(){
  //set up class variables
  this->_VIO_volts = 1.8;
  this->_VCC_volts = 1.8;
  this->_slave_address = 0x01;
  this->_VIO_code = _volts_to_bits(1.8); // use to convert from voltage to DAC code
  this->_VCC_code = _volts_to_bits(1.8);
  this->_EVEN_parity = 0;
  this->_ODD_parity  = 1;
  this->_NO_parity   = 2;
   
} /* RFFE_master */

/**
	function used to begin connection to the rffe master's peripherals
*/
void RFFE_master::begin(){

//set up i2c pins for RPI (maybe  unecessary idk)
  i2c_init(i2c_default, 1000 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

  //start up all the dacs
  tca.setChannelMask(ALL_SELECT);
  dat.begin();
  clk.begin();
  vcc.begin();
  vio.begin();

  //make sure all dacs are in normal operation
  dat.writePowerDownMode(MCP4725_PDMODE_NORMAL);
  clk.writePowerDownMode(MCP4725_PDMODE_NORMAL);
  vcc.writePowerDownMode(MCP4725_PDMODE_NORMAL);
  vio.writePowerDownMode(MCP4725_PDMODE_NORMAL);
  
  //intialize logic channels LOW
  tca.setChannelMask(LOGIC_SELECT);
  dat.setValue(0);
  clk.setValue(0);

  //turn on power channels
  tca.setChannelMask(POWER_SELECT);
  vcc.setValue(_VCC_code);
  vio.setValue(_VIO_code);

  //go back to logic for comms
  tca.setChannelMask(LOGIC_SELECT);
  
  //adc setup
  adc_init();
  adc_gpio_init(ADC_PIN); // Make sure GPIO is high-impedance, no pullups etc
  adc_select_input(0);

  //current monitor setup
  vcc_monitor.begin();
  vcc_monitor.configure(INA219::RANGE_16V, INA219::GAIN_1_40MV, INA219::ADC_12BIT, INA219::ADC_12BIT, INA219::CONT_SH_BUS);
  vcc_monitor.calibrate(0.1, 0.04, 16, 0.4);
  
  vio_monitor.begin();
  vio_monitor.configure(INA219::RANGE_16V, INA219::GAIN_1_40MV, INA219::ADC_12BIT, INA219::ADC_12BIT, INA219::CONT_SH_BUS);
  vio_monitor.calibrate(0.1, 0.04, 16, 0.4); 
	
} /* begin */

/**
   setter for slave address

   @param slave_address is the desired slave address

*/
void RFFE_master::set_slave(byte slave_address) {
  this->_slave_address = slave_address;
} /* set_slave */


/**
   used to singify end of transmission according to MIPI Standards
*/
void RFFE_master::_bus_park() {

  //pull clk and dat low
  tca.setChannelMask(LOGIC_SELECT);
  dat.setValue(0);  
  clk.setValue(0);

  //wiggle clock
  clk.setValue(_VIO_code);
  clk.setValue(0);

//  /* skyworks bit bang code has 5 clock cycles after bus park for whatever reason */
//  mcp.setChannelValue(SCLK_CHANNEL, _VIO_code); mcp.setChannelValue(SCLK_CHANNEL, 0);
//  mcp.setChannelValue(SCLK_CHANNEL, _VIO_code); mcp.setChannelValue(SCLK_CHANNEL, 0);
//  mcp.setChannelValue(SCLK_CHANNEL, _VIO_code); mcp.setChannelValue(SCLK_CHANNEL, 0);
//  mcp.setChannelValue(SCLK_CHANNEL, _VIO_code); mcp.setChannelValue(SCLK_CHANNEL, 0);
//  mcp.setChannelValue(SCLK_CHANNEL, _VIO_code); mcp.setChannelValue(SCLK_CHANNEL, 0);

} /* void RFFE_master::_bus_park */

/**
   used to assert the start of a transmission
*/
void RFFE_master::_assert_SSC() {
  //select logic channel, maybe be redundant in some cases
  tca.setChannelMask(LOGIC_SELECT);

  //check if dat is in normal operation just in case
  if(dat.readPowerDownModeDAC() != MCP4725_PDMODE_NORMAL){
    dat.writePowerDownMode(MCP4725_PDMODE_NORMAL); 
  }

  //pull clk low
  clk.setValue(0);

  //wiggle data
  dat.setValue(_VIO_code);
  dat.setValue(0);
}
/**
   helper function used to write a bit out using the
   MCP4725 on the front end

   @param bit_vlaue is the value of the bit to be written out
*/
void RFFE_master::_bit_write(byte bit_value) {
  //filter out first bit
  bit_value &= 0x01;
  
  //set data
  dat.setValue(_VIO_code * bit_value);  //looks a little weird but is a simple way of writing VIO_code when high and 0 when low

  //pulse clock
  clk.setValue(_VIO_code);
  clk.setValue(0);  

} /* void RFFE_master::bit_write */

/**
 * helper function that performs a series of bit writes based on input parameters
 * 
 * @param write_value the word (16 bits) to be written
 * @param bit_count the number of bits in the word containing information
 * @param parity the type of parity to be sent with word (0 = even, 1 = odd, 2 = none)
 */
void RFFE_master::_frame_write(word write_value, byte bit_count, byte parity) {
  word bit_mask;
  byte bit_index;
  byte bit_value;
  byte one_count = 0;
  byte zero_count = 0;

  bit_mask = 1 << (bit_count-1);
  for (bit_index = 0; bit_index < bit_count; bit_index++) {
    if ((write_value & bit_mask) != 0) {
      this->_bit_write(1); one_count++;
    } else {
      this->_bit_write(0); zero_count++; /* for equal time */
    } /* else */
    write_value = write_value << 1;
  } /* for */
  switch (parity) {
    case 0: /* EVEN_parity */
      this->_bit_write( one_count % 2 );
      break;
    case 1: /* ODD_parity */
      this->_bit_write( (one_count % 2) ^ 1 ); /* ^ is xor */
      break;
    case 2: /* NO_parity */
      break;
  } /* switch (parity) */
} /* void RFFE_master::_frame_write */

/**
 * helper function used to read a frame of 8 bits from a slave device
 * 
 * @return the byte read from the slave device
 */
byte RFFE_master::_frame_read() {
  int input_bit;
  int threshold = _VIO_code/2;
  byte read_value = 0;
  
  // Frame read - use the ADC on PIN A0 (GP26)
  // On board ADC and external DAC are 12 bit using 3.3 Vref
  // this means HIGH = VIO_code and LOW = 0
  // threshold is VIO_code/2

  //put dat channel in high impedance for read back
  dat.writePowerDownMode(MCP4725_PDMODE_500K);

  //read the byte
  for (byte bit_index=0; bit_index <= 7; bit_index++) {
    //clock and read bit
    clk.setValue(_VIO_code);
    clk.setValue(0);
    input_bit = adc_read();

    //decide if its a 1 or 0 
    if (input_bit > threshold) {
      read_value = read_value | 0b00000001;
    } 
    
    //move to next bit unless we are on the last bit
    if(bit_index < 7){
      read_value = read_value << 1;
    }
    
  } /* for bit_index */
  
  //Send clock for parity bit but ignore */
  clk.setValue(_VIO_code);
  clk.setValue(0);

  //put dat channel back in normal operation
  dat.writePowerDownMode(MCP4725_PDMODE_NORMAL);

  //send back read value
  return read_value;
} /* int RFFE_master::_frame_read() */

/**
 * helper fucntion that uses the frame write to construct and write a command frame 
 * based on RFFE command sequences
 * 
 * @param Slave_address is the slave address of the intendent target (4 bits)
 * @param Command is the command to be sent (3 bits)
 * @param Register_address is the address of the target register (5 bits)
 */
void RFFE_master::_command_frame(byte Slave_address, byte Command, byte Register_address) {
  word _command_word;
  _command_word= (word(Slave_address & 0X0F) << 8) | (word(Command & 0x07) << 5) | (word(Register_address & 0x1F));
  this->_frame_write(_command_word,12,this->_ODD_parity);
} /* void RFFE_master::_command_frame(byte Slave_address, byte Command, byte Register_address) */

/**
 * helper function that runs through the proccess of writing to a given registersin a slave device
 * 
 * @param Slave_address the address of the intended target device
 * @param Register_address the target register in the intended target device
 * @param Register_value the value to write to the target register
 */
void RFFE_master::_register_write(byte Slave_address, byte Register_address, byte Register_value) {
  /* frame_write makes this simple */
  this->_assert_SSC();
  this->_command_frame(Slave_address,0b010,Register_address);
  this->_frame_write(word(Register_value),8,_ODD_parity);
  this->_bus_park();
} /* void RFFE_master::_register_write(byte Slave_adress, byte Register_address, byte Register_value) */

/**
 * helper function that runs through the proccess of writing to a given register in a slave device
 * when the register address is more than 4 bits 
 * 
 * @param Slave_address the address of the intended target device
 * @param Register_address the target register in the intended target device
 * @param Register_value the value to write to the target register
 */
void RFFE_master::_extended_register_write(byte Slave_address, byte Register_address, byte Register_value) {
  /* frame_write makes this simple */
  this->_assert_SSC();
  this->_command_frame(Slave_address,0x00,0x00);
  this->_frame_write(word(Register_address),8,_ODD_parity);
  this->_frame_write(word(Register_value),8,_ODD_parity);
  this->_bus_park();
} /* void RFFE_master::_extended_register_write(byte Slave_adress, byte Register_address, byte Register_value) */

/**
 * helper function used to ask a slave device to read a given register
 * then read the byte sent from the slave device
 * 
 * @return the byte read from the slave device
 */
byte RFFE_master::_register_read(byte Slave_address, byte Register_address) {
  byte result;
  /* frame_read makes this simple */
  this->_assert_SSC();
  this->_command_frame(Slave_address,0b011,Register_address);
  this->_bus_park();
  result = this->_frame_read();
  this->_bus_park();
  return result;
} /* int RFFE_master::_register_read(byte Slave_address, byte Register_address) */

/**
 * helper function used to convert a desired voltage between 0-3.3 V
 * into a 12 bit DAC code for the MCP4728
 * 
 * @param v is the desired voltage
 * @return the corresponding 12 bit DAC code
 */
int RFFE_master::_volts_to_bits(double v){
  if(v >= 3.3){ return 4095;}
  else if(v <= 0){return 0;}
  else{return int(v * 4096.0 / 3.3);}
}

/**
 * function to write to a slave device's register when the register address 
 * contains 4 bits or less
 * 
 * @param Register_addr is the target register in the slave device
 * @param Register_value is the value to be written to the given register
 */
void RFFE_master::write(byte Register_addr, byte Register_value) {
  this->_register_write(this->_slave_address, Register_addr, Register_value);
  } /* RFFE_master::write */

/**
 * function to perform an extended write to a slave device's register
 * when the register address contains more than 4 bits
 * 
 * @param Register_addr is the target register in the slave device
 * @param Register_value is the value to be written to the given register
 */
void RFFE_master::extended_write(byte Register_addr, byte Register_value) {
  this->_extended_register_write(this->_slave_address, Register_addr, Register_value);
  } /* RFFE_master::write */
  
/**
 * function to read a register from the given slave device
 * 
 * @param Register_addr the target register in the slave device
 */
byte RFFE_master::read(byte Register_addr){
  return this->_register_read(this->_slave_address, Register_addr);
} /* RFFE_master::read */

/**
 * function to read to current to Vcc using INA219 current monitor
 * 
 * @return the current in microamps
 */
float RFFE_master::vcc_current_microamps(){
  float offset_uA = 6.95;	//roughly characterizes the offset introduced by DAC
  return ((this->vcc_monitor.shuntVoltage()/SHUNT_ohm * 1000000) - offset_uA);
  //return this->vcc_monitor.shuntVoltage()/SHUNT_ohm * 1000000;	//uncomment this line for no offset adjust
} /* vcc_curent_microamps */

/**
 * function to read to current to Vio using INA219 current monitor
 * 
 * @return the current in microamps
 */
float RFFE_master::vio_current_microamps(){
  float offset_uA = 11.0;	//roughly characterizes the offset introduced by DAC
  return ((this->vio_monitor.shuntVoltage()/SHUNT_ohm * 1000000) - offset_uA);
  //return this->vio_monitor.shuntVoltage()/SHUNT_ohm * 1000000;	//uncomment this line for no offset adjust
} /* vio_curent_microamps */

/**
	function used to change the io voltage
	@param vio_volt is the desired vio voltage
*/
void RFFE_master::set_VIO_volts(double vio_volt){
	this->_VIO_volts = vio_volt;
	this->_VIO_code = _volts_to_bits(vio_volt);
}/* set_VIO_volts */

/**
	function used to change the supply voltage
	@param vcc_volt is the desired vcc voltage	
*/
void RFFE_master::set_VCC_volts(double vcc_volt){
	this->_VCC_volts = vcc_volt;
	this->_VCC_code = _volts_to_bits(vcc_volt);
}/* set_vCC_volts */