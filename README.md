# Web Radio Control morse key and PTT adapter

**NOTE:** This documentation and code is for the latest version 2 (v2) of the USB adapter.
See the [v1 branch](../../tree/v1) if you are looking for the old version 1 documentation. 

This is a USB adapter for a physical morse key and a PTT switch to be used with [Web Radio Control](https://doc.webradiocontrol.tech/)
amateur radio station remote control software. The adapter connects to a computer or a mobile device via a USB port
and represents itself as a standard keyboard. The adapter will use Web Radio Control user interface keyboard shortcuts
(in a web browser) to control PTT and to perform morse keying.

The morse key adapter supports straight keys and provides a automatic keyer for dual-lever paddles.
The keying speed is adjustable via a potentiometer. The adapter generates CW sidetone and the pitch
and the volume of the sidetone are also adjustable via potentiometers.

See [Web Radio Control documentation](https://doc.webradiocontrol.tech/) for more information about Web Radio Control
amateur radio station remote control software.

## Features

* Integrated CW keyer for dual-lever paddle with adjustable speed
* CW sidetone generation (via audio amplifier) with adjustable pitch and volume
* Straight key support
* Option to use Iambic mode with a dual-lever paddle
* Option to invert dual-lever paddle functions
* Support for an external PTT switch

## How do I get one?

This is a DIY kit, so you will have to obtain all components and build
the device yourself.

However, there are free printed circuit board (PCB) designs available.
Use the links below to download Gerber files for either the Arduino Micro
or the Arduino Pro Micro PCB version.

### Printed Circuit Boards available for order (Europe only)

There is a limited number of PCBs available for order from Mikael OH3BHX.
**Note that the PCBs are available in Europe only.**

The circuit board price is 10 euros + postage. Payments accepted via PayPal only.

Please send email to `mikael (at) webradiocontrol.tech` for more instructions
on how to order PCBs.

## Hardware requirements

An Arduino Micro, Micro Pro or a compatible board that has an
[ATmega32U4](https://www.microchip.com/wwwproducts/en/ATMEGA32U4) microcontroller.

The following boards have been tested:

* [Arduino Micro](https://store.arduino.cc/arduino-micro)
* [Arduino Pro Micro](https://deskthority.net/wiki/Arduino_Pro_Micro) running at 5V/16MHz (*NOT* the 3.3V/8MHz)

The Arduino firmware settings assume use of an
[ATmega32U4](https://www.microchip.com/wwwproducts/en/ATMEGA32U4) microcontroller.
This is the only microcontroller supported currently, because it can act as a USB client device.
Support for other microcontrollers with USB client functionality may be added later.

### Full schematic

#### For Arduino Micro

![Web Radio Control morse key USB adapter schematic (Arduino Micro)](hardware/wrc-morse-key-adapter-schematic-v2-arduino-micro.png?raw=true)

#### For Arduino Pro Micro

![Web Radio Control morse key USB adapter schematic (Arduino Micro)](hardware/wrc-morse-key-adapter-schematic-v2-arduino-pro-micro.png?raw=true)

#### Pin descriptions

Arduino Micro input pin descriptions:

* D0 (RX) = PTT (active low)
* D2 = Straight key or dual-lever paddle dit (tip, active low)
* D3 = Dual-lever paddle dah (ring, active low)
* D8 = Straight/automatic (low = straight)
* D9 = Iambic mode (active high)
* D10 = Inverted dual-lever paddle functions mode (active high)
* A0 = Keyer sidetone pitch control
* A1 = Keyer speed control

Arduino Micro output pin descriptions:

* D6 = CW sidetone output (PWM square wave) - optional

The schematic includes a simple low-pass filter for the PWM square wave
CW sidetone output, which is then passed to a TDA7050 audio amplifier
in order to amplify the sidetone for speaker output.

### Printed Circuit Boards

The PCB marking should be mostly self-explanatory.

The markings for TRS connectors are:

* `T` = tip
* `R` = ring
* `S` = shield

Potentiometers use the following convention:

* `+` = positive voltage
* `O` = output (variable output)
* `G` = ground

**NOTE:** The pads for the volume potentiometer `R7` are missing these markings. When looking at the board
in the orientation of the images below, the markings should be: `G`, `O`, `+`.

#### For Arduino Micro

![Web Radio Control morse key USB adapter schematic (Arduino Micro)](hardware/wrc-morse-key-adapter-pcb-v2-arduino-micro.svg?raw=true)

Download [Gerber files for Arduino Micro version](hardware/wrc-morse-key-adapter-pcb-v2-arduino-micro-gerber.zip?raw=true) of the PCB.

#### For Arduino Pro Micro

![Web Radio Control morse key USB adapter schematic (Arduino Micro)](hardware/wrc-morse-key-adapter-pcb-v2-arduino-pro-micro.svg?raw=true)

Download [Gerber files for Arduino Pro Micro version](hardware/wrc-morse-key-adapter-pcb-v2-arduino-pro-micro-gerber.zip?raw=true) of the PCB.

### Component list

All switches, potentiometers and jacks are designed to be mounted in the enclosure, so they should be connected
electrically to the circuit board using jumper wires. The circuit board has connections for 2.54mm (100mil) pin headers
where these external components can be connected.

It is recommended to mount the Arduino using 2.54mm (100mil) pin headers and sockets and
to use a DIL8 socket for the TDA7050 amplifier -- mainly to avoid overheating them while soldering and
for flexibility to remove them for other uses later.

Component list:

* SW1, SW2, SW3, SW4: Latching switch (that keeps its state)
* J1: 3.5mm stereo jack for dual-lever paddle morse key.
  * The tip and the shield are used in case of straight key.
* J2, J3: 3.5mm stereo jacks for incoming audio and headphones.
* R1, R2, R3: 10k ohm 1/4-watt through-hole resistor
* R4, R5, R6: 15k ohm 1/4-watt through-hole resistor
* R7, R8, R9: 10k ohm potentiometer
* R10, R11: 1k ohm 1/4-watt through-hole resistor
* C1: 470µF electrolytic through-hole capacitor
* C2: 100µF electrolytic through-hole capacitor
* C3: 100nF ceramic through-hole capacitor
* C4, C5, C6: 10nF ceramic through-hole capacitor
* C7: 100nF ceramic through-hole capacitor
* U1: An official Arduino Micro or an Arduino Pro Micro, depending on the PCB model.
  * See below for links to stores selling Arduino products.
* U2: TDA7050 audio amplifier with DIP8 packaging

### Obtaining components

Most of the regular through-hole components should be available in basically any electronics store. The TDA7050
amplifier is also available widely.

See the links below for a listing of stores selling Arduino products, an enclosure and TDA7050s.

While the Arduino Pro Micro and Arduino Micro have slightly different pinouts, they are completely interchangeable
in the context of this USB adapter. Arduino Pro Micros, being Arduino clone products, tend to be a bit cheaper.

#### Enclosure

* https://shopware.donau-elektronik.de/artikel/gehaeuse/gehaeuseserie-kgb/3299/kgb11-euro-box-klein-95x135x45-schwarz
* https://www.partco.fi/en/mechanics/housing/plastic-enclosures/198-kot-45x95x135-mus.html (Finland)

#### Arduino Micro (World-wide)

* https://store.arduino.cc/products/arduino-micro

#### Arduino Pro Micro 5V/16MHz: (World-wide)

* https://www.banggood.com/Geekcreit-Pro-Micro-5V-16M-Mini-Leonardo-Microcontroller-Development-Board-Geekcreit-for-Arduino-products-that-work-with-official-Arduino-boards-p-1077675.html

#### Arduino Pro Micro 5V/16MHz: (Finland)

* https://ihmevekotin.fi/arduino-alustat/674-arduino-pro-micro-atmega32u4-5v16mhz.html
* https://www.elektroniikkaosat.com/c-45/p-425511661/Kehitysalusta-Pro-Micro.html
* http://robomaa.fi/arduino/arduino-pro-sarja/pro-micro-5v-16mhz

#### Arduino Micro: (Finland)

* https://www.verkkokauppa.com/fi/product/54618/fgjfx/Arduino-Micro-kehitysalusta
* https://www.triopak.fi/fi/tuote/ARDUINO-A000053
* https://www.kouluelektroniikka.fi/tuote/ohjelmoitava-elektroniikka-ja-tarvikkeet/arduino/arduino-alustat/4600053/arduino-micro-kehitysalusta
* http://robomaa.fi/index.php?route=product/product&product_id=1648
* http://www.starelec.fi/product_info.php?products_id=32593
* https://www.partco.fi/en/arduino/arduino-models/137-arduino-micro.html

#### TDA7050 audio amplifier: (Finland)

* https://www.partco.fi/en/electronic-components/actives/integrated-circuits/ic-tda/1331-tda7050.html
* https://www.partco.fi/en/electronic-components/actives/ic-sockets/2095-p8.html (DIL8 socket)

## Use of external keyers

When using this adapter in straight key mode, an external keyer, such as the
[K3NG CW Keyer](https://blog.radioartisan.com/arduino-cw-keyer/) can be connected
to this adapter to provide additional functionality.

The CW key output of an external keyer should be connected to pin D2
of the Arduino board. The incoming signal must be pull D2 pin to GND
when active. Optionally, PTT control from an external keyer can be connected
to pin D0 (active low). 

## Flashing Arduino firmware

Follow the operating system-specific instructions below to flash the morse key adapter firmware
on an Arduino Micro or Arduino Pro Micro.

### Windows

* Download the latest binary firmware file from `release` directory
  * For Arduino Micro: [wrc-morse-key-adapter-v2-micro-20211004.zip](release/wrc-morse-key-adapter-v2-micro-20211004.zip?raw=true).
  * For Arduino Pro Micro: [wrc-morse-key-adapter-v2-promicro-20211004.zip](release/wrc-morse-key-adapter-v2-promicro-20211004.zip?raw=true).
* **Extract the HEX firmware binary file inside the ZIP file**
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
avrdude -C ..\etc\avrdude.conf -c avr109 -p atmega32u4 -P COM4 -b 57600 -D -U flash:w:wrc-morse-key-adapter-v2-micro-20211004.hex
```

### Linux

* Download the latest binary firmware file from `release` directory
  * For Arduino Micro: [wrc-morse-key-adapter-v2-micro-20211004.zip](release/wrc-morse-key-adapter-v2-micro-20211004.zip?raw=true).
  * For Arduino Pro Micro: [wrc-morse-key-adapter-v2-promicro-20211004.zip](release/wrc-morse-key-adapter-v2-promicro-20211004.zip?raw=true).
* **Extract the HEX firmware binary file inside the ZIP file**
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
./hardware/tools/avr/bin/avrdude -C ./hardware/tools/avr/etc/avrdude.conf -c avr109 -p atmega32u4 -P /dev/ttyACM1 -b 57600 -D -U flash:w:wrc-morse-key-adapter-v2-micro-20211004.hex
```

## Developer documentation

Download and install [PlatformIO](https://platformio.org/) embedded development platform. Building the
firmware requires at least the [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/index.html)
command-line tools to be installed.

Execute the following command to build the firmware binary from the source code.
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
