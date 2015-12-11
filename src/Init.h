#ifndef INIT_FUNCS_H
#define INIT_FUNCS_H
void setupMotor(void);
void initTimers(void);
void initInputButton(void);

extern motorType mcb ;   // motor control block
extern MotorId m1 ;      // motor id
extern MotorMode motorMode;

#endif
