# Web Radio Control morse key adapter

The default PlatformIO config and the source code settings assume use of Arduino Micro.

## Build

```bash
platformio run
```

## Flash

```bash
platformio run --target upload
```

## Flashing binary only

### Windows

* Install Arduino IDE for Windows to get the latest version of `avrdude.exe` that
  is a command-line application used to flash Arduino boards.
* Connect the Arduino to a USB port
* Find out the COM port for the Arduino using Device Manager in Control Panel.
  * Windows 10: Control Panel -> Hardware and Sound -> Device Manager.
  * The listing shows COM ports under: Ports (COM & LPT)
  * Look for: USB Serial Device (COMx)
* Execute the following command to reset Arduino and to switch it to mode for flashing.
  Remember to adjust the COM port in the command (COM3 in the example).
```bash
avrdude -p atmega32u4 -c arduino -P COM3 -b 1200
  ```
* Execute the following command to flash the new firmware in file `firmware.hex`.
  You will have only 8 seconds to do this until Arduino returns back to normal mode.
  Note that the COM port changes once again, usually to a port with
  a number one higher than previously (COM4 in this example).
```
avrdude -c avr109 -p atmega32u4 -P COM4 -b 57600 -D -U flash:w:firmware.hex
```
