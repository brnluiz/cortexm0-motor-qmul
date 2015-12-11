/*----------------------------------------------------------------------------
	Code for Lab 6

	In this project
		   - the red light flashes on a periodically 
			 - pressing the button toggles the green light
		There are two tasks
	   t_button: receives an event from the button ISR, and toggles green
	   t_led: switches the red on and off, using a delay

	REQUIRED CHANGES
		Change the program so that 
		   1. The leds flash: red/green/blue with each on for 3sec
		   2. Pressing the button stops the fashing; the next press restarts it
			 The response to the button has no noticable delay
			 The flashing continues with the same colour
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <MKL25Z4.H>
#include "GpioDefs.h"
#include "Settings.h"
#include "StepperMotor.h"
#include "Leds.h"
#include "MotorMode.h"
#include "Pit.h"
#include "Init.h"

#include "stdlib.h"

OS_TID t_mode_evt_mngr;
OS_TID t_reset_evt_mngr;
OS_TID t_tasks[TOTAL_TASKS]; /*  task ids */

// Port D IRQ Handler
void PORTD_IRQHandler(void) {  
	NVIC_ClearPendingIRQ(PORTD_IRQn);
	// Send the BUTTON1 event to the mode manager
	if ((PORTD->ISFR & MASK(BUTTON_POS))) {
		isr_evt_set (MODE_BTN_PRESSED, t_mode_evt_mngr);
	}
	// Send the BUTTON2 event to the reset manager
	if ((PORTD->ISFR & MASK(BUTTON2_POS))) {
		isr_evt_set (RESET_BTN_PRESSED, t_reset_evt_mngr);
	}

	// Clear status flags 
	PORTD->ISFR = 0xffffffff; 
}

// Helper: setup and starting the timer
void setupTimer(int speed) {
	stopTimer(0);
	setTimer(0, speed);
	startTimer(0);
}

// Helper: setup the motor return in the fastest possible way
void setupMotorReturn(void) {
	int32_t returnSteps;
	bool    rotation;
	
	// Get the number of actual steps (considering the initial step)
	if(motorMode.rotation == MOTOR_CLCK) {
		returnSteps = getSteps(m1) - motorMode.initStep;
	} else {
		returnSteps = motorMode.initStep - getSteps(m1);
	}
	
	// Get the shortest way to move the motor backwards (inverting the rotation)
	returnSteps = returnSteps % 48;
	rotation = !motorMode.rotation;

	// Check if there is a faster way to return
	if (returnSteps > STEPS_HALF) {
		// Invert the return steps and rotation
		returnSteps = STEPS - returnSteps;
		rotation = !rotation;
	}
	
	// Configure the timer (move the motor on the right speed) and start it
	setupTimer((float)((float)FAST_RETURN_TIME/(float)returnSteps) * PIT_SEC);
	
	// If there returnSteps > 0, setup the return movement, else don't do nothing
	if(returnSteps > 0) {
		moveSteps(m1, returnSteps, rotation);
	}
}

// Helper: Clear all possible events associated with some TID
void clearEvents(OS_TID tid) {
	os_evt_clr (MODE_BTN_PRESSED, tid);
	os_evt_clr (RESET_BTN_PRESSED, tid);
	os_evt_clr (RESET_DONE, tid);
	os_evt_clr (RETURN_ENABLED, tid);
}

// Timer 0 interrupt handler
void PIT_IRQHandler(void) {
	// Clear pending interrupts
	NVIC_ClearPendingIRQ(PIT_IRQn);

	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
		// Clear TIF
		PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK ;
		
		// Update the motor status (this will make the motor move)
		updateMotor(m1);
	}
}

// Task to feedback user click with LEDs
__task void ledFeedbackTask(void) {
	int state = COLOR_RED;
	greenLEDOnOff(LED_OFF);
	
	while(1) {
		// Wait for the user click on BUTTON1
		os_evt_wait_and (MODE_BTN_PRESSED, 0xFFFF);
		switch (state) {
			case COLOR_RED:
				redLEDOnOff   (LED_ON);
				blueLEDOnOff  (LED_OFF);
				
				state = COLOR_BLUE;
				break;
			case COLOR_BLUE:
				redLEDOnOff  (LED_OFF);
				blueLEDOnOff (LED_ON);
				
				state = COLOR_RED;
				break;
		}
		
		// Discard pending notifications
		clearEvents(t_tasks[T_LEDS]);
	}
}

// Button Event Manager Task
__task void modeBtnEventManagerTask(void) {
	int i = 0;

	while(1) {
		// Wait until button is pressed
		os_evt_wait_and (MODE_BTN_PRESSED, 0xffff);
		
		// Propagate the event through all listeners
		for(i=0; i<TOTAL_TASKS;i++) {
			// TODO: Check if it is 0... If is not, then send the event
			os_evt_set (MODE_BTN_PRESSED, t_tasks[i]);
		}
		
		// Wait some time to debounce the buton
		os_dly_wait(DEBOUNCE_TIMEOUT);
		
		// Discard pending notifications
		clearEvents(t_mode_evt_mngr);
	}
}

// Button 2 Event Manager Task
__task void resetBtnEventManagerTask(void) {
	int i = 0;

	while(1) {
		// Wait until button is pressed
		os_evt_wait_and (RESET_BTN_PRESSED, 0xffff);
		
		// Propagate the event through all listeners
		for(i=0; i<TOTAL_TASKS;i++) {
			// TODO: Check if it is 0... If is not, then send the event
			os_evt_set (RESET_BTN_PRESSED, t_tasks[i]);
		}
		
		// Wait some time to debounce the buton
		os_dly_wait(DEBOUNCE_TIMEOUT);
		
		// Discard pending notifications
		clearEvents(t_reset_evt_mngr);
	}
}

// Motor Control task, responsible to start the execution of the movement
__task void controlMotorTask(void) {
	FSMControlStates state = ST_CONTROL_START;
	
	while(1) {
		// If the user pressed the BUTTON2, wait until the motor is successful reset
		if(os_evt_wait_and (RESET_BTN_PRESSED, 1) == OS_R_EVT) {
			state = ST_CONTROL_RESETWAIT;
		}
		
		switch (state) {
			case ST_CONTROL_START:
				os_evt_wait_and (MODE_BTN_PRESSED, 0xFFFF); 
			
				// Setup the movement
				stopMotor(m1);
				motorMode.initStep = getSteps(m1);
				moveSteps(m1, motorMode.steps, motorMode.rotation) ;

				// Configure the timer (move the motor on the right speed) and start it
				setupTimer(motorMode.speed);
				
				state = ST_CONTROL_GO;			
				break ;
			
			case ST_CONTROL_GO:
				// If it already finished the movement
				if(!isMoving(m1)) {
					os_evt_wait_and (RETURN_ENABLED, 0xFFFF); 
					setupMotorReturn();
					state = ST_CONTROL_RETURN;
				}
				break;

			case ST_CONTROL_RESETWAIT:
				// Wait until the Reset task signal that it already reset the motor
				os_evt_wait_and (RESET_DONE, 0xFFFF); 
				state = ST_CONTROL_RETURN;
				break;
			
			case ST_CONTROL_RETURN:
				// Wait until the motor stop the backward movement
				if(!isMoving(m1)) {
					// Setups the next movement
					motorMode.next(&motorMode);
					state = ST_CONTROL_START;
				}
				break;
		}
		
		clearEvents(t_tasks[T_CONTROL_MOTOR]);
	}
}

// Reset Motor task, responsible to stop it and reset it to the beginning
__task void resetMotorTask(void) {
	FSMResetStates state = ST_RESET_START;
	bool stop = false;
	
	while(1) {
		switch(state) {
			case ST_RESET_START:
				os_evt_wait_and(RESET_BTN_PRESSED, 0xFFFF); 
			
				// If it was moving and was stopped by this task
				if(!isMoving(m1) && stop) {
					state = ST_RESET_SETUPRETURN;
				}
				// If it is moving and reset button was pressed, stop the motor
				else if(isMoving(m1) && !stop) {
					state = ST_RESET_STOP;
				}
				// Motor waiting to return
				else if(!isMoving(m1) && !stop) {
					os_evt_set (RETURN_ENABLED, t_tasks[T_CONTROL_MOTOR]);
					stop = false;
				}
				break;
				
			case ST_RESET_STOP:
				// Stop the motor
				stopMotor(m1);
				
				if(!isMoving(m1)) {
					// Signal that the motor was stopped (enabling the "reset to initial position")
					stop = true;
					state = ST_RESET_START;
				}
				break;

			case ST_RESET_SETUPRETURN:
				setupMotorReturn();
				state = ST_RESET_RETURN;
				break;
			
			case ST_RESET_RETURN:
				// Wait until the motor stop the backward movement
				if(!isMoving(m1)) {
					// Send a signal to the Control task that the reset is done
					os_evt_set (RESET_DONE, t_tasks[T_CONTROL_MOTOR]);
					state = ST_RESET_START;
					stop = false;
				}
				break;
		}
		clearEvents(t_tasks[T_RESET_MOTOR]);
	}
}

// Init all tasks
__task void boot (void) {
  t_mode_evt_mngr  = os_tsk_create (modeBtnEventManagerTask, 0);
	t_reset_evt_mngr = os_tsk_create (resetBtnEventManagerTask, 0);
	
	t_tasks[T_LEDS]          = os_tsk_create (ledFeedbackTask, 0);
	t_tasks[T_CONTROL_MOTOR] = os_tsk_create (controlMotorTask, 0);
	t_tasks[T_RESET_MOTOR]   = os_tsk_create (resetMotorTask, 0);
	
	os_tsk_delete_self ();
}

int main (void) {
	// Initialize input and outputs
	initOutputLeds();
	initInputButton();
	initTimers() ;
	setupMotor();
	
	// Initialize RTX and start init
	os_sys_init(boot);
}
