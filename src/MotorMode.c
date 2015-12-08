#include "MotorMode.h"

void mode_next(MotorMode *self) {
	if (self->state == MOTOR_1) {
		self->state  = MOTOR_2;
		self->steps = MOTOR_STEPS_2;
		self->rotation = MOTOR_CLCK;
		self->speed = 100;
	}
	else if (self->state == MOTOR_2) {
		self->state  = MOTOR_3;
		self->steps = MOTOR_STEPS_3;
		self->rotation = MOTOR_CLCK;
		self->speed = 100;
	}
	else if (self->state == MOTOR_3) {
		self->state  = MOTOR_4;
		self->steps = MOTOR_STEPS_4;
		self->rotation = MOTOR_ANTICLCK;
		self->speed = 100;
	}
	else if (self->state == MOTOR_4) {
		self->state  = MOTOR_5;
		self->steps = MOTOR_STEPS_5;
		self->rotation = MOTOR_ANTICLCK;
		self->speed = 100;
	}
	else if (self->state == MOTOR_5) {
		self->state  = MOTOR_6;
		self->steps = MOTOR_STEPS_6;
		self->rotation = MOTOR_ANTICLCK;
		self->speed = 100;
	}
	else if (self->state == MOTOR_6) {
		self->state  = MOTOR_7;
		self->steps = MOTOR_STEPS_7;
		self->rotation = MOTOR_CLCK;
		self->speed = 100;
	}
	else if (self->state == MOTOR_7) {
		self->state  = MOTOR_8;
		self->steps = MOTOR_STEPS_8;
		self->rotation = MOTOR_CLCK;
		self->speed = 100;
	}
	else if (self->state == MOTOR_8) {
		self->reset(self);
	}
}

void mode_reset(MotorMode *self) {
	self->state  = MOTOR_1;
	self->steps  = MOTOR_STEPS_1;
	self->speed = 100;
	self->rotation = MOTOR_CLCK;
}

MotorMode mode_construct() {
	MotorMode mode;
	
	mode.state = MOTOR_1;
	mode.speed = 100;
	mode.rotation = MOTOR_CLCK;
	mode.steps = MOTOR_STEPS_1;
	
	mode.next = mode_next;
	mode.reset = mode_reset;
	
	return mode;
}
