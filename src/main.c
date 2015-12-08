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

OS_TID t_mode_evt_mngr;
OS_TID t_tasks[TOTAL_TASKS]; /*  task ids */

motorType mcb ;   // motor control block
MotorId m1 ;      // motor id
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
}

void initInputButton(void) {
	SIM->SCGC5 |=  SIM_SCGC5_PORTD_MASK; /* enable clock for port D */

	/* Select GPIO and enable pull-up resistors and interrupts 
		on falling edges for pins connected to switches */
	PORTD->PCR[BUTTON_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0a);
		
	/* Set port D switch bit to inputs */
	PTD->PDDR &= ~MASK(BUTTON_POS);

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
	// Clear status flags 
	PORTD->ISFR = 0xffffffff; 
	// Ok to clear all since this handler is for all of Port D
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
		os_evt_clr (MODE_BTN_PRESSED, t_tasks[T_LEDS]);
	}
}

// Button Event Manager Task
__task void modeBtnEventManagerTask(void) {
	int i = 0;
	int buttonPressed;

	while(1) {
		// Wait until button is pressed
		buttonPressed = os_evt_wait_and (MODE_BTN_PRESSED, 0xffff);
		
		// Propagate the event through all listeners
		for(i=0; i<TOTAL_TASKS;i++) {
			// TODO: Check if it is 0... If is not, then send the event
			os_evt_set (MODE_BTN_PRESSED, t_tasks[i]);
		}
		
		// Wait some time to debounce the buton
		os_dly_wait(DEBOUNCE_TIMEOUT);
		
		// Discard pending notifications
		os_evt_clr (MODE_BTN_PRESSED, t_mode_evt_mngr);
	}
}

MotorSteps nextMode(MotorSteps mode) {
	if (mode == MOTOR_STEPS_1) {
		mode = MOTOR_STEPS_2;
	} else if (mode == MOTOR_STEPS_2) {
		mode = MOTOR_STEPS_3;
	} else if (mode == MOTOR_STEPS_3) {
		mode = MOTOR_STEPS_4;
	} else if (mode == MOTOR_STEPS_4) {
		mode = MOTOR_STEPS_5;
	} else if (mode == MOTOR_STEPS_5) {
		mode = MOTOR_STEPS_6;
	} else if (mode == MOTOR_STEPS_6) {
		mode = MOTOR_STEPS_7;
	} else if (mode == MOTOR_STEPS_7) {
		mode = MOTOR_STEPS_8;
	} else if (mode == MOTOR_STEPS_8) {
		mode = MOTOR_STEPS_1;
	}
	
	return mode;
}

__task void controlMotorTask(void) {
	FSTControlStates state = ST_CONTROL_START;
	bool motorRunning;
	MotorSteps modeSteps = MOTOR_STEPS_1;
	
	while(1) {		
		updateMotor(m1) ;
		motorRunning = isMoving(m1) ;
		
		switch (state) {
			case ST_CONTROL_START:
				os_evt_wait_and (MODE_BTN_PRESSED, 0xFFFF); 

				// Clockwise movement
				moveSteps(m1, modeSteps, true) ;
				state = ST_CONTROL_GO;
				break ;
			case ST_CONTROL_GO:
				os_dly_wait(MOTOR_SPEED); 
				if(!motorRunning) {
					stopMotor(m1);
					// Anti-clockwise movement
					moveSteps(m1, modeSteps, false) ;
					state = ST_CONTROL_RETURN;
				}
				break;
			case ST_CONTROL_RETURN:
				os_dly_wait(MOTOR_SPEED);
				if(!motorRunning) {
					modeSteps = nextMode(modeSteps);
					state = ST_CONTROL_START;
				}
				break;
		}
		
		os_evt_clr (MODE_BTN_PRESSED, t_tasks[T_CONTROL_MOTOR]);
	}
}

__task void boot (void) {
  t_mode_evt_mngr = os_tsk_create (modeBtnEventManagerTask, 0);   // start button task
	
	t_tasks[T_LEDS]          = os_tsk_create (ledFeedbackTask, 0);          // start led task (only user feedback)
	t_tasks[T_CONTROL_MOTOR] = os_tsk_create (controlMotorTask, 0);        // start tone task (generate the tone)
	
	os_tsk_delete_self ();
}

int main (void) {
	// Initialize input and outputs
	initOutputLeds();
	initInputButton();
	setupMotor();
	
	// Initialize RTX and start init
	os_sys_init(boot);
}
