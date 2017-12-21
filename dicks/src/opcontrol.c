/** @file opcontrol.c
 * @brief File for operator control code
 *
 * This file should contain the user operatorControl() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "main.h"
#include "string.h"
/*
 * Runs the user operator control code. This function will be started in its own task with the
 * default priority and stack size whenever the robot is enabled via the Field Management System
 * or the VEX Competition Switch in the operator control mode. If the robot is disabled or
 * communications is lost, the operator control task will be stopped by the kernel. Re-enabling
 * the robot will restart the task, not resume it from where it left off.
 *
 * If no VEX Competition Switch or Field Management system is plugged in, the VEX Cortex will
 * run the operator control task. Be warned that this will also occur if the VEX Cortex is
 * tethered directly to a computer via the USB A to A cable without any VEX Joystick attached.
 *
 * Code running in this task can take almost any action, as the VEX Joystick is available and
 * the scheduler is operational. However, proper use of delay() or taskDelayUntil() is highly
 * recommended to give other tasks (including system tasks such as updating LCDs) time to run.
 *
 * This task should never exit; it should end with some kind of infinite loop, even if empty.
 */
#define threshold(x) (abs(x)>24?x:0)

#define mogoVal joystickGetAnalog('2','3')
#define mogoVal_actual joystickGetDigital(1, 5, JOY_UP)?-127:(joystickGetDigital(1, 5, JOY_DOWN)?127:0)

#define Y joystickGetAnalog(1, 3)
#define R joystickGetAnalog(1, 1)
#define X joystickGetAnalog(1, 4)
#define slaveR joystickGetAnalog(2, 1)
#define drive threshold(Y)
#define strafe threshold(X)
#define rotate threshold(R)
#define rotateSlave threshold(slaveR)
#define lift joystickGetAnalog(2,3)

#define claw joystickGetDigital(2, 6, JOY_UP)?-127:(joystickGetDigital(2, 6, JOY_DOWN)?127:0)
#define swing joystickGetAnalog(2,2)
#define grabbyMcGrabberson joystickGetDigital(1, 7, JOY_DOWN)?127:(joystickGetDigital(1, 7, JOY_RIGHT)?-127:0)

void hc05Init(char uart, bool atMode);

char *bluetoothRead(char uart);

void displaysensordata() {

  int le;
  int us;
  unsigned int bt;

//  bprintf(uart1, "%c[2J", 27);  //Clear Screen
//  bprintf(uart1, "%c[H", 27);   // Top left corner
  bprintf(uart1, " \r\n \r\n \r\n",27);   // Top left corner
  us = ultrasonicGet(dexterUS);
  bprintf(uart1, "Ultrasonic: %d\n", us);
  le = encoderGet(liftEnc);
  bprintf(uart1, "Lift Encoder: %d\n", le);
  bt = powerLevelMain();
  bprintf(uart1, "Main Battery Voltage: %0.4fV\n", ((float)bt) / 1000);

    bprintf(uart1, " \r\n \r\n \r\n",27);
      delay(1000);
}


void blueListen(char * message) {
    printf("%x %x %x %x %x %x %x %x\n", message[0], message[1], message[2], message[3], message[4], message[5], message[6], message[7]);
    printf("|%s|\n", message);
    printf("%d\n", strcmp(message, "reset\r\n"));
    if (strcmp(message, "reset\r\n") == 0) {
      analogCalibrate(LINE_TRACKER_PORT);
      fprint("Reset Sensor\r\n", uart1);
      delay(2000);
    }
    else if(strcmp(message, "ping\r\n") == 0) {
      bprintf(uart1, "pong", 27);
  }
}





void operatorControl() {
	hc05Init(1, false);
	blisten(1, blueListen);

	printf("crap");
	int downshift = 1;
	bool lastButton6D = 0;
	bool lastButton6U = 0;
	while (1) {
		displaysensordata();
		motorSet(1, claw);
		motorSet(2, mogoVal_actual);
		motorSet(3, (drive + rotate)/downshift);
		motorSet(4, (drive + rotate)/downshift);
		motorSet(5, lift);
		motorSet(6, lift);
		motorSet(7, (-drive + rotate)/downshift);
		motorSet(8, swing);
		motorSet(9, (-drive + rotate)/downshift);
		motorSet(10, grabbyMcGrabberson);

		if(joystickGetDigital(1, 6, JOY_DOWN)) {
			if(lastButton6D & joystickGetDigital(1, 6, JOY_DOWN)) {
				downshift = 3;
			}
			lastButton6D = joystickGetDigital(1, 6, JOY_DOWN);
		}
		if(joystickGetDigital(1, 6, JOY_UP)) {
			if(lastButton6U & joystickGetDigital(1, 6, JOY_UP)) {
				downshift = 1;
			}
			lastButton6U = joystickGetDigital(1, 6, JOY_UP);
		}
	}
}
