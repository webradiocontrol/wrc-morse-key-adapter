/**
 * Arduino USB Morse Key adapter for Web Radio Control amateur radio remote control software
 * Copyright (C) 2020-2021 Mikael Nousiainen OH3BHX <mikael@webradiocontrol.tech>
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
 */

#ifndef WRC_MORSE_KEY_ADAPTER_DDS_SINE_GENERATOR_H
#define WRC_MORSE_KEY_ADAPTER_DDS_SINE_GENERATOR_H

#include <Arduino.h>

uint32_t millisToPwmTicks(double milliseconds);

uint32_t getPwmTicks();

void pwmInit(double frequency);

void pwmSetEnabled(bool enabled);

bool pwmIsEnabled();

void pwmSetFrequency(double frequency);

#endif
