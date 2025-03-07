# MIPI-RFFE-interface-board
Gavin Turner  
<gbturner@gmail.com>

## Background
This project was built by Gavin Turner in the summer of 2022 during an 
internship at GlobalFoundries' electrical failure analysis lab. The goal 
of the project was to develop a simple communications interface that would
allow for quick screening of product fallout through die level probing.
The interface was built specifically for the communications through
the MIPI RFFE protocol as outlined by the MIPI alliance, but software 
modifications could allow for communciations with any protocol requiring
four channels or less.

## Stakeholder Specifications
- Implement easy to use functions for **read**, **extended read**, **write**, **extended write**, **bus park**, and **assert SSC**
- Supply and monitor **VDD** and **VIO** (1V to 3.3V up to 5mA)
- Adjustable logic levels on **DATA** and **CLK** pins (1V to 3.3V)
- Utilize an RP2040 microcontroller
- Implement solution on a PCB with BNC connectors for ease of use with micromanipulators during probing

## Implementation
The core of the software solution is an API implemented through the use of a C++ header file.
This header file defines a custom class named `RFFE_master`. This class builds upon a variety
of private functions to enable the user to "bit bang" dataframes with occordance to the MIPI
alliance specifications. The software allows for low speed communications with a DUT with active
power supply monitoring to characterize defective parts. The final software solution can be viewed
in the **MIPI.h** and **MIPI.cpp** files as well as application examples in the **source_code** folder.

The hardware solution implements 4 MCP4725 ADC's for variable logic levels and power supplies, 2
INA219 current shunts monitors for active power supply monitoring, 1 OLED display for data output,
1 TCA9548 I2C multiplexer to manage communications with peripherals, and 5 BNC connectors for easy
connection to micromanipulators. All components are integrated on a standalone PCB with an on board
RP2040 based microcontroller. The final hardware implementation can be viewed in the **board_files** folder