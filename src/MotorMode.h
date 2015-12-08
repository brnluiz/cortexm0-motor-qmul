#ifndef MOTOR_MODES
#define MOTOR_MODES

#define STEPS 48
typedef enum {
	MOTOR_STEPS_1 = (STEPS    + STEPS/3),
	MOTOR_STEPS_2 = (5*STEPS  + 2*STEPS/3),
	MOTOR_STEPS_3 = (10*STEPS),
	MOTOR_STEPS_4 = (20*STEPS + STEPS/3),
	MOTOR_STEPS_5 = (10*STEPS + 2*STEPS/3),
	MOTOR_STEPS_6 = (20*STEPS),
	MOTOR_STEPS_7 = (30*STEPS + STEPS/3),
	MOTOR_STEPS_8 = (40*STEPS + 2*STEPS/3)
} MotorSteps;

typedef enum {
	MOTOR_1,
	MOTOR_2,
	MOTOR_3,
	MOTOR_4,
	MOTOR_5,
	MOTOR_6,
	MOTOR_7,
	MOTOR_8
} MotorState;

typedef enum {
	MOTOR_CLCK,
	MOTOR_ANTICLCK,
} MotorRotation;

typedef enum {
	MOTOR_TIME_1 = 20,
	MOTOR_TIME_2 = 20,
	MOTOR_TIME_3 = 20,
	MOTOR_TIME_4 = 20,
	MOTOR_TIME_5 = 10,
	MOTOR_TIME_6 = 10,
	MOTOR_TIME_7 = 10,
	MOTOR_TIME_8 = 10,
} MotorTime;

#define PIT_SEC 24000000
#define SPEED_CALC(sec, steps) (float)((float)sec/(float)steps) * PIT_SEC

typedef struct MotorMode MotorMode;

struct MotorMode {
	unsigned speed;
	MotorState state;
	MotorSteps steps;
	MotorRotation rotation;
	
	void (*next)(MotorMode* self);
	void (*reset)(MotorMode* self);
};

void mode_next(MotorMode* self);
void mode_reset(MotorMode* self);
MotorMode mode_construct(void);

#endif
