#include "MotorMode.h"

void mode_next(MotorMode *self) {
	if (self->state == MOTOR_1) {
		self->state    = MOTOR_2;
		self->steps    = MOTOR_STEPS_2;
		self->rotation = MOTOR_CLCK;
		self->speed    = SPEED_CALC(MOTOR_TIME_2, MOTOR_STEPS_2);
	}
	else if (self->state == MOTOR_2) {
		self->state    = MOTOR_3;
		self->steps    = MOTOR_STEPS_3;
		self->rotation = MOTOR_CLCK;
		self->speed    = SPEED_CALC(MOTOR_TIME_3, MOTOR_STEPS_3);
	}
	else if (self->state == MOTOR_3) {
		self->state    = MOTOR_4;
		self->steps    = MOTOR_STEPS_4;
		self->rotation = MOTOR_ANTICLCK;
		self->speed    = SPEED_CALC(MOTOR_TIME_4, MOTOR_STEPS_4);
	}
	else if (self->state == MOTOR_4) {
		self->state    = MOTOR_5;
		self->steps    = MOTOR_STEPS_5;
		self->rotation = MOTOR_ANTICLCK;
		self->speed    = SPEED_CALC(MOTOR_TIME_5, MOTOR_STEPS_5);
	}
	else if (self->state == MOTOR_5) {
		self->state    = MOTOR_6;
		self->steps    = MOTOR_STEPS_6;
		self->rotation = MOTOR_ANTICLCK;
		self->speed    = SPEED_CALC(MOTOR_TIME_6, MOTOR_STEPS_6);
	}
	else if (self->state == MOTOR_6) {
		self->state    = MOTOR_7;
		self->steps    = MOTOR_STEPS_7;
		self->rotation = MOTOR_CLCK;
		self->speed    = SPEED_CALC(MOTOR_TIME_7, MOTOR_STEPS_7);
	}
	else if (self->state == MOTOR_7) {
		self->state    = MOTOR_8;
		self->steps    = MOTOR_STEPS_8;
		self->rotation = MOTOR_CLCK;
		self->speed    = SPEED_CALC(MOTOR_TIME_8, MOTOR_STEPS_8);
	}
	else if (self->state == MOTOR_8) {
		self->reset(self);
	}
}

void mode_reset(MotorMode *self) {
	self->state    = MOTOR_1;
	self->steps    = MOTOR_STEPS_1;
	self->speed    = SPEED_CALC(MOTOR_TIME_1, MOTOR_STEPS_1);
	self->rotation = MOTOR_CLCK;
}

MotorMode mode_construct() {
	MotorMode mode;
	
	mode.state    = MOTOR_1;
	mode.steps    = MOTOR_STEPS_1;
	mode.speed    = SPEED_CALC(MOTOR_TIME_1, MOTOR_STEPS_1);
	mode.rotation = MOTOR_CLCK;
	
	mode.next = mode_next;
	mode.reset = mode_reset;
	
	return mode;
}
