/*
 * Gavin Turner
 * 7/20/2022
 * 
 * This is an example application of the MIPI RFFE_master class when
 * used in conjunction with the MIPI interface board. This source 
 * code would be uploaded to the onboard microcontroller. The necessary
 * power and communication signals routed through the BNC connectors
 * to micromanipulators for die level probing.
 *
 * The body of the code first puts the DUT into a lowe power state.
 * Then proceeds to run a series of commands reading the power
 * consumption after each test. All necessary commands and register
 * addresses were provided by design engineers.
 * 
 * Note that ideally the sections of code labelled LPM, Test1,
 * Test2, etc would be declared outside of the main loop as 
 * functions, but a bug in the code is not allowing proper use
 * of the RFFE master when declared globally. For now this 
 * works.
 * 
 */

#include <MIPI.h>
#include <Adafruit_SSD1327.h> 
#include <splash.h>

#define OLED_RESET -1

void setup() {
  
  //initialize OLED display
  Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET, 1000000);
  display.begin(0x3C);
  display.setRotation(2);
  
  //initialize communication with DUT
  RFFE_master dut(0x08, 1.8, 3.3); 
  dut.begin();

  while(true){
	  
    //reset screen
    display.setCursor(0,0);
    display.clearDisplay();
    display.display();
    
    //put part in low power state
    dut.extended_write(0x2D,0x0F);  
    dut.extended_write(0x2E,0x00); 
    dut.write(0x1C,0x80);           

    delay(1);
    display.printf("LPM\nIdd: %4.1f uA\n", dut.vcc_current_microamps());
    display.display();

    //Test1 as provided by customer
    dut.extended_write(0x2D,0x0F);  // EXT_TRIG_MASK
    dut.extended_write(0x2E,0x00);  // EXT_TRIG
    dut.write(0x1C,0x38);           // PM_TRIG
    dut.write(0x03,0x16);           // C1_B42_LTE
    dut.write(0x04,0x01);           // C2_B71_LTE
    dut.write(0x05,0x0A);           // C3_B11_LTE
    dut.write(0x07,0x43);           // C2_F_0p0_24_O2
    dut.write(0x06,0x85);           // C3_F_0p0_50_O1
    dut.write(0x08,0x00);           // ATTEN_O1_X1_O2_X1

    display.printf("Test1\nIdd: %4.1f uA\n", dut.vcc_current_microamps());
    display.display();
    delay(200);
	

    //Test2 as provided by customer
    dut.extended_write(0x2D,0x0F);  // EXT_TRIG_MASK
    dut.extended_write(0x2E,0x00);  // EXT_TRIG
    dut.write(0x1C,0x38);           // PM_TRIG
    dut.write(0x03,0x16);           // C1_B42_LTE
    dut.write(0x04,0x01);           // C2_B71_LTE
    dut.write(0x05,0x0A);           // C3_B11_LTE
    dut.write(0x06,0x44);           // C2_R_0p0_24_O1
    dut.write(0x07,0x86);           // C3_R_0p0_50_O2
    dut.write(0x08,0x00);           // ATTEN_O1_X1_O2_X1

    display.printf("Test2\nIdd: %4.1f uA\n", dut.vcc_current_microamps());
    display.display();
    delay(200);
       
	   
    //Test3 as provided by customer
    dut.extended_write(0x2D,0x0F);  // EXT_TRIG_MASK
    dut.extended_write(0x2E,0x00);  // EXT_TRIG
    dut.write(0x1C,0x38);           // PM_TRIG
    dut.write(0x03,0x16);           // C1_B42_LTE
    dut.write(0x04,0x01);           // C2_B71_LTE
    dut.write(0x05,0x0A);           // C3_B11_LTE
    dut.write(0x07,0x01);           // C1_F_0p0_NF_O2
    dut.write(0x06,0x03);           // C2_F_0p0_NF_O1
    dut.write(0x08,0x00);           // ATTEN_O1_X1_O2_X1

    display.printf("Test3\nIdd: %4.1f uA\n", dut.vcc_current_microamps());
    display.display();
    delay(200);


    //Test4 as provided by customer
    dut.extended_write(0x2D,0x0F);  // EXT_TRIG_MASK
    dut.extended_write(0x2E,0x00);  // EXT_TRIG
    dut.write(0x1C,0x38);           // PM_TRIG
    dut.write(0x03,0x16);           // C1_B42_LTE
    dut.write(0x04,0x01);           // C2_B71_LTE
    dut.write(0x05,0x0A);           // C3_B11_LTE
    dut.write(0x06,0x86);           // C3_R_0p0_50_O1
    dut.write(0x07,0x82);           // C1_R_0p0_50_O2
    dut.write(0x08,0x00);           // ATTEN_O1_X1_O2_X1

    display.printf("Test4\nIdd: %4.1f uA\n", dut.vcc_current_microamps());
    display.display();
    delay(200);
	

    //Test5 as provided by customer
    dut.extended_write(0x2D,0x0F);  // EXT_TRIG_MASK
    dut.extended_write(0x2E,0x00);  // EXT_TRIG
    dut.write(0x1C,0x38);           // PM_TRIG
    dut.write(0x03,0x16);           // C1_B42_LTE
    dut.write(0x04,0x01);           // C2_B71_LTE
    dut.write(0x05,0x0A);           // C3_B11_LTE
    dut.write(0x06,0x81);           // C1_F_0p0_50_O1
    dut.write(0x07,0x85);           // C3_F_0p0_50_O2
    dut.write(0x08,0x00);           // ATTEN_O1_X1_O2_X1

    display.printf("Test5\nIdd: %4.1f uA\n", dut.vcc_current_microamps());
    display.display();
    delay(200);
   
  }  
  
}

void loop() {

}
