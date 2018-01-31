# KlikAanKlikUit RF Timer

### Introduction
Remote controlled power switches of the KlikAanKlikUit brand (see images for a receiver) are regularly offered at reasonable prices. We use several of them to switch lights on and off. Would it not be a great idea if this could continue during our holidays to scare off unwanted intruders? Of course off the shelf product are available to do so, but it is more fun to design and build something yourself. So this project presents a piece of microcontroller based hardware which can send commands to the power switches based on a schedule you've created. The software to turn the microcontroller into a timer is written in C. Schedules can be uploaded from your PC to the timer using a serial-via-USB connection. And in order to be able to create and maintain the schedules I've written a PC client in Python which can communicate with the timer.

### Hardware
The manual transmitters which are provided bij KlikAanKlikUit operate at 433 MHz and use a PT2262 Remote Control Encoder. So the timer must be able to emulate the PT2262's protocol in software. As microcontroller I have used a ATmega168 mySmartControl module from www.myavr.com. The module has an onboard USB interface which makes it easy to connect to your PC, and has a small soldering grid where you can add a power supply. The mySmartControl module has a 20-pin header which connects to the myavr prototyping board. On this board I have placed the transmitter module, a I2C EEPROM (as the AVR itself only has a 512 bytes EEPROM) and a DS1307 real-time clock. See images for the schematic and the actual pcb.

### Software

#### Timer
The AVR code which turns the microcontroller into a timer is written in C. It operates stand alone, and starts to run as soon as the hardware is powered on. The building blocks of the software are:
- A main loop which waits for commands to come in via the serial (usart) port and wakes up every minute to see if according to the schedule on or off commands must be transmitted to a switch (main.c, timer1.c).
- A parser which handles all the received commands, mainly used to upload a new schedule (main.c).
- Routines which drive the RF transmitter emulating the PT2262's protocol (remote.c).
- Interrupt driven serial communication (usart0.c).
- Routines to send and receive data via the I2C port which connects the AVR to the EEPROM and DS1307 (i2c.c, ds1307.c, 24cXX.h).

All these files reside in the same directory. For the preprocessor symbol F_CPU=20000000UL must be defined.
Microchip - the producer of AVR microcontrollers - offers the free Atmel Studio software development environment which you can use to compile the program. The resulting .elf file can then be uploaded to the mySmartControl via myAvr's ProgTool.

#### Client
The client programs main function it to write schedules to the timer. In order to do so you can read the current schedule from the timer, load a schedule which you have previously saved or enter a new one. For serial communication is depends on package PySerial. Exchanging schedules with the timer takes a while as the EEPROM is not that fast.
After starting the program it will look for available COM port an offer you the choice to connect to one. If you have not connected the timer via USB to the PC you will not see the corresponding COM port.
The UI is build using a PyQt5 StackedWidget. All screens and their corresponding classes are located in package ui. QT Designer is used to create the screens, and the .ui files it produces are also placed in directory ui. Upon opening a screen the .ui is loaded. For maintaining schedules a TableView was subclassed to have an editing widget (combox, date- or timepicker) for every field (editor.py). A custom TabelModel (model.py) is connected to the TableView and combines all the functions to read and write schedules to the timer and from disk.
