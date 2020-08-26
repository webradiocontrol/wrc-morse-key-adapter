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
 */

#include <Arduino.h>
#include <Keyboard.h>

#include "dds_sine_generator.h"

// Pin definitions

#define PIN_PTT 0

#define PIN_KEY_RING 2
#define PIN_KEY_TIP 3
#define PIN_KEY_AUTOMATIC_MODE 9
#define PIN_KEY_IAMBIC 10
#define PIN_KEY_INVERTED 11

#define PIN_ANALOG_KEYER_SPEED A0
#define PIN_ANALOG_KEYER_PITCH A1

#define PIN_STATE_KEY_ON LOW
#define PIN_STATE_PTT_ON LOW

// Keyboard definitions

#define KEYBOARD_KEY_MODIFIER_PTT KEY_LEFT_ALT
#define KEYBOARD_KEY_PTT_ON 'i'
#define KEYBOARD_KEY_PTT_OFF 'o'

#define KEYBOARD_KEY_STRAIGHT ','
#define KEYBOARD_KEY_PASS_THROUGH_DIT '.'
#define KEYBOARD_KEY_PASS_THROUGH_DAH '/'

// Automatic keyer definitions

#define KEYER_SYSTEM_VOLTAGE 5.00
#define KEYER_ANALOG_INPUT_REFERENCE_VOLTAGE 3.20
#define KEYER_ANALOG_INPUT_REFERENCE_MULTIPLIER (KEYER_ANALOG_INPUT_REFERENCE_VOLTAGE / KEYER_SYSTEM_VOLTAGE)

#define KEYER_SPEED_WPM_MINIMUM 5
#define KEYER_SPEED_WPM_MAXIMUM 50

#define KEYER_SPEED_WPM_MINIMUM_DELTA 1
#define KEYER_SPEED_WPM_MAXIMUM_ANALOG_VALUE (1023.0 * KEYER_ANALOG_INPUT_REFERENCE_MULTIPLIER)

#define KEYER_PITCH_MINIMUM 300
#define KEYER_PITCH_MAXIMUM 1200

#define KEYER_PITCH_MINIMUM_DELTA 1
#define KEYER_PITCH_MAXIMUM_ANALOG_VALUE (1023.0 * KEYER_ANALOG_INPUT_REFERENCE_MULTIPLIER)

// Defaults

#define KEYER_PITCH_DEFAULT 750.0
#define KEYER_SPEED_WPM_DEFAULT 20

// Definitions

#define INPUT_STATE_ON_CHANGED 3
#define INPUT_STATE_OFF_CHANGED 2
#define INPUT_STATE_ON 1
#define INPUT_STATE_OFF 0
#define INPUT_STATE_IGNORE -1

#define KEYER_ACTION_NONE 0
#define KEYER_ACTION_DIT 1
#define KEYER_ACTION_DAH 2

#define DEBOUNCE_FILTER_SAMPLE_COUNT 100

// Switch states

volatile bool isAutomaticKey = false;
volatile bool isAutomaticKeyInverted = false;
volatile bool isAutomaticKeyIambic = true;

volatile bool isPassThroughMode = false;

// Internal state

volatile uint32_t ditDurationTicks = 0;
volatile uint32_t dahDurationTicks = 0;
volatile uint32_t pauseDurationTicks = 0;
volatile uint32_t scheduleAheadTicks = 0;

volatile int rawStraightState = HIGH;
int previousRawStraightState = HIGH;
volatile int rawDitState = HIGH;
int previousRawDitState = HIGH;
volatile int rawDahState = HIGH;
int previousRawDahState = HIGH;

bool ditActive = false;
bool ditPending = false;
bool dahActive = false;
bool dahPending = false;

uint32_t lastScheduledEventStartTime = 0;
uint32_t lastScheduledEventEndTime = 0;
char lastScheduledEventAction = KEYER_ACTION_NONE;

uint16_t previousRawKeyerSpeed = 0;
uint16_t rawKeyerSpeed = 0;

uint16_t previousRawKeyerPitch = 0;
uint16_t rawKeyerPitch = 0;

volatile int rawPttState = HIGH;
int previousRawPttState = HIGH;

inline uint32_t getTicks()
{
    return getPwmTicks();
}

int debounceInput(volatile int *state, int *previousState, int onState)
{
    int initialState = *state;
    if (initialState == *previousState) {
        return initialState == onState ? INPUT_STATE_ON : INPUT_STATE_OFF;
    }

    // TODO: Debouncing does not work reliably here for pass-through mode
    for (int i = 0; i < DEBOUNCE_FILTER_SAMPLE_COUNT; i++) {
        if (*state != initialState) {
            return INPUT_STATE_IGNORE;
        }
        // Delay without interrupts
        // for (int j = 0; j < 10000; j++);
    }

    *previousState = initialState;

    return initialState == onState ? INPUT_STATE_ON_CHANGED : INPUT_STATE_OFF_CHANGED;
}

void keyerSetSpeedWpm(int wpm)
{
    // PARIS: 50 dot durations, 20 WPM -> 60ms per unit
    // CODEX: 60 dot durations, 20 WPM -> 50ms per unit
    double unitDurationMillis = (60.0 * 20.0) / (double) wpm; // Use PARIS

    ditDurationTicks = millisToPwmTicks(unitDurationMillis);
    dahDurationTicks = millisToPwmTicks(unitDurationMillis * 3.0);
    pauseDurationTicks = millisToPwmTicks(unitDurationMillis);

    scheduleAheadTicks = millisToPwmTicks(unitDurationMillis / 10.0);

#ifdef DEBUG_TIMING
    Serial.print("unit: ");
    Serial.println(unitDurationMillis);
    Serial.print("dit: ");
    Serial.println(ditDurationTicks);
    Serial.print("dah: ");
    Serial.println(dahDurationTicks);
    Serial.print("pause: ");
    Serial.println(pauseDurationTicks);
    Serial.print("ahead: ");
    Serial.println(scheduleAheadTicks);
#endif
}

void generatePassThroughKeyEvent(volatile int *state, int *previousState, const char *name, char key)
{
    int debouncedState = debounceInput(state, previousState, PIN_STATE_KEY_ON);

    switch (debouncedState) {
        case INPUT_STATE_ON_CHANGED:
            Keyboard.press(key);
            if (!isPassThroughMode) {
                pwmSetEnabled(true);
            }
            break;
        case INPUT_STATE_OFF_CHANGED:
            Keyboard.release(key);
            if (!isPassThroughMode) {
                pwmSetEnabled(false);
            }
            break;
        default:
            return;
    }
}

bool keyerIsSchedulingPossibleAt(uint32_t ticks)
{
    return ((lastScheduledEventEndTime + pauseDurationTicks) < ticks + scheduleAheadTicks);
}

bool keyerIsEventActiveAt(uint32_t ticks)
{
    return (ticks >= lastScheduledEventStartTime && ticks < lastScheduledEventEndTime);
}

void keyerKey(bool on, char key)
{
    if (on) {
        Keyboard.press(key);
    } else {
        Keyboard.release(key);
    }
    pwmSetEnabled(on);
}

void keyerScheduleEvent(uint32_t ticks, char action, uint32_t actionDurationTicks)
{
    if (ticks > lastScheduledEventEndTime + pauseDurationTicks) {
        lastScheduledEventStartTime = ticks;
        lastScheduledEventEndTime = lastScheduledEventStartTime + actionDurationTicks;
    } else {
        lastScheduledEventStartTime = lastScheduledEventEndTime + pauseDurationTicks;
        lastScheduledEventEndTime = lastScheduledEventStartTime + actionDurationTicks;
    }

    lastScheduledEventAction = action;

#ifdef DEBUG_SCHEDULING
    Serial.print("schedule ticks: ");
    Serial.print(ticks);
    Serial.print(" start: ");
    Serial.print(lastScheduledEventStartTime);
    Serial.print(" end: ");
    Serial.println(lastScheduledEventEndTime);
#endif
}

void keyerKeyIfActive(char key, uint32_t ticks, bool *isActive)
{
    bool eventActive = keyerIsEventActiveAt(ticks);
#ifdef DEBUG_KEY
    Serial.print("event active: ");
    Serial.println(eventActive);
#endif

    if (!eventActive && (isActive != NULL && *isActive)) {
        keyerKey(false, key);
        *isActive = false;
    } else if (eventActive && (isActive != NULL && !*isActive)) {
        keyerKey(true, key);
        *isActive = true;
    }
}

void keyerHandleActionChange(char action, int actionState, uint32_t actionDurationTicks,
        bool *pending, bool *otherPending, uint32_t ticks)
{
    bool scheduleNewEvent = keyerIsSchedulingPossibleAt(ticks);

    switch (actionState) {
        case INPUT_STATE_ON_CHANGED:
            if (scheduleNewEvent) {
                keyerScheduleEvent(ticks, action, actionDurationTicks);
            } else if (lastScheduledEventAction != action) {
                *pending = true;
            }
            break;
        case INPUT_STATE_ON:
            if (scheduleNewEvent && !*otherPending) {
                *pending = false;
                keyerScheduleEvent(ticks, action, actionDurationTicks);
            }
            break;
        case INPUT_STATE_OFF_CHANGED:
        case INPUT_STATE_OFF:
            if (*pending && scheduleNewEvent && !*otherPending) {
                *pending = false;
                keyerScheduleEvent(ticks, action, actionDurationTicks);
            }
            break;
    }
}

void keyerGenerateEvent(int ditState, int dahState, char key)
{
    uint32_t ticks = getTicks();

    if (lastScheduledEventAction == KEYER_ACTION_DIT) {
        if (isAutomaticKeyIambic) {
            keyerHandleActionChange(KEYER_ACTION_DAH, dahState, dahDurationTicks, &dahPending, &ditPending, ticks);
            keyerHandleActionChange(KEYER_ACTION_DIT, ditState, ditDurationTicks, &ditPending, &dahPending, ticks);
        } else {
            keyerHandleActionChange(KEYER_ACTION_DIT, ditState, ditDurationTicks, &ditPending, &dahPending, ticks);
            keyerHandleActionChange(KEYER_ACTION_DAH, dahState, dahDurationTicks, &dahPending, &ditPending, ticks);
        }
    } else {
        if (isAutomaticKeyIambic) {
            keyerHandleActionChange(KEYER_ACTION_DIT, ditState, ditDurationTicks, &ditPending, &dahPending, ticks);
            keyerHandleActionChange(KEYER_ACTION_DAH, dahState, dahDurationTicks, &dahPending, &ditPending, ticks);
        } else {
            keyerHandleActionChange(KEYER_ACTION_DAH, dahState, dahDurationTicks, &dahPending, &ditPending, ticks);
            keyerHandleActionChange(KEYER_ACTION_DIT, ditState, ditDurationTicks, &ditPending, &dahPending, ticks);
        }
    }

    bool *active = NULL;
    switch (lastScheduledEventAction) {
        case KEYER_ACTION_DIT:
            active = &ditActive;
            break;
        case KEYER_ACTION_DAH:
            active = &dahActive;
            break;
    }

    keyerKeyIfActive(key, ticks, active);
}

void keyerHandleSpeedChange()
{
    rawKeyerSpeed = analogRead(PIN_ANALOG_KEYER_SPEED);

    // Ignore small changes
    if (rawKeyerSpeed >= previousRawKeyerSpeed - KEYER_SPEED_WPM_MINIMUM_DELTA &&
        rawKeyerSpeed <= previousRawKeyerSpeed + KEYER_SPEED_WPM_MINIMUM_DELTA) {
        return;
    }
    previousRawKeyerSpeed = rawKeyerSpeed;

    int speedWpm = KEYER_SPEED_WPM_MINIMUM +
                   ((KEYER_SPEED_WPM_MAXIMUM - KEYER_SPEED_WPM_MINIMUM) / KEYER_SPEED_WPM_MAXIMUM_ANALOG_VALUE) *
                   (double) rawKeyerSpeed;
    if (speedWpm > KEYER_SPEED_WPM_MAXIMUM) {
        speedWpm = KEYER_SPEED_WPM_MAXIMUM;
    }
    keyerSetSpeedWpm(speedWpm);

#ifdef DEBUG_CONTROLS
    Serial.print("Raw Speed: ");
    Serial.print((int) rawKeyerSpeed);
    Serial.println();

    Serial.print("Speed: ");
    Serial.print(speedWpm);
    Serial.println();
#endif
}

void keyerHandlePitchChange()
{
    rawKeyerPitch = analogRead(PIN_ANALOG_KEYER_PITCH);

    // Ignore small changes
    if (rawKeyerPitch >= previousRawKeyerPitch - KEYER_PITCH_MINIMUM_DELTA &&
        rawKeyerPitch <= previousRawKeyerPitch + KEYER_PITCH_MINIMUM_DELTA) {
        return;
    }
    previousRawKeyerPitch = rawKeyerPitch;

    double pitch =
            KEYER_PITCH_MINIMUM +
            ((KEYER_PITCH_MAXIMUM - KEYER_PITCH_MINIMUM) / KEYER_PITCH_MAXIMUM_ANALOG_VALUE) * (double) rawKeyerPitch;
    if (pitch > KEYER_PITCH_MAXIMUM) {
        pitch = KEYER_PITCH_MAXIMUM;
    }
    pwmSetFrequency(pitch);

#ifdef DEBUG_CONTROLS
    Serial.print("Raw Pitch: ");
    Serial.print((int) rawKeyerPitch);
    Serial.println();

    Serial.print("Pitch: ");
    Serial.print((int) pitch);
    Serial.println();
#endif
}

inline void handleInterruptAndReadPin(int pin, volatile int *state)
{
    *state = digitalRead(pin);
}

void pinChangeHandleRing()
{
    if (!isAutomaticKey) {
        return;
    }

    if (isAutomaticKeyInverted) {
        handleInterruptAndReadPin(PIN_KEY_RING, &rawDitState);
    } else {
        handleInterruptAndReadPin(PIN_KEY_RING, &rawDahState);
    }
}

void pinChangeHandleTip()
{
    if (isAutomaticKey) {
        if (isAutomaticKeyInverted) {
            handleInterruptAndReadPin(PIN_KEY_TIP, &rawDahState);
        } else {
            handleInterruptAndReadPin(PIN_KEY_TIP, &rawDitState);
        }
    } else {
        handleInterruptAndReadPin(PIN_KEY_TIP, &rawStraightState);
    }
}

void pinChangeHandlePtt()
{
    handleInterruptAndReadPin(PIN_PTT, &rawPttState);
}

inline void readPinToBoolean(int pin, volatile bool *value)
{
    int state = digitalRead(pin);
    *value = state == HIGH;
}

void setPtt(bool on)
{
    if (on) {
#ifdef KEYBOARD_KEY_MODIFIER_PTT
        Keyboard.press(KEYBOARD_KEY_MODIFIER_PTT);
#endif
        Keyboard.press(KEYBOARD_KEY_PTT_ON);
        Keyboard.release(KEYBOARD_KEY_PTT_ON);
#ifdef KEYBOARD_KEY_MODIFIER_PTT
        Keyboard.release(KEYBOARD_KEY_MODIFIER_PTT);
#endif
#ifdef DEBUG_PTT
        Serial.println("PTT on");
#endif
    } else {
#ifdef KEYBOARD_KEY_MODIFIER_PTT
        Keyboard.press(KEYBOARD_KEY_MODIFIER_PTT);
#endif
        Keyboard.press(KEYBOARD_KEY_PTT_OFF);
        Keyboard.release(KEYBOARD_KEY_PTT_OFF);
#ifdef KEYBOARD_KEY_MODIFIER_PTT
        Keyboard.release(KEYBOARD_KEY_MODIFIER_PTT);
#endif
#ifdef DEBUG_PTT
        Serial.println("PTT off");
#endif
    }
}

void handlePttChange()
{
    int debouncedState = debounceInput(&rawPttState, &previousRawPttState, PIN_STATE_PTT_ON);

    switch (debouncedState) {
        case INPUT_STATE_ON_CHANGED:
            setPtt(true);
            break;
        case INPUT_STATE_OFF_CHANGED:
            setPtt(false);
            break;
        default:
            break;
    }
}

void setup()
{
    // while (!Serial);

    Serial.begin(115200);
    Serial.println("USB Morse Key adapter initializing");

    pinMode(PIN_KEY_RING, INPUT_PULLUP);
    pinMode(PIN_KEY_TIP, INPUT_PULLUP);

    pinMode(PIN_KEY_AUTOMATIC_MODE, INPUT_PULLUP);
    pinMode(PIN_KEY_IAMBIC, INPUT_PULLUP);
    pinMode(PIN_KEY_INVERTED, INPUT_PULLUP);

    pinMode(PIN_PTT, INPUT_PULLUP);

    pinMode(PIN_ANALOG_KEYER_PITCH, INPUT);
    pinMode(PIN_ANALOG_KEYER_SPEED, INPUT);

    pwmInit(KEYER_PITCH_DEFAULT);
    keyerSetSpeedWpm(KEYER_SPEED_WPM_DEFAULT);

    attachInterrupt(digitalPinToInterrupt(PIN_KEY_RING), pinChangeHandleRing, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_KEY_TIP), pinChangeHandleTip, CHANGE);

    attachInterrupt(digitalPinToInterrupt(PIN_PTT), pinChangeHandlePtt, CHANGE);

    Keyboard.begin();
}

void loop()
{
    readPinToBoolean(PIN_KEY_AUTOMATIC_MODE, &isAutomaticKey);
    readPinToBoolean(PIN_KEY_IAMBIC, &isAutomaticKeyIambic);
    readPinToBoolean(PIN_KEY_INVERTED, &isAutomaticKeyInverted);

    if (!pwmIsEnabled()) {
        // Read analog inputs only when PWM (CW sidetone) is not active to minimize changes caused by voltage fluctuations
        keyerHandleSpeedChange();
        keyerHandlePitchChange();
    }

    handlePttChange();

    if (isPassThroughMode) {
        if (isAutomaticKey) {
            generatePassThroughKeyEvent(&rawDitState, &previousRawDitState, "dit: ", KEYBOARD_KEY_PASS_THROUGH_DIT);
            generatePassThroughKeyEvent(&rawDahState, &previousRawDahState, "dah: ", KEYBOARD_KEY_PASS_THROUGH_DAH);
        } else {
            generatePassThroughKeyEvent(&rawStraightState, &previousRawStraightState,
                    "straight: ", KEYBOARD_KEY_STRAIGHT);
        }
    } else {
        if (isAutomaticKey) {
            int ditStateDebounced = debounceInput(&rawDitState, &previousRawDitState, PIN_STATE_KEY_ON);
            int dahStateDebounced = debounceInput(&rawDahState, &previousRawDahState, PIN_STATE_KEY_ON);

            keyerGenerateEvent(ditStateDebounced, dahStateDebounced, KEYBOARD_KEY_STRAIGHT);
        } else {
            generatePassThroughKeyEvent(&rawStraightState, &previousRawStraightState,
                    "straight: ", KEYBOARD_KEY_STRAIGHT);
        }
    }
}
