#ifndef SETTINGS_H
#define SETTINGS_H

// Tasks configurations
#define TOTAL_TASKS 2
enum {
	T_LEDS,
	T_CONTROL_MOTOR,
};

enum {
	OFF,
	ON
};

// Timeout configurations
#define LED_TIMEOUT      30000 // 3000ms
#define DEBOUNCE_TIMEOUT 2000  // 200ms
#define MOTOR_SPEED      100   // 10ms

// Definitions (ENUMS and DEFINES)
#define TOTAL_EVENTS 1
typedef enum {
	MODE_BTN_PRESSED = 1,
	RESET_BTN_PRESSED,
} Events;

typedef enum {
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE
} LedStates;

#define FASTDELAY (0) 
#define SLOWDELAY (4) 

#define STATESTART (0)
#define STATERUNNING (1)
#define STATERETURN (2)
#define STATESTOPPED (3)

#define BUTTONOPEN (0)
#define BUTTONCLOSED (1)
#define BUTTONBOUNCE (2)

typedef enum {
	ST_CONTROL_START,
	ST_CONTROL_GO,
	ST_CONTROL_RETURN
} FSTControlStates;

#endif
