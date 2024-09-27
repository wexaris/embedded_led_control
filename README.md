# Embedded LED Control

Small test of serial controllable LEDs.
Based on the ATmega328P in an Arduino Uno board.

Uses standard AVR headers bundled with an Arduino installation,
`Make` for build management, `avr-gcc` for compilation and `AVRDUDE` for upload.

## Build Options

Most build options can be configured at the top of the Makefile.
 * `LED_PIN_0` and `LED_PIN_1` values correspond to pins `[0-3]`.
 They match the ATmega328P's pins `[B5-B2]` and Arduino Uno's pins `[13-10]`
 * `SERIAL_BAUD_RATE` for serial configuration.
 * `PORT` specifies the port the device is connected to.

## Compilation

Run `make build` to compile the program, and `make install` to upload it to a device.