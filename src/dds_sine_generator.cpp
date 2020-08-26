/**
 * Arduino USB Morse Key adapter for Web Radio Control amateur radio remote control software
 * Copyright (C) 2020 Mikael Nousiainen OH3BHX <mikael.nousiainen@iki.fi>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * PWM DDS sine generator code based on:
 * http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-dds-sinewave-generator/
 *
 * DDS Sine Generator for ATMEGA 168
 * Timer2 generates 31250 KHz clock interrupt
 *
 * KHM 2009 /  Martin Nawrath
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne
 */

#include <Arduino.h>
#include <avr/pgmspace.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// REFCLK=16MHz / 510
// #define REFCLK 31372.549
// Measured REFCLK
#define REFCLK 31376.6

// Table of 256 sine values, one sine period, stored in flash memory
PROGMEM const uint8_t sine256[] = {
        127, 130, 133, 136, 139, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 176, 178, 181, 184, 187, 190,
        192, 195, 198, 200, 203, 205, 208, 210, 212, 215, 217, 219, 221, 223, 225, 227, 229, 231, 233, 234, 236, 238,
        239, 240,
        242, 243, 244, 245, 247, 248, 249, 249, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254,
        253, 253, 253, 252, 252, 251, 250, 249, 249, 248, 247, 245, 244, 243, 242, 240, 239, 238, 236, 234, 233, 231,
        229, 227, 225, 223,
        221, 219, 217, 215, 212, 210, 208, 205, 203, 200, 198, 195, 192, 190, 187, 184, 181, 178, 176, 173, 170, 167,
        164, 161, 158, 155, 152, 149, 146, 143, 139, 136, 133, 130, 127, 124, 121, 118, 115, 111, 108, 105, 102, 99, 96,
        93, 90, 87, 84, 81, 78,
        76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 16, 15,
        14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10,
        11, 12, 14, 15, 16, 18, 20, 21, 23, 25, 27, 29, 31,
        33, 35, 37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76, 78, 81, 84, 87, 90, 93, 96, 99, 102,
        105, 108, 111, 115, 118, 121, 124
};

volatile bool pwmEnabled = false;
volatile uint32_t pwmInterruptCounter = 0;
volatile unsigned long phaseAccumulator;
volatile unsigned long ddsTuningWord;

uint32_t millisToPwmTicks(double milliseconds)
{
    return milliseconds * 125.0 / 4.0;
}

uint32_t getPwmTicks()
{
    return pwmInterruptCounter;
}

// Timer setup
// Set prescaler to 1, PWM mode to phase correct PWM, 16000000/510 = 31372.55 Hz clock
void pwmInitTimer()
{
    // Timer Clock Prescaler to : 1
    sbi(TCCR4B, CS40);
    cbi(TCCR4B, CS41);
    cbi(TCCR4B, CS42);
    cbi(TCCR4B, CS43);

    // Timer PWM Mode set to Phase Correct PWM
    // Clear Compare Match
    cbi(TCCR4A, COM4A0);
    sbi(TCCR4A, COM4A1);

    sbi(TCCR4A, PWM4A);

    // Mode 1  / Phase Correct PWM
    sbi(TCCR4D, WGM40);
    cbi(TCCR4D, WGM41);
}

void pwmSetFrequency(double frequency)
{
    ddsTuningWord = pow(2, 32) * frequency / REFCLK;
}

void pwmInit(double frequency)
{
    // PWM frequency output
    pinMode(13, OUTPUT);

    // Pin 7 can be used to debug timing
    // pinMode(7, OUTPUT);

    pwmInitTimer();

    // Disable interrupts to avoid timing distortion
    // Disable Timer0 -- delay() is now not available
    cbi(TIMSK0, TOIE0);

    // Enable timer interrupt
    sbi(TIMSK4, TOIE4);

    // Disable timer interrupt
    //cbi(TIMSK4, TOIE4);

    pwmSetFrequency(frequency);
}

void pwmSetEnabled(bool enabled)
{
    pwmEnabled = enabled;
}

bool pwmIsEnabled()
{
    return pwmEnabled;
}

// Timer4 Interrupt Service at 31372,550 KHz = 32uSec
// This is the timebase REFCLOCK for the DDS generator
// FOUT = (M (REFCLK)) / (2 exp 32)
// Runtime: 8 microseconds (including push and pop)
ISR(TIMER4_OVF_vect)
{
    // Toggle PORTD, pin 7 to observe timing with a scope
    // sbi(PORTD, 7);

    // Soft DDS, use phase accumulator with 32 bits
    phaseAccumulator = phaseAccumulator + ddsTuningWord;
    // Use upper 8 bits of phase accumulator as frequency information
    byte pwmSineIndex = phaseAccumulator >> 24;

    if (pwmEnabled) {
        // Read value from sine table and send to PWM DAC
        OCR4A = pgm_read_byte_near(sine256 + pwmSineIndex);
    } else {
        OCR4A = 0;
    }

    pwmInterruptCounter++;

    // cbi(PORTD, 7);
}
