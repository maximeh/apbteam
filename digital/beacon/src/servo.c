/* servo.c */
/* Beacon servomotor management. {{{
 *
 * Copyright (C) 2012 Florent Duchon
 *
 * APBTeam:
 *        Web: http://apbteam.org/
 *      Email: team AT apbteam DOT org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * }}} */

#include <types.h>
#include <irq.h>
#include "servo.h" 
#include "print.h"

servo_s servo1;
servo_s servo2;
static HAL_AppTimer_t waveTimer;						// TIMER descripor used by the DEBUG task

/* This function initializes low and high level modules for servos */
void servo_init(void)
{
	servo_timer1_init();
	servo_structure_init();
}

/* This function initializes the timer used for servomotor signal generation */
void servo_timer1_init(void)
{

	//Fpwm = f_IO / (prescaler * (1 + TOP)) = 7200 Hz. */
  	OCR1B = SERVO_ANGLE_MAX; 
  	OCR1A = SERVO_ANGLE_MIN;

	
	/* Fast PWM 10bits with TOP=0x03FF */
	TCCR1A |= (1<<WGM11)|(1<<WGM10);
	TCCR1B |= (1<<WGM12);
	
	/* Prescaler = 256 */
	TCCR1B |= (1<<CS11) | (1<<CS10);
	
// 	/* Prescaler = 1 */
// 	TCCR1B |= (1<<CS10);

	/* Postive Logic */
 	TCCR1A |= (1<<COM1A1)|(1<<COM1B1);
	
	/* Select ouptut */
 	DDRB |= (1<<PB5) | (1<<PB6);
	
	
	/* Configure Overflow and Input compare interrupts */
   	TIMSK1 |= (1<<TOIE1); 

	/* Enable Interrupts */
 	sei(); 
}


/* This function initializes the servo structures */
void servo_structure_init(void)
{
	/* Servo 1 init values */
	servo1.id = SERVO_1;
	servo1.state = SERVO_SCANNING_OFF;
	servo1.top = 0;
	servo1.bottom = 0;
	servo1.scanning_sense = RISING;
	
	/* Servo 2 init values */
	servo2.id = SERVO_2;
	servo2.state = SERVO_SCANNING_OFF;
	servo2.top = 0;
	servo2.bottom = 0;
	servo2.scanning_sense = FALLING;
}

/* This function increases by one unit the angle of the defined servo and returns "angle" value */
int16_t servo_angle_increase(TServo_ID servo_id)
{
	switch(servo_id)
	{
		case SERVO_1:
			if(OCR1A > SERVO_ANGLE_MIN)
			{
				OCR1A--;
			}
			return OCR1A;
			break;
		case SERVO_2:
			if(OCR1B < SERVO_ANGLE_MAX)
			{
				OCR1B++;
			}
			return OCR1B;
			break;
		default:
			break;
	}
	return -1;
}


/* This function decreases by one unit the angle of the defined servo and returns "angle" value*/
int16_t servo_angle_decrease(TServo_ID servo_id)
{
	switch(servo_id)
	{
		case SERVO_1:
			if(OCR1A < SERVO_ANGLE_MAX)
			{
				OCR1A++;
			}
			return OCR1A;
			break;
		case SERVO_2:
			if(OCR1B > SERVO_ANGLE_MIN)
			{
				OCR1B--;
			}
			return OCR1B;
			break;
		default:
			break;
	}
	return -1;
}

/* This function returns the "angle" value of a defined servo */
int16_t servo_get_value(TServo_ID servo_id)
{
	switch(servo_id)
	{
		case SERVO_1:
			return OCR1A;
			break;
		case SERVO_2:
			return OCR1B;
			break;
		default:
			break;
	}
	return -1;
}

/* This function sets the "angle" value of a defined servo */
int16_t servo_set_value(TServo_ID servo_id,int16_t value)
{
	if(value < SERVO_ANGLE_MIN)
		value = SERVO_ANGLE_MIN;
	else if(value > SERVO_ANGLE_MAX)
		value = SERVO_ANGLE_MAX;
	
	switch(servo_id)
	{
		case SERVO_1:
			OCR1A = value;
			break;
		case SERVO_2:
			OCR1B= value;
			break;
		default:
			break;
	}
	return value;
}

/* This function returns the current state of a defined servo */
TServo_state servo_get_state(TServo_ID servo_id)
{
	switch(servo_id)
	{
		case SERVO_1:
			return servo1.state;
		case SERVO_2:
			return servo2.state;
		default:
			break;
	}
	return -1;
}

/* This function updates the state of a defined servo */
void servo_set_state(TServo_ID servo_id,TServo_state state)
{
	switch(servo_id)
	{
		case SERVO_1:
			servo1.state = state;
			break;
		case SERVO_2:
			servo2.state = state;
			break;
		default:
			break;
	}
}

/* This function returns the scanning sens of the defined servo */
int8_t servo_get_scanning_sense(TServo_ID servo_id)
{
	switch(servo_id)
	{
		case SERVO_1:
			return servo1.scanning_sense;
			break;
		case SERVO_2:
			return servo2.scanning_sense;
			break;
		default:
			break;
	}
	return 0;
}

/* This function inverses the scanning sense of the servo */
void servo_inverse_scanning_sense(TServo_ID servo_id)
{
	switch(servo_id)
	{
		case SERVO_1:
			servo1.scanning_sense *= -1;
			break;
		case SERVO_2:
			servo2.scanning_sense *= -1;
			break;
		default:
			break;
	}
}

/* This function generates a wave scanning */
void servo_waveform_scanning(TServo_ID servo_id, uint8_t average_value)
{
	
	uint16_t next_value = 0;
	uint16_t current_value = 0;

	/* Compute next value to set to the servo */
	next_value = servo_get_value(servo_id) + servo_get_scanning_sense(servo_id);

	/* Set it and check the return value in order to inverse the sense MIN or MAX is reached */
	current_value = servo_set_value(servo_id,next_value);

	if((current_value <= average_value - SERVO_WAVE_OFFSET) || (current_value >= average_value + SERVO_WAVE_OFFSET))
	{
		servo_inverse_scanning_sense(servo_id);
	}
}

/* Wave Task */
void servo_wave_task(void)
{
	static bool first_time = 1;
	static uint8_t average_value[2] = {0};
	
	/* Get and save the average value found with the calibration */
	if(first_time == 1)
	{
		average_value[SERVO_1] = servo_get_value(SERVO_1);
		average_value[SERVO_2] = servo_get_value(SERVO_2);
		first_time = 0;
	}
	
	/* Scan each servos */
	servo_waveform_scanning(SERVO_1,average_value[SERVO_1]);
	servo_waveform_scanning(SERVO_2,average_value[SERVO_2]);
}

/* Start wave task */
void servo_start_wave_task(void)
{
	waveTimer.interval = WAVE_TASK_PERIOD;
	waveTimer.mode     = TIMER_REPEAT_MODE;
	waveTimer.callback = servo_wave_task;
	HAL_StartAppTimer(&waveTimer);
}

/* Stop wave task */
void servo_stop_wave_task(void)
{
	HAL_StopAppTimer(&waveTimer);
}

SIGNAL (SIG_OVERFLOW1)
{
}

