// Definitions 

#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

#define MASK(x) (1UL << (x))

// Freedom KL25Z LEDs
#define RED_LED_POS (18)		// on port B
#define GREEN_LED_POS (19)	// on port B
#define BLUE_LED_POS (1)		// on port D

// Button is on port D, pin 6
#define BUTTON_POS (6)

// LED states
#define LED_ON (1)
#define LED_OFF (0)

// Outputs for stepper motor, on port E
#define MOTOR_IN1 (30) // phase A+
#define MOTOR_IN2 (29) // phase A-
#define MOTOR_IN3 (23) // phase B+
#define MOTOR_IN4 (22) // phase B-

#endif
