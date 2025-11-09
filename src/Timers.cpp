/*
 * Timers.c
 *
 *  Created on: 24.09.2017
 *      Author: anonymous
 */

#include "globals.h"
#include "defines.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_types.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp32-hal-timer.h"

extern "C" {

portMUX_TYPE muxer1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE muxer2 = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR timer1ISR() {
	portENTER_CRITICAL_ISR(&muxer1);

	if (dir_M1 != 0) {
		// Generate step pulse - longer pulse for better driver compatibility
		digitalWrite(PIN_MOTOR1_STEP, HIGH);
		delayMicroseconds(2); // 2us pulse width
		digitalWrite(PIN_MOTOR1_STEP, LOW);

		// Update step counter
		if (dir_M1 > 0)
			steps1--;
		else
			steps1++;
	}

	portEXIT_CRITICAL_ISR(&muxer1);
}

void IRAM_ATTR timer2ISR() {
	portENTER_CRITICAL_ISR(&muxer2);

	if (dir_M2 != 0) {
		// Generate step pulse - longer pulse for better driver compatibility
		digitalWrite(PIN_MOTOR2_STEP, HIGH);
		delayMicroseconds(2); // 2us pulse width
		digitalWrite(PIN_MOTOR2_STEP, LOW);

		// Update step counter
		if (dir_M2 > 0)
			steps2--;
		else
			steps2++;
	}

	portEXIT_CRITICAL_ISR(&muxer2);
}
}

void initTimers() {
	Serial.println("Initializing motor timers...");
	
	// Initialize Timer 1 for Motor 1
	timer1 = timerBegin(0, 40, true); // Timer 0, prescaler 40 (2MHz), count up
	if (timer1 == NULL) {
		Serial.println("ERROR: Failed to initialize timer1");
		return;
	}
	timerAttachInterrupt(timer1, &timer1ISR, true);
	timerAlarmWrite(timer1, ZERO_SPEED, true);
	timerAlarmDisable(timer1); // Start disabled

	// Initialize Timer 2 for Motor 2
	timer2 = timerBegin(1, 40, true); // Timer 1, prescaler 40 (2MHz), count up
	if (timer2 == NULL) {
		Serial.println("ERROR: Failed to initialize timer2");
		return;
	}
	timerAttachInterrupt(timer2, &timer2ISR, true);
	timerAlarmWrite(timer2, ZERO_SPEED, true);
	timerAlarmDisable(timer2); // Start disabled

	Serial.println("Motor timers initialized successfully");
}