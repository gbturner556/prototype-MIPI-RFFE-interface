/*
 * Gavin Turner
 * 7/25/2022
 * 
 * This is program helps determine the slave address of an unknown DUT
 * utlizing the MIPI RFFE_master class when used in conjunction with the
 * MIPI interface board. This source code would be uploaded to the 
 * onboard microcontroller. The necessary power and communication 
 * signals routed through the BNC connectors to micromanipulators for 
 * die level probing.
 *
 * The body of the code loops through various slave addresses. It then
 * attempts to write and read back from various registers. If the master
 * reads back anything other than zero (some registers may be read only)
 * it outputs the corresponding slave address.
 * 
 */

#include <MIPI.h>
#include <Adafruit_SSD1327.h>
#include <splash.h>
#define OLED_RESET -1

void setup() {
  //initialize display
  Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET, 1000000);
  byte received = 0;

  //setup display
  display.begin(0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1327_WHITE);
  display.println("Scanning ...");
  display.display();
  
  //intialize communications to DUT
  RFFE_master master(0x0, 1.8, 3.3);
  master.begin();

  //loop through various slave addresses 
  for(byte slave_address = 0x00; slave_address <= 0x0F; slave_address++){
    //set to new slave address
    master.set_slave(slave_address);
    
	//write to various registers then read back value
    for(byte register_address = 0x00; register_address <= 0x1F; register_address++){
      //write a value to new address and read it back
      master.write(register_address, 0x01);
      received = master.read(register_address);

      //check if we received any thing
      if(received != 0){
        //display the address if we did
        display.printf("slave: 0x%X\n---------------------\n", slave_address);
        display.display();
        register_address = 0x1F;  //kick out of this loop if we find something

        //clear the display if we've used up all lines
        if(display.getCursorY()>= 128){
          display.clearDisplay();
          display.display();
          display.setCursor(0,0);
        }
      }
    }
  }
	//end of program display results
    display.println("Done");
    display.display();
}


void loop() {

}
