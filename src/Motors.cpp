/*
 * Motors.cpp
 *
 *  Created on: 25.09.2017
 *      Author: anonymous
 */

#include <Arduino.h>
#include "defines.h"
#include "globals.h"

// Set speed of Stepper Motor1
// tspeed could be positive or negative (reverse)
void setMotorSpeedM1(int16_t tspeed)
{
  long timer_period;
  int16_t speed;

  // WE LIMIT MAX ACCELERATION of the motors
  if ((speed_M1 - tspeed) > MAX_ACCEL)
    speed_M1 -= MAX_ACCEL;
  else if ((speed_M1 - tspeed) < -MAX_ACCEL)
    speed_M1 += MAX_ACCEL;
  else
    speed_M1 = tspeed;

  // Calculate actual motor speed based on microstepping
#if MICROSTEPPING==16
  speed = speed_M1 * 50; // 16 microsteps: higher resolution
#else
  speed = speed_M1 * 25; // 8 microsteps: lower resolution
#endif

  // Set direction and calculate timer period
  if (speed == 0)
  {
    timer_period = ZERO_SPEED;
    dir_M1 = 0;
    timerAlarmWrite(timer1, ZERO_SPEED, true);
    timerAlarmDisable(timer1); // Stop timer when speed is 0
  }
  else if (speed > 0)
  {
    timer_period = 2000000 / speed; // 2MHz timer base
    dir_M1 = 1;
    digitalWrite(PIN_MOTOR1_DIR, HIGH);
    timerAlarmWrite(timer1, timer_period, true);
    timerAlarmEnable(timer1); // Enable timer
  }
  else
  {
    timer_period = 2000000 / (-speed);
    dir_M1 = -1;
    digitalWrite(PIN_MOTOR1_DIR, LOW);
    timerAlarmWrite(timer1, timer_period, true);
    timerAlarmEnable(timer1); // Enable timer
  }
  
  // Limit minimum timer period to prevent overflow (only if not already set above)
  if (speed != 0) {
    if (timer_period > ZERO_SPEED)
      timer_period = ZERO_SPEED;
    if (timer_period < 100) // Minimum period for safety
      timer_period = 100;
  }
}

// Set speed of Stepper Motor2
// tspeed could be positive or negative (reverse)
void setMotorSpeedM2(int16_t tspeed)
{
  long timer_period;
  int16_t speed;

  // WE LIMIT MAX ACCELERATION of the motors
  if ((speed_M2 - tspeed) > MAX_ACCEL)
    speed_M2 -= MAX_ACCEL;
  else if ((speed_M2 - tspeed) < -MAX_ACCEL)
    speed_M2 += MAX_ACCEL;
  else
    speed_M2 = tspeed;

  // Calculate actual motor speed based on microstepping
#if MICROSTEPPING==16
  speed = speed_M2 * 50; // 16 microsteps: higher resolution
#else
  speed = speed_M2 * 25; // 8 microsteps: lower resolution
#endif

  // Set direction and calculate timer period
  if (speed == 0)
  {
    timer_period = ZERO_SPEED;
    dir_M2 = 0;
    timerAlarmWrite(timer2, ZERO_SPEED, true);
    timerAlarmDisable(timer2); // Stop timer when speed is 0
  }
  else if (speed > 0)
  {
    timer_period = 2000000 / speed; // 2MHz timer base
    dir_M2 = 1;
    digitalWrite(PIN_MOTOR2_DIR, LOW); // Motor 2 reverse direction
    timerAlarmWrite(timer2, timer_period, true);
    timerAlarmEnable(timer2); // Enable timer
  }
  else
  {
    timer_period = 2000000 / (-speed);
    dir_M2 = -1;
    digitalWrite(PIN_MOTOR2_DIR, HIGH); // Motor 2 forward direction
    timerAlarmWrite(timer2, timer_period, true);
    timerAlarmEnable(timer2); // Enable timer
  }
  
  // Limit minimum timer period to prevent overflow (only if not already set above)
  if (speed != 0) {
    if (timer_period > ZERO_SPEED)
      timer_period = ZERO_SPEED;
    if (timer_period < 100) // Minimum period for safety
      timer_period = 100;
  }
}

// Simple motor test function
void testMotorHardware() {
  Serial.println("=== MOTOR HARDWARE TEST ===");
  
  // Enable motors
  digitalWrite(PIN_ENABLE_MOTORS, LOW);
  delay(100);
  
  Serial.println("Testing Motor 1...");
  digitalWrite(PIN_MOTOR1_DIR, HIGH);
  for (int i = 0; i < 100; i++) {
    digitalWrite(PIN_MOTOR1_STEP, HIGH);
    delayMicroseconds(3000);
    digitalWrite(PIN_MOTOR1_STEP, LOW);
    delayMicroseconds(3000);
  }
  
  Serial.println("Testing Motor 2...");
  digitalWrite(PIN_MOTOR2_DIR, HIGH);
  for (int i = 0; i < 100; i++) {
    digitalWrite(PIN_MOTOR2_STEP, HIGH);
    delayMicroseconds(3000);
    digitalWrite(PIN_MOTOR2_STEP, LOW);
    delayMicroseconds(3000);
  }
  
  // Disable motors
  digitalWrite(PIN_ENABLE_MOTORS, HIGH);
  Serial.println("=== MOTOR TEST COMPLETE ===");
}