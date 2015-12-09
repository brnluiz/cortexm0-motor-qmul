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

#include "stdlib.h"

OS_TID t_mode_evt_mngr;
OS_TID t_reset_evt_mngr;
OS_TID t_tasks[TOTAL_TASKS]; /*  task ids */

motorType mcb ;   // motor control block
MotorId m1 ;      // motor id
MotorMode motorMode;

void setupMotor() {
	m1 = & mcb ;
	m1->port = PTE ;
  m1->bitAp = MOTOR_IN1 ;
  m1->bitAm = MOTOR_IN2 ;
  m1->bitBp = MOTOR_IN3 ;
  m1->bitBm = MOTOR_IN4 ;

	// Enable clock to port E
	SIM->SCGC5 |=  SIM_SCGC5_PORTE_MASK; /* enable clock for port E */
	
	// Initialise motor data and set to state 1
  initMotor(m1) ; // motor initially stopped, with step 1 powered
	
	motorMode = mode_construct();	
}

void initTimers() {
	configurePIT(0) ;

	setTimer(0, 2400000) ;  // 100ms

	startTimer(0) ;
}

void initInputButton(void) {
	SIM->SCGC5 |=  SIM_SCGC5_PORTD_MASK; /* enable clock for port D */

	/* Select GPIO and enable pull-up resistors and interrupts 
		on falling edges for pins connected to switches */
	PORTD->PCR[BUTTON_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0a);
	PORTD->PCR[BUTTON2_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0a);
		
	/* Set port D switch bit to inputs */
	PTD->PDDR &= ~MASK(BUTTON_POS);
	PTD->PDDR &= ~MASK(BUTTON2_POS);

	/* Enable Interrupts */
	NVIC_SetPriority(PORTD_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(PORTD_IRQn);  // clear any pending interrupts
	NVIC_EnableIRQ(PORTD_IRQn);
}

void PORTD_IRQHandler(void) {  
	NVIC_ClearPendingIRQ(PORTD_IRQn);
	if ((PORTD->ISFR & MASK(BUTTON_POS))) {
		isr_evt_set (MODE_BTN_PRESSED, t_mode_evt_mngr);
	}
	if ((PORTD->ISFR & MASK(BUTTON2_POS))) {
		isr_evt_set (RESET_BTN_PRESSED, t_reset_evt_mngr);
	}

	// Clear status flags 
	PORTD->ISFR = 0xffffffff; 
	// Ok to clear all since this handler is for all of Port D
}

void clearEvents(OS_TID tid) {
	os_evt_clr (MODE_BTN_PRESSED, tid);
	os_evt_clr (RESET_BTN_PRESSED, tid);
	os_evt_clr (RESET_DONE, tid);
}

/* -------------------------------------
    Timer interrupt handler

    Check each channel to see if caused interrupt
    Write 1 to TIF to reset interrupt flag
   ------------------------------------- */
void PIT_IRQHandler(void) {
	// clear pending interrupts
	NVIC_ClearPendingIRQ(PIT_IRQn);

	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
		// clear TIF
		PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK ;
		
		updateMotor(m1);
	}
}

// Task to feedback user click with LEDs
__task void ledFeedbackTask(void) {
	int state = COLOR_RED;
	greenLEDOnOff(LED_OFF);
	
	while(1) {
		// Wait for the user click
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

__task void controlMotorTask(void) {
	FSMControlStates state = ST_CONTROL_START;
	
	while(1) {		
		if(os_evt_wait_and (RESET_BTN_PRESSED, 1) == OS_R_EVT) {
			state = ST_CONTROL_RESETWAIT;
		}
		
		switch (state) {
			case ST_CONTROL_START:
				os_evt_wait_and (MODE_BTN_PRESSED, 0xFFFF); 
			
				// Clockwise movement
				stopMotor(m1);
				moveSteps(m1, motorMode.steps, motorMode.rotation) ;

				// Configure the timer and start it
				stopTimer(0);
				setTimer(0, motorMode.speed);
				startTimer(0);
				
				state = ST_CONTROL_GO;			
				break ;
			
			case ST_CONTROL_GO:
				if(!isMoving(m1)) {
					int returnSteps = motorMode.steps % STEPS;
					
					// Anti-clockwise movement
					stopMotor(m1);
					moveSteps(m1, returnSteps, !motorMode.rotation) ;
					
					state = ST_CONTROL_RETURN;
				}
				break;

			case ST_CONTROL_RESETWAIT:
				os_evt_wait_and (RESET_DONE, 0xFFFF); 
				state = ST_CONTROL_RETURN;
				break;
			
			case ST_CONTROL_RETURN:
				if(!isMoving(m1)) {
					motorMode.next(&motorMode);
					state = ST_CONTROL_START;
				}
				break;
		}
		
		clearEvents(t_tasks[T_CONTROL_MOTOR]);
	}
}

__task void resetMotorTask(void) {
	FSMResetStates state = ST_RESET_START;
	bool stop = false;
	while(1) {
		switch(state) {
			case ST_RESET_START:
				os_evt_wait_and (RESET_BTN_PRESSED, 0xFFFF); 
			
				if(!isMoving(m1) && stop) {
					
					// Anti-clockwise movement
					int32_t returnSteps = abs(getSteps(m1)) % 48;
					stopMotor(m1);
					if(returnSteps > 0) {
						moveSteps(m1, returnSteps, !motorMode.rotation);
					}
					
					state = ST_RESET_RETURN;
				}
				else if(isMoving(m1) && !stop) {
					state = ST_RESET_STOP;
				}
				break;
				
			case ST_RESET_STOP:
				// Stop the motor
				while(isMoving(m1)) {
					stopMotor(m1);
				}
				
				if(!isMoving(m1)) {
					stop = true;
					state = ST_RESET_START;
				}
				break;
			
			case ST_RESET_RETURN:
				if(!isMoving(m1)) {
					os_evt_set (RESET_DONE, t_tasks[T_CONTROL_MOTOR]);
					state = ST_RESET_START;
					stop = false;
				}
				break;
		}
		clearEvents(t_tasks[T_RESET_MOTOR]);
	}
}

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
