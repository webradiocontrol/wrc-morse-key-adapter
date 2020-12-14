# Web Radio Control morse key and PTT adapter

This is a USB adapter for a physical morse key and a PTT switch to be used with Web Radio Control
amateur radio station remote control software. The adapter connects to a computer or a mobile device via a USB port
and represents itself as a standard keyboard. The adapter will use Web Radio Control user interface keyboard shortcuts
(in a web browser) to control PTT and to perform morse keying.

The morse key adapter supports straight keys and provides a automatic keyer for dual-lever paddles.
The keying speed is adjustable via a potentiometer. The adapter generates CW sidetone and the pitch
and the volume of the sidetone are also adjustable via potentiometers.

## Features

* Straight key support
* Integrated CW keyer for dual-lever paddle with adjustable speed
* CW sidetone generation (via audio amplifier) with adjustable pitch and volume
* Option to use Iambic mode with a dual-lever paddle
* Option to invert dual-lever paddle functions
* Support for an external PTT switch

## Hardware requirements

An Arduino Micro, Micro Pro or a compatible board that has an
[ATmega32U4](https://www.microchip.com/wwwproducts/en/ATMEGA32U4) microcontroller.

The following boards have been tested:

* [Arduino Micro](https://store.arduino.cc/arduino-micro)
* [Arduino Pro Micro](https://deskthority.net/wiki/Arduino_Pro_Micro) running at 5V/16MHz (*NOT* the 3.3V/8MHz)

The default PlatformIO config and the source code settings assume use of .
This is the only microcontroller supported currently, because many Arduino models use microcontrollers that
do not provide USB client device support. Support for other microcontrollers with USB client functionality
may be added later.

### Full schematic

![Web Radio Control morse key USB adapter schematic](hardware/wrc-morse-key-adapter-schematic-20200416-01.png?raw=true)

Arduino Micro input pin descriptions:

* D0 (RX) = PTT (active low)
* D2 = Straight key or dual-lever paddle dit (tip, active low)
* D3 = Dual-lever paddle dah (ring, active low)
* D8 = Straight/automatic (low = straight)
* D9 = Iambic mode (active high)
* D10 = Inverted dual-lever paddle functions mode (active high)
* A0 = Keyer speed control
* A1 = Keyer sidetone pitch control

Arduino Micro output pin descriptions:

* D6 (or D13) = CW sidetone output (PWM square wave) - optional

The schematic includes a simple low-pass filter for the PWM square wave
CW sidetone output, which is then passed to an LM386 audio amplifier
in order to amplify the sidetone for speaker output.

## Use of external keyers

When using this adapter in straight key mode, an external keyer, such as the
[K3NG CW Keyer](https://blog.radioartisan.com/arduino-cw-keyer/) can be connected
to this adapter to provide additional functionality.

The CW key output of an external keyer should be connected to pin D2
of the Arduino board. The incoming signal must be pull D2 pin to GND
when active. Optionally, PTT control from an external keyer can be connected
to pin D0 (active low). 

## Building the firmware

Execute the following command to build the firmware.
The command depends on the microcontroller board type, which should be either Arduino Micro or Arduino Pro Micro.

### Arduino Micro

Building the firmware for Arduino Micro:

```bash
platformio run --environment micro
```

The built firmware can be found in directory `.pio/build/micro` in files `firmware.hex` in HEX format
and in `firmware.elf` in ELF format.

### Arduino Pro Micro

Building the firmware for Arduino Pro Micro:

```bash
platformio run --environment promicro
```

The built firmware can be found in directory `.pio/build/promicro` in files `firmware.hex` in HEX format
and in `firmware.elf` in ELF format.

## Flashing the firmware

Execute the following command to flash the firmware to an Arduino connected to a USB port. The command depends
on the microcontroller board type, which should be either Arduino Micro or Arduino Pro Micro.

### Arduino Micro

Flashing Arduino Micro:

```bash
platformio run --environment micro --target upload
```

### Arduino Pro Micro

Flashing Arduino Pro Micro:

```bash
platformio run --environment promicro --target upload
```

## Flashing binary only

### Windows

* Download the latest binary firmware file from `release` directory
  * For Arduino Micro: [wrc-morse-key-adapter-micro-20201213.hex](release/wrc-morse-key-adapter-micro-20201213.hex).
  * For Arduino Pro Micro: [wrc-morse-key-adapter-promicro-20201213.hex](release/wrc-morse-key-adapter-promicro-20201213.hex).
* Install [Arduino IDE for Windows](https://www.arduino.cc/en/Main/software) to get the latest version of `avrdude.exe` tool,
  which is a command-line application used to flash Arduino boards.
* Connect the Arduino to a USB port
* Find out the COM port for the Arduino using Device Manager in Control Panel.
  * Windows 10: Control Panel -> Hardware and Sound -> Device Manager.
  * The listing shows COM ports under: `Ports (COM & LPT)`
  * Look for: `USB Serial Device (COMx)`
* Find the location of `avrdude.exe`, located in directory `hardware/tools/avr/bin` under the directory where Arduino IDE is installed
* Open `Command Prompt` and change the current directory to where `avrdude.exe` is located
* Execute the following command to reset Arduino and to switch it to mode for flashing.
  *NOTE:* The command will fail, printing out error messages, and you will need to forcibly stop it by pressing `CTRL+C`.
  The only purpose of this command is to open the serial port to make the Arduino chip ready for flashing. 
  Remember to adjust the COM port in the command (COM3 in the example).
```bash
avrdude -C ..\etc\avrdude.conf -p atmega32u4 -c arduino -P COM3 -b 1200
  ```
* Press `Ctrl+C` to interrupt the command now that the Arduino is ready for flashing.
* Execute the following command to flash the new firmware in the downloaded `.hex` file.
  You will have only 8 seconds to do this until Arduino returns back to normal mode.
  Note that the COM port changes once again, usually to a port with
  a number one higher than previously (COM4 in this example).
```
avrdude -C ..\etc\avrdude.conf -c avr109 -p atmega32u4 -P COM4 -b 57600 -D -U flash:w:wrc-morse-key-adapter-micro-20201213.hex
```

### Linux

* Download the latest binary firmware file from `release` directory
  * For Arduino Micro: [wrc-morse-key-adapter-micro-20201213.hex](release/wrc-morse-key-adapter-micro-20201213.hex).
  * For Arduino Pro Micro: [wrc-morse-key-adapter-promicro-20201213.hex](release/wrc-morse-key-adapter-promicro-20201213.hex).
* Install [Arduino IDE for Linux](https://www.arduino.cc/en/Main/software) to get the latest version of `avrdude` tool,
  which is a command-line application used to flash Arduino boards.
* Connect the Arduino to a USB port
* Find out the USB device file port for the Arduino, the device file name is usually `/dev/ttyACMx` where `x` is some number
  * List the devices using command: `ls /dev/ttyACM*`
* Open a terminal and change the current directory to the directory where the Arduino IDE is installed
* Execute the following command to reset Arduino and to switch it to mode for flashing.
  *NOTE:* The command will fail, printing out error messages, and you will need to forcibly stop it by pressing `CTRL+C`.
  The only purpose of this command is to open the serial port to make the Arduino chip ready for flashing. 
  Remember to adjust the USB device file in the command (`/dev/ttyACM0` in the example).
```bash
./hardware/tools/avr/bin/avrdude -C ./hardware/tools/avr/etc/avrdude.conf -p atmega32u4 -c arduino -P /dev/ttyACM0 -b 1200
  ```
* Press `Ctrl+C` to interrupt the command now that the Arduino is ready for flashing.
* Execute the following command to flash the new firmware in the downloaded `.hex` file.
  You will have only 8 seconds to do this until Arduino returns back to normal mode.
  Note that the USB device file changes once again, usually to a port with
  a number one higher than previously (`/dev/ttyACM1` in this example).
```
./hardware/tools/avr/bin/avrdude -C ./hardware/tools/avr/etc/avrdude.conf -c avr109 -p atmega32u4 -P /dev/ttyACM1 -b 57600 -D -U flash:w:wrc-morse-key-adapter-micro-20201213.hex
```
