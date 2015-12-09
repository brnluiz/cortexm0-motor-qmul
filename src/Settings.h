#ifndef SETTINGS_H
#define SETTINGS_H

// Tasks configurations
#define TOTAL_TASKS 3
enum {
	T_LEDS,
	T_CONTROL_MOTOR,
	T_RESET_MOTOR
};

enum {
	OFF,
	ON
};

// Timeout configurations
#define LED_TIMEOUT      30000 // 3000ms
#define DEBOUNCE_TIMEOUT 3000  // 200ms

// Definitions (ENUMS and DEFINES)
#define TOTAL_EVENTS 1
typedef enum {
	MODE_BTN_PRESSED = 1,
	RESET_BTN_PRESSED,
	RESET_DONE,
} Events;


#define STEPS 48
#define PIT_SEC 24000000
#define SPEED_CALC(sec, steps) (float)((float)sec/(float)steps) * PIT_SEC

typedef enum {
	ST_CONTROL_START,
	ST_CONTROL_GO,
	ST_CONTROL_RESETWAIT,
	ST_CONTROL_RETURN
} FSMControlStates;

typedef enum {
	ST_RESET_START,
	ST_RESET_STOP,
	ST_RESET_RETURN
} FSMResetStates;

typedef enum {
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE
} LedStates;

#endif
