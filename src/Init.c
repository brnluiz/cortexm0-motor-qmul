#include <RTL.h>
#include <MKL25Z4.H>
#include "GpioDefs.h"
#include "Settings.h"
#include "Pit.h"
#include "MotorMode.h"
#include "StepperMotor.h"

motorType mcb ;   // motor control block
MotorId m1 ;      // motor id
MotorMode motorMode;

void setupMotor(void) {
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

void initTimers(void) {
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
